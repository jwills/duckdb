#include "catch.hpp"
#include "duckdb/common/types/date.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include "duckdb/execution/operator/persistent/csv_reader_options.hpp"
#include "duckdb/main/appender.hpp"
#include "test_helpers.hpp"
#include "duckdb/main/client_data.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace duckdb;
using namespace std;

//! CSV Files
const string csv = "*.csv";
const string tsv = "*.tsv";
const string csv_gz = "csv.gz";
const string csv_zst = "csv.zst";
const string tbl_zst = "tbl.zst";

const string csv_extensions[5] = {csv, tsv, csv_gz, csv_zst, tbl_zst};

bool debug = true;
bool RunFull(std::string &path, std::set<std::string> &skip, duckdb::Connection &conn,
             const string &add_parameters = "") {
	bool single_threaded_passed;
	// Here we run the csv file first with the single thread reader.
	// Then the parallel csv reader with a combination of multiple threads and buffer sizes.
	if (skip.find(path) != skip.end()) {
		// Gotta skip this
		return true;
	}
	// Set max line length to 0 when starting a ST CSV Read
	conn.context->client_data->max_line_length = 0;
	unique_ptr<MaterializedQueryResult> single_threaded_res;
	ColumnDataCollection *ground_truth = nullptr;
	single_threaded_res =
	    conn.Query("SELECT * FROM read_csv_auto('" + path + add_parameters + "', parallel = 0) ORDER BY ALL");
	if (single_threaded_res->HasError()) {
		single_threaded_passed = false;
	} else {
		single_threaded_passed = true;
		ground_truth = &single_threaded_res->Collection();
	}

	// For parallel CSV Reading the buffer must be at least the size of the biggest line in the File.
	idx_t min_buffer_size = conn.context->client_data->max_line_length + 2;
	// So our tests don't take infinite time, we will go till a max buffer size of 5 positions higher than the minimum.
	idx_t max_buffer_size = min_buffer_size + 5;
	// Let's go from 1 to 8 threads.
	if (debug) {
		std::cout << path << std::endl;
	}
	for (auto thread_count = 1; thread_count <= 8; thread_count++) {
		DuckDB db(nullptr);
		Connection multi_conn(db);
		for (auto buffer_size = min_buffer_size; buffer_size < max_buffer_size; buffer_size++) {
			if (debug) {
				std::cout << " Thread count: " << to_string(thread_count) << " Buffer Size: " << to_string(buffer_size)
				          << std::endl;
			}
			multi_conn.Query("SET preserve_insertion_order=false;");
			multi_conn.Query("PRAGMA threads=" + to_string(thread_count));
			unique_ptr<MaterializedQueryResult> multi_threaded_result =
			    multi_conn.Query("SELECT * FROM read_csv_auto('" + path + add_parameters +
			                     "', buffer_size = " + to_string(buffer_size) + ") ORDER BY ALL");
			bool multi_threaded_passed;
			ColumnDataCollection *result = nullptr;
			if (multi_threaded_result->HasError()) {
				multi_threaded_passed = false;
			} else {
				multi_threaded_passed = true;
				result = &multi_threaded_result->Collection();
			}
			if (!single_threaded_passed && !multi_threaded_passed) {
				// Two wrongs can make a right
				continue;
			}
			if (!single_threaded_passed && multi_threaded_passed) {
				//! oh oh, this should not pass
				std::cout << path << " Failed on single threaded but succeeded on parallel reading" << std::endl;
				return true;
			}
			if (!multi_threaded_passed) {
				std::cout << path << " Multithreaded failed" << std::endl;
				std::cout << multi_threaded_result->GetError() << std::endl;
				return true;
			}
			// Results do not match
			string error_message;
			if (!ColumnDataCollection::ResultEquals(*ground_truth, *result, error_message, true)) {
				std::cout << path << " Thread count: " << to_string(thread_count)
				          << " Buffer Size: " << to_string(buffer_size) << std::endl;
				std::cout << error_message << std::endl;
				return true;
			}
		}
	}

	return true;
}

