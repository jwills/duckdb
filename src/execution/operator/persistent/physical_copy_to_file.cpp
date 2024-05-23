#include "duckdb/execution/operator/persistent/physical_copy_to_file.hpp"

#include "duckdb/common/file_opener.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/common/hive_partitioning.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/common/types/uuid.hpp"
#include "duckdb/common/value_operations/value_operations.hpp"
#include "duckdb/common/vector_operations/vector_operations.hpp"
#include "duckdb/storage/storage_lock.hpp"

#include <algorithm>

namespace duckdb {

struct PartitionWriteInfo {
	unique_ptr<GlobalFunctionData> global_state;
};

struct VectorOfValuesHashFunction {
	uint64_t operator()(const vector<Value> &values) const {
		hash_t result = 0;
		for (auto &val : values) {
			result ^= val.Hash();
		}
		return result;
	}
};

struct VectorOfValuesEquality {
	bool operator()(const vector<Value> &a, const vector<Value> &b) const {
		if (a.size() != b.size()) {
			return false;
		}
		for (idx_t i = 0; i < a.size(); i++) {
			if (ValueOperations::DistinctFrom(a[i], b[i])) {
				return false;
			}
		}
		return true;
	}
};

template <class T>
using vector_of_value_map_t = unordered_map<vector<Value>, T, VectorOfValuesHashFunction, VectorOfValuesEquality>;

class CopyToFunctionGlobalState : public GlobalSinkState {
public:
	explicit CopyToFunctionGlobalState(unique_ptr<GlobalFunctionData> global_state)
	    : rows_copied(0), last_file_offset(0), global_state(std::move(global_state)) {
	}
	StorageLock lock;
	atomic<idx_t> rows_copied;
	atomic<idx_t> last_file_offset;
	unique_ptr<GlobalFunctionData> global_state;
	//! Created directories
	unordered_set<string> created_directories;
	//! shared state for HivePartitionedColumnData
	shared_ptr<GlobalHivePartitionState> partition_state;
	//! File names
	vector<Value> file_names;

	void CreateDir(const string &dir_path, FileSystem &fs) {
		if (created_directories.find(dir_path) != created_directories.end()) {
			// already attempted to create this directory
			return;
		}
		if (!fs.DirectoryExists(dir_path)) {
			fs.CreateDirectory(dir_path);
		}
		created_directories.insert(dir_path);
	}

	string GetOrCreateDirectory(const vector<idx_t> &cols, const vector<string> &names, const vector<Value> &values,
	                            string path, FileSystem &fs) {
		CreateDir(path, fs);
		for (idx_t i = 0; i < cols.size(); i++) {
			const auto &partition_col_name = names[cols[i]];
			const auto &partition_value = values[i];
			string p_dir = partition_col_name + "=" + partition_value.ToString();
			path = fs.JoinPath(path, p_dir);
			CreateDir(path, fs);
		}
		return path;
	}

	void FinalizePartition(ClientContext &context, const PhysicalCopyToFile &op, PartitionWriteInfo &info) {
		if (!info.global_state) {
			// already finalized
			return;
		}
		// finalize the partition
		op.function.copy_to_finalize(context, *op.bind_data, *info.global_state);
		info.global_state.reset();
	}

	void FinalizePartitions(ClientContext &context, const PhysicalCopyToFile &op) {
		// finalize any remaining partitions
		for (auto &entry : active_partitioned_writes) {
			FinalizePartition(context, op, *entry.second);
		}
	}

	PartitionWriteInfo &GetPartitionWriteInfo(ExecutionContext &context, const PhysicalCopyToFile &op,
	                                          const vector<Value> &values) {
		auto l = lock.GetExclusiveLock();
		// check if we have already started writing this partition
		auto entry = active_partitioned_writes.find(values);
		if (entry != active_partitioned_writes.end()) {
			// we have - continue writing in this partition
			return *entry->second;
		}
		auto &fs = FileSystem::GetFileSystem(context.client);
		// Create a writer for the current file
		auto trimmed_path = op.GetTrimmedPath(context.client);
		string hive_path = GetOrCreateDirectory(op.partition_columns, op.names, values, trimmed_path, fs);
		string full_path(op.filename_pattern.CreateFilename(fs, hive_path, op.file_extension, 0));
		if (op.return_files) {
			file_names.emplace_back(full_path);
		}
		// initialize writes
		auto info = make_uniq<PartitionWriteInfo>();
		info->global_state = op.function.copy_to_initialize_global(context.client, *op.bind_data, full_path);
		auto &result = *info;
		// store in active write map
		active_partitioned_writes.insert(make_pair(values, std::move(info)));
		return result;
	}

private:
	//! The active writes per partition (for partitioned write)
	vector_of_value_map_t<unique_ptr<PartitionWriteInfo>> active_partitioned_writes;
};

string PhysicalCopyToFile::GetTrimmedPath(ClientContext &context) const {
	auto &fs = FileSystem::GetFileSystem(context);
	string trimmed_path = file_path;
	StringUtil::RTrim(trimmed_path, fs.PathSeparator(trimmed_path));
	return trimmed_path;
}

class CopyToFunctionLocalState : public LocalSinkState {
public:
	explicit CopyToFunctionLocalState(unique_ptr<LocalFunctionData> local_state) : local_state(std::move(local_state)) {
	}
	unique_ptr<GlobalFunctionData> global_state;
	unique_ptr<LocalFunctionData> local_state;

