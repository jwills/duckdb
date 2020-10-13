#include "duckdb/function/scalar/string_functions.hpp"

#include "duckdb/common/crypto/md5.hpp"

using namespace std;

namespace duckdb {

static void md5_function(DataChunk &args, ExpressionState &state, Vector &result) {
	assert(args.column_count() == 1);

	UnaryExecutor::Execute<string_t, string_t, true>(args.data[0], result, args.size(), [&](string_t input) {
		MD5 md5;
		md5.Add(string(input.GetData(), input.GetSize()));
		string result_str = md5.Finish();
		return StringVector::AddString(result, result_str);
	});
	StringVector::AddHeapReference(result, args.data[0]);
}

void MD5Fun::RegisterFunction(BuiltinFunctions &set) {
	set.AddFunction(ScalarFunction("md5", {LogicalType::VARCHAR}, LogicalType::VARCHAR, md5_function));
}

} // namespace duckdb