// Collects All CSV-Like files from folder and execute Parallel Scans on it
void RunTestOnFolder(const string &path, std::set<std::string> &skip, const string &add_parameters = "") {
	DuckDB db(nullptr);
	Connection con(db);

	auto &fs = duckdb::FileSystem::GetFileSystem(*con.context);
	for (auto &ext : csv_extensions) {
		auto csv_files = fs.Glob(path + "*" + ext);
		for (auto &csv_file : csv_files) {
			REQUIRE(RunFull(csv_file, skip, con, add_parameters));
		}
	}
}

 TEST_CASE("Test One File", "[parallel-csv]") {
	DuckDB db(nullptr);
	Connection con(db);
	std::set<std::string> skip;
	con.Query("SET preserve_insertion_order=false;");

	string file = "data/csv/nullpadding_big_mixed.csv";

	auto thread_count = 3;
	auto buffer_size = 207;
//	skip.insert("data/csv/tpcds_59.csv");
//	skip.insert("data/csv/nullpadding_big_mixed.csv");
//	//! This file is way too big
//	skip.insert("data/csv/sequences.csv.gz");
//	skip.insert("test/sql/copy/csv/data/real/voter.tsv");
//
//	con.Query("PRAGMA threads=" + to_string(thread_count));
//	unique_ptr<MaterializedQueryResult> multi_threaded_result = con.Query(
//	    "SELECT * FROM read_csv_auto('" + file + "', buffer_size = " + to_string(buffer_size) + ")");
//	auto &result = multi_threaded_result->Collection();
////	auto rows = result.GetRows();
	REQUIRE(RunFull(file, skip, con));
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto", "[parallel-csv]") {
	std::set<std::string> skip;
	// This file is from a 'mode skip' test
	skip.insert("test/sql/copy/csv/data/auto/titlebasicsdebug.tsv");
	RunTestOnFolder("test/sql/copy/csv/data/auto/", skip);
}
//! Test case with specific parameters that allow us to run the titlebasicsdebug.tsv we were skipping
TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto/titlebasicsdebug.tsv", "[parallel-csv]") {
	std::set<std::string> skip;
	DuckDB db(nullptr);
	Connection con(db);
	string add_parameters = ", nullstr=\'\\N\', sample_size = -1";
	string file = "test/sql/copy/csv/data/auto/titlebasicsdebug.tsv";
	REQUIRE(RunFull(file, skip, con, add_parameters));
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto/glob", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/auto/glob/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/error/date_multiple_file", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/error/date_multiple_file/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a1", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/glob/a1/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a2", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/glob/a2/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a3", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/glob/a3/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/empty", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/glob/empty/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/i1", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/glob/i1/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/real", "[parallel-csv]") {
	std::set<std::string> skip;
	//! Thread count: 2 Buffer Size: 1266
	skip.insert("test/sql/copy/csv/data/real/voter.tsv");
	RunTestOnFolder("test/sql/copy/csv/data/real/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/test", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/test/", skip);
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/zstd", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("test/sql/copy/csv/data/zstd/", skip);
}

TEST_CASE("Test Parallel CSV All Files - data/csv", "[parallel-csv]") {
	std::set<std::string> skip;
	skip.insert("data/csv/tpcds_59.csv");
	skip.insert("data/csv/nullpadding_big_mixed.csv");
	//! This file is way too big
	skip.insert("data/csv/sequences.csv.gz");
	RunTestOnFolder("data/csv/", skip);
}

TEST_CASE("Test Parallel CSV All Files - data/csv/decimal_separators", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("data/csv/decimal_separators/", skip);
}

TEST_CASE("Test Parallel CSV All Files - data/csv/中文", "[parallel-csv]") {
	std::set<std::string> skip;
	RunTestOnFolder("data/csv/中文/", skip);
}