	//! Buffers the tuples in partitions before writing
	unique_ptr<HivePartitionedColumnData> part_buffer;
	unique_ptr<PartitionedColumnDataAppendState> part_buffer_append_state;

	idx_t append_count = 0;

	void InitializeAppendState(ClientContext &context, const PhysicalCopyToFile &op,
	                           CopyToFunctionGlobalState &gstate) {
		part_buffer = make_uniq<HivePartitionedColumnData>(context, op.expected_types, op.partition_columns,
		                                                   gstate.partition_state);
		part_buffer_append_state = make_uniq<PartitionedColumnDataAppendState>();
		part_buffer->InitializeAppendState(*part_buffer_append_state);
		append_count = 0;
	}

	void AppendToPartition(ExecutionContext &context, const PhysicalCopyToFile &op, CopyToFunctionGlobalState &g,
	                       DataChunk &chunk) {
		if (!part_buffer) {
			// re-initialize the append
			InitializeAppendState(context.client, op, g);
		}
		part_buffer->Append(*part_buffer_append_state, chunk);
		append_count += chunk.size();
		if (append_count >= ClientConfig::GetConfig(context.client).partitioned_write_flush_threshold) {
			// flush all cached partitions
			FlushPartitions(context, op, g);
		}
	}

	void ResetAppendState() {
		part_buffer_append_state.reset();
		part_buffer.reset();
		append_count = 0;
	}

	void FlushPartitions(ExecutionContext &context, const PhysicalCopyToFile &op, CopyToFunctionGlobalState &g) {
		if (!part_buffer) {
			return;
		}
		part_buffer->FlushAppendState(*part_buffer_append_state);
		auto &partitions = part_buffer->GetPartitions();
		auto partition_key_map = part_buffer->GetReverseMap();

		for (idx_t i = 0; i < partitions.size(); i++) {
			auto entry = partition_key_map.find(i);
			if (entry == partition_key_map.end()) {
				continue;
			}
			// get the partition write info for this buffer
			auto &info = g.GetPartitionWriteInfo(context, op, entry->second->values);

			auto local_copy_state = op.function.copy_to_initialize_local(context, *op.bind_data);
			// push the chunks into the write state
			for (auto &chunk : partitions[i]->Chunks()) {
				op.function.copy_to_sink(context, *op.bind_data, *info.global_state, *local_copy_state, chunk);
			}
			op.function.copy_to_combine(context, *op.bind_data, *info.global_state, *local_copy_state);
			local_copy_state.reset();
			partitions[i].reset();
		}
		ResetAppendState();
	}
};

unique_ptr<GlobalFunctionData> PhysicalCopyToFile::CreateFileState(ClientContext &context,
                                                                   GlobalSinkState &sink) const {
	auto &g = sink.Cast<CopyToFunctionGlobalState>();
	idx_t this_file_offset = g.last_file_offset++;
	auto &fs = FileSystem::GetFileSystem(context);
	string output_path(filename_pattern.CreateFilename(fs, file_path, file_extension, this_file_offset));
	if (return_files) {
		auto l = g.lock.GetExclusiveLock();
		g.file_names.emplace_back(output_path);
	}
	return function.copy_to_initialize_global(context, *bind_data, output_path);
}

unique_ptr<LocalSinkState> PhysicalCopyToFile::GetLocalSinkState(ExecutionContext &context) const {
	if (partition_output) {
		auto &g = sink_state->Cast<CopyToFunctionGlobalState>();

		auto state = make_uniq<CopyToFunctionLocalState>(nullptr);
		state->InitializeAppendState(context.client, *this, g);
		return std::move(state);
	}
	auto res = make_uniq<CopyToFunctionLocalState>(function.copy_to_initialize_local(context, *bind_data));
	return std::move(res);
}

void CheckDirectory(FileSystem &fs, const string &file_path, bool overwrite) {
	if (fs.IsRemoteFile(file_path) && overwrite) {
		// we only remove files for local file systems
		// as remote file systems (e.g. S3) do not support RemoveFile
		return;
	}
	vector<string> file_list;
	vector<string> directory_list;
	directory_list.push_back(file_path);
	for (idx_t dir_idx = 0; dir_idx < directory_list.size(); dir_idx++) {
		auto directory = directory_list[dir_idx];
		fs.ListFiles(directory, [&](const string &path, bool is_directory) {
			auto full_path = fs.JoinPath(directory, path);
			if (is_directory) {
				directory_list.emplace_back(std::move(full_path));
			} else {
				file_list.emplace_back(std::move(full_path));
			}
		});
	}
	if (file_list.empty()) {
		return;
	}
	if (overwrite) {
		for (auto &file : file_list) {
			fs.RemoveFile(file);
		}
	} else {
		throw IOException("Directory \"%s\" is not empty! Enable OVERWRITE_OR_IGNORE option to force writing",
		                  file_path);
	}
}

unique_ptr<GlobalSinkState> PhysicalCopyToFile::GetGlobalSinkState(ClientContext &context) const {
	if (partition_output || per_thread_output || rotate) {
		auto &fs = FileSystem::GetFileSystem(context);
		if (fs.FileExists(file_path)) {
			// the target file exists AND is a file (not a directory)
			if (fs.IsRemoteFile(file_path)) {
				// for remote files we cannot do anything - as we cannot delete the file
				throw IOException("Cannot write to \"%s\" - it exists and is a file, not a directory!", file_path);
			} else {
				// for local files we can remove the file if OVERWRITE_OR_IGNORE is enabled
				if (overwrite_or_ignore) {
					fs.RemoveFile(file_path);
				} else {
					throw IOException("Cannot write to \"%s\" - it exists and is a file, not a directory! Enable "
					                  "OVERWRITE_OR_IGNORE option to force writing",
					                  file_path);
				}
			}
		}
		// what if the target exists and is a directory
		if (!fs.DirectoryExists(file_path)) {
			fs.CreateDirectory(file_path);
		} else {
			CheckDirectory(fs, file_path, overwrite_or_ignore);
		}

		auto state = make_uniq<CopyToFunctionGlobalState>(nullptr);
		if (!per_thread_output && rotate) {
			state->global_state = CreateFileState(context, *state);
		}

		if (partition_output) {
			state->partition_state = make_shared_ptr<GlobalHivePartitionState>();
		}

		return std::move(state);
	}

	auto state =
	    make_uniq<CopyToFunctionGlobalState>(function.copy_to_initialize_global(context, *bind_data, file_path));
	if (use_tmp_file) {
		state->file_names.emplace_back(GetNonTmpFile(context, file_path));
	} else {
		state->file_names.emplace_back(file_path);
	}
	return std::move(state);
}

//===--------------------------------------------------------------------===//
// Sink
//===--------------------------------------------------------------------===//
void PhysicalCopyToFile::MoveTmpFile(ClientContext &context, const string &tmp_file_path) {
	auto &fs = FileSystem::GetFileSystem(context);
	auto file_path = GetNonTmpFile(context, tmp_file_path);
	if (fs.FileExists(file_path)) {
		fs.RemoveFile(file_path);
	}
	fs.MoveFile(tmp_file_path, file_path);
}

string PhysicalCopyToFile::GetNonTmpFile(ClientContext &context, const string &tmp_file_path) {
	auto &fs = FileSystem::GetFileSystem(context);

	auto path = StringUtil::GetFilePath(tmp_file_path);
	auto base = StringUtil::GetFileName(tmp_file_path);

	auto prefix = base.find("tmp_");
	if (prefix == 0) {
		base = base.substr(4);
	}

	return fs.JoinPath(path, base);
}

PhysicalCopyToFile::PhysicalCopyToFile(vector<LogicalType> types, CopyFunction function_p,
                                       unique_ptr<FunctionData> bind_data, idx_t estimated_cardinality)
    : PhysicalOperator(PhysicalOperatorType::COPY_TO_FILE, std::move(types), estimated_cardinality),
      function(std::move(function_p)), bind_data(std::move(bind_data)), parallel(false) {
}

SinkResultType PhysicalCopyToFile::Sink(ExecutionContext &context, DataChunk &chunk, OperatorSinkInput &input) const {
	auto &g = input.global_state.Cast<CopyToFunctionGlobalState>();
	auto &l = input.local_state.Cast<CopyToFunctionLocalState>();

	if (partition_output) {
		l.AppendToPartition(context, *this, g, chunk);
		return SinkResultType::NEED_MORE_INPUT;
	}

	g.rows_copied += chunk.size();

	if (per_thread_output) {
		auto &gstate = l.global_state;
		if (!gstate) {
			// Lazily create file state here to prevent creating empty files
			gstate = CreateFileState(context.client, *sink_state);
		} else if (rotate && function.rotate_next_file(*gstate, *bind_data, file_size_bytes)) {
			function.copy_to_finalize(context.client, *bind_data, *gstate);
			gstate = CreateFileState(context.client, *sink_state);
		}
		function.copy_to_sink(context, *bind_data, *gstate, *l.local_state, chunk);
		return SinkResultType::NEED_MORE_INPUT;
	}

	if (!file_size_bytes.IsValid() && !rotate) {
		function.copy_to_sink(context, *bind_data, *g.global_state, *l.local_state, chunk);
		return SinkResultType::NEED_MORE_INPUT;
	}

	// FILE_SIZE_BYTES/rotate is set, but threads write to the same file, synchronize using lock
	auto &gstate = g.global_state;
	auto lock = g.lock.GetExclusiveLock();
	if (rotate && function.rotate_next_file(*gstate, *bind_data, file_size_bytes)) {
		auto owned_gstate = std::move(gstate);
		gstate = CreateFileState(context.client, *sink_state);
		lock.reset();
		function.copy_to_finalize(context.client, *bind_data, *owned_gstate);
	} else {
		lock.reset();
	}

	lock = g.lock.GetSharedLock();
	function.copy_to_sink(context, *bind_data, *gstate, *l.local_state, chunk);

	return SinkResultType::NEED_MORE_INPUT;
}

SinkCombineResultType PhysicalCopyToFile::Combine(ExecutionContext &context, OperatorSinkCombineInput &input) const {
	auto &g = input.global_state.Cast<CopyToFunctionGlobalState>();
	auto &l = input.local_state.Cast<CopyToFunctionLocalState>();

	if (partition_output) {
		// flush all remaining partitions
		l.FlushPartitions(context, *this, g);
	} else if (function.copy_to_combine) {
		if (per_thread_output) {
			// For PER_THREAD_OUTPUT, we can combine/finalize immediately (if there is a gstate)
			if (l.global_state) {
				function.copy_to_combine(context, *bind_data, *l.global_state, *l.local_state);
				function.copy_to_finalize(context.client, *bind_data, *l.global_state);
			}
		} else if (rotate) {
			// File in global state may change with FILE_SIZE_BYTES/rotate, need to grab lock
			auto lock = g.lock.GetSharedLock();
			function.copy_to_combine(context, *bind_data, *g.global_state, *l.local_state);
		} else {
			function.copy_to_combine(context, *bind_data, *g.global_state, *l.local_state);
		}
	}

	return SinkCombineResultType::FINISHED;
}

SinkFinalizeType PhysicalCopyToFile::Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
                                              OperatorSinkFinalizeInput &input) const {
	auto &gstate = input.global_state.Cast<CopyToFunctionGlobalState>();
	if (partition_output) {
		// finalize any outstanding partitions
		gstate.FinalizePartitions(context, *this);
		return SinkFinalizeType::READY;
	}
	if (per_thread_output) {
		// already happened in combine
		return SinkFinalizeType::READY;
	}
	if (function.copy_to_finalize) {
		function.copy_to_finalize(context, *bind_data, *gstate.global_state);

		if (use_tmp_file) {
			D_ASSERT(!per_thread_output);
			D_ASSERT(!partition_output);
			D_ASSERT(!file_size_bytes.IsValid());
			D_ASSERT(!rotate);
			MoveTmpFile(context, file_path);
		}
	}
	return SinkFinalizeType::READY;
}

//===--------------------------------------------------------------------===//
// Source
//===--------------------------------------------------------------------===//

SourceResultType PhysicalCopyToFile::GetData(ExecutionContext &context, DataChunk &chunk,
                                             OperatorSourceInput &input) const {
	auto &g = sink_state->Cast<CopyToFunctionGlobalState>();

	chunk.SetCardinality(1);
	chunk.SetValue(0, 0, Value::BIGINT(NumericCast<int64_t>(g.rows_copied.load())));

	if (return_files) {
		chunk.SetValue(1, 0, Value::LIST(LogicalType::VARCHAR, g.file_names));
	}

	return SourceResultType::FINISHED;
}

} // namespace duckdb
