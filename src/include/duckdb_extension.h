//===----------------------------------------------------------------------===//
//
//                         DuckDB
//
// duckdb_extension.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

//! duplicate of duckdb/main/winapi.hpp
#ifndef DUCKDB_API
#ifdef _WIN32
#if defined(DUCKDB_BUILD_LIBRARY) && !defined(DUCKDB_BUILD_LOADABLE_EXTENSION)
#define DUCKDB_API __declspec(dllexport)
#else
#define DUCKDB_API __declspec(dllimport)
#endif
#else
#define DUCKDB_API
#endif
#endif

//! duplicate of duckdb/main/winapi.hpp
#ifndef DUCKDB_EXTENSION_API
#ifdef _WIN32
#ifdef DUCKDB_BUILD_LOADABLE_EXTENSION
#define DUCKDB_EXTENSION_API __declspec(dllexport)
#else
#define DUCKDB_EXTENSION_API
#endif
#else
#define DUCKDB_EXTENSION_API __attribute__((visibility("default")))
#endif
#endif

//! In the future, we are planning to move extension functions to a separate header. For now you can set the define
//! below to remove the functions that are planned to be moved out of this header.
// #define DUCKDB_NO_EXTENSION_FUNCTIONS

//! Set the define below to remove all functions that are deprecated or planned to be deprecated
// #define DUCKDB_API_NO_DEPRECATED

//! API versions
//! If no explicit API version is defined, the latest API version is used.
//! Note that using older API versions (i.e. not using DUCKDB_API_LATEST) is deprecated.
//! These will not be supported long-term, and will be removed in future versions.
#ifndef DUCKDB_API_0_3_1
#define DUCKDB_API_0_3_1 1
#endif
#ifndef DUCKDB_API_0_3_2
#define DUCKDB_API_0_3_2 2
#endif
#ifndef DUCKDB_API_LATEST
#define DUCKDB_API_LATEST DUCKDB_API_0_3_2
#endif

#ifndef DUCKDB_API_VERSION
#define DUCKDB_API_VERSION DUCKDB_API_LATEST
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//===--------------------------------------------------------------------===//
// Enums
//===--------------------------------------------------------------------===//
// WARNING: the numbers of these enums should not be changed, as changing the numbers breaks ABI compatibility
// Always add enums at the END of the enum
//! An enum over DuckDB's internal types.
typedef enum DUCKDB_TYPE {
	DUCKDB_TYPE_INVALID = 0,
	// bool
	DUCKDB_TYPE_BOOLEAN = 1,
	// int8_t
	DUCKDB_TYPE_TINYINT = 2,
	// int16_t
	DUCKDB_TYPE_SMALLINT = 3,
	// int32_t
	DUCKDB_TYPE_INTEGER = 4,
	// int64_t
	DUCKDB_TYPE_BIGINT = 5,
	// uint8_t
	DUCKDB_TYPE_UTINYINT = 6,
	// uint16_t
	DUCKDB_TYPE_USMALLINT = 7,
	// uint32_t
	DUCKDB_TYPE_UINTEGER = 8,
	// uint64_t
	DUCKDB_TYPE_UBIGINT = 9,
	// float
	DUCKDB_TYPE_FLOAT = 10,
	// double
	DUCKDB_TYPE_DOUBLE = 11,
	// duckdb_timestamp, in microseconds
	DUCKDB_TYPE_TIMESTAMP = 12,
	// duckdb_date
	DUCKDB_TYPE_DATE = 13,
	// duckdb_time
	DUCKDB_TYPE_TIME = 14,
	// duckdb_interval
	DUCKDB_TYPE_INTERVAL = 15,
	// duckdb_hugeint
	DUCKDB_TYPE_HUGEINT = 16,
	// duckdb_uhugeint
	DUCKDB_TYPE_UHUGEINT = 32,
	// const char*
	DUCKDB_TYPE_VARCHAR = 17,
	// duckdb_blob
	DUCKDB_TYPE_BLOB = 18,
	// decimal
	DUCKDB_TYPE_DECIMAL = 19,
	// duckdb_timestamp, in seconds
	DUCKDB_TYPE_TIMESTAMP_S = 20,
	// duckdb_timestamp, in milliseconds
	DUCKDB_TYPE_TIMESTAMP_MS = 21,
	// duckdb_timestamp, in nanoseconds
	DUCKDB_TYPE_TIMESTAMP_NS = 22,
	// enum type, only useful as logical type
	DUCKDB_TYPE_ENUM = 23,
	// list type, only useful as logical type
	DUCKDB_TYPE_LIST = 24,
	// struct type, only useful as logical type
	DUCKDB_TYPE_STRUCT = 25,
	// map type, only useful as logical type
	DUCKDB_TYPE_MAP = 26,
	// duckdb_array, only useful as logical type
	DUCKDB_TYPE_ARRAY = 33,
	// duckdb_hugeint
	DUCKDB_TYPE_UUID = 27,
	// union type, only useful as logical type
	DUCKDB_TYPE_UNION = 28,
	// duckdb_bit
	DUCKDB_TYPE_BIT = 29,
	// duckdb_time_tz
	DUCKDB_TYPE_TIME_TZ = 30,
	// duckdb_timestamp
	DUCKDB_TYPE_TIMESTAMP_TZ = 31,
} duckdb_type;
//! An enum over the returned state of different functions.
typedef enum { DuckDBSuccess = 0, DuckDBError = 1 } duckdb_state;
//! An enum over the pending state of a pending query result.
typedef enum {
	DUCKDB_PENDING_RESULT_READY = 0,
	DUCKDB_PENDING_RESULT_NOT_READY = 1,
	DUCKDB_PENDING_ERROR = 2,
	DUCKDB_PENDING_NO_TASKS_AVAILABLE = 3
} duckdb_pending_state;
//! An enum over DuckDB's different result types.
typedef enum {
	DUCKDB_RESULT_TYPE_INVALID = 0,
	DUCKDB_RESULT_TYPE_CHANGED_ROWS = 1,
	DUCKDB_RESULT_TYPE_NOTHING = 2,
	DUCKDB_RESULT_TYPE_QUERY_RESULT = 3,
} duckdb_result_type;
//! An enum over DuckDB's different statement types.
typedef enum {
	DUCKDB_STATEMENT_TYPE_INVALID = 0,
	DUCKDB_STATEMENT_TYPE_SELECT = 1,
	DUCKDB_STATEMENT_TYPE_INSERT = 2,
	DUCKDB_STATEMENT_TYPE_UPDATE = 3,
	DUCKDB_STATEMENT_TYPE_EXPLAIN = 4,
	DUCKDB_STATEMENT_TYPE_DELETE = 5,
	DUCKDB_STATEMENT_TYPE_PREPARE = 6,
	DUCKDB_STATEMENT_TYPE_CREATE = 7,
	DUCKDB_STATEMENT_TYPE_EXECUTE = 8,
	DUCKDB_STATEMENT_TYPE_ALTER = 9,
	DUCKDB_STATEMENT_TYPE_TRANSACTION = 10,
	DUCKDB_STATEMENT_TYPE_COPY = 11,
	DUCKDB_STATEMENT_TYPE_ANALYZE = 12,
	DUCKDB_STATEMENT_TYPE_VARIABLE_SET = 13,
	DUCKDB_STATEMENT_TYPE_CREATE_FUNC = 14,
	DUCKDB_STATEMENT_TYPE_DROP = 15,
	DUCKDB_STATEMENT_TYPE_EXPORT = 16,
	DUCKDB_STATEMENT_TYPE_PRAGMA = 17,
	DUCKDB_STATEMENT_TYPE_VACUUM = 18,
	DUCKDB_STATEMENT_TYPE_CALL = 19,
	DUCKDB_STATEMENT_TYPE_SET = 20,
	DUCKDB_STATEMENT_TYPE_LOAD = 21,
	DUCKDB_STATEMENT_TYPE_RELATION = 22,
	DUCKDB_STATEMENT_TYPE_EXTENSION = 23,
	DUCKDB_STATEMENT_TYPE_LOGICAL_PLAN = 24,
	DUCKDB_STATEMENT_TYPE_ATTACH = 25,
	DUCKDB_STATEMENT_TYPE_DETACH = 26,
	DUCKDB_STATEMENT_TYPE_MULTI = 27,
} duckdb_statement_type;

//===--------------------------------------------------------------------===//
// General type definitions
//===--------------------------------------------------------------------===//

//! DuckDB's index type.
typedef uint64_t idx_t;

//! The callback that will be called to destroy data, e.g.,
//! bind data (if any), init data (if any), extra data for replacement scans (if any)
typedef void (*duckdb_delete_callback_t)(void *data);

//! Used for threading, contains a task state. Must be destroyed with `duckdb_destroy_state`.
typedef void *duckdb_task_state;

//===--------------------------------------------------------------------===//
// Types (no explicit freeing)
//===--------------------------------------------------------------------===//

//! Days are stored as days since 1970-01-01
//! Use the duckdb_from_date/duckdb_to_date function to extract individual information
typedef struct {
	int32_t days;
} duckdb_date;
typedef struct {
	int32_t year;
	int8_t month;
	int8_t day;
} duckdb_date_struct;

//! Time is stored as microseconds since 00:00:00
//! Use the duckdb_from_time/duckdb_to_time function to extract individual information
typedef struct {
	int64_t micros;
} duckdb_time;
typedef struct {
	int8_t hour;
	int8_t min;
	int8_t sec;
	int32_t micros;
} duckdb_time_struct;

//! TIME_TZ is stored as 40 bits for int64_t micros, and 24 bits for int32_t offset
typedef struct {
	uint64_t bits;
} duckdb_time_tz;
typedef struct {
	duckdb_time_struct time;
	int32_t offset;
} duckdb_time_tz_struct;

//! Timestamps are stored as microseconds since 1970-01-01
//! Use the duckdb_from_timestamp/duckdb_to_timestamp function to extract individual information
typedef struct {
	int64_t micros;
} duckdb_timestamp;
typedef struct {
	duckdb_date_struct date;
	duckdb_time_struct time;
} duckdb_timestamp_struct;
typedef struct {
	int32_t months;
	int32_t days;
	int64_t micros;
} duckdb_interval;

//! Hugeints are composed of a (lower, upper) component
//! The value of the hugeint is upper * 2^64 + lower
//! For easy usage, the functions duckdb_hugeint_to_double/duckdb_double_to_hugeint are recommended
typedef struct {
	uint64_t lower;
	int64_t upper;
} duckdb_hugeint;
typedef struct {
	uint64_t lower;
	uint64_t upper;
} duckdb_uhugeint;

//! Decimals are composed of a width and a scale, and are stored in a hugeint
typedef struct {
	uint8_t width;
	uint8_t scale;
	duckdb_hugeint value;
} duckdb_decimal;

//! A type holding information about the query execution progress
typedef struct {
	double percentage;
	uint64_t rows_processed;
	uint64_t total_rows_to_process;
} duckdb_query_progress_type;

//! The internal representation of a VARCHAR (string_t). If the VARCHAR does not
//! exceed 12 characters, then we inline it. Otherwise, we inline a prefix for faster
//! string comparisons and store a pointer to the remaining characters. This is a non-
//! owning structure, i.e., it does not have to be freed.
typedef struct {
	union {
		struct {
			uint32_t length;
			char prefix[4];
			char *ptr;
		} pointer;
		struct {
			uint32_t length;
			char inlined[12];
		} inlined;
	} value;
} duckdb_string_t;

//! The internal representation of a list metadata entry contains the list's offset in
//! the child vector, and its length. The parent vector holds these metadata entries,
//! whereas the child vector holds the data
typedef struct {
	uint64_t offset;
	uint64_t length;
} duckdb_list_entry;

//! A column consists of a pointer to its internal data. Don't operate on this type directly.
//! Instead, use functions such as duckdb_column_data, duckdb_nullmask_data,
//! duckdb_column_type, and duckdb_column_name, which take the result and the column index
//! as their parameters
typedef struct {
#if DUCKDB_API_VERSION < DUCKDB_API_0_3_2
	void *data;
	bool *nullmask;
	duckdb_type type;
	char *name;
#else
	// deprecated, use duckdb_column_data
	void *__deprecated_data;
	// deprecated, use duckdb_nullmask_data
	bool *__deprecated_nullmask;
	// deprecated, use duckdb_column_type
	duckdb_type __deprecated_type;
	// deprecated, use duckdb_column_name
	char *__deprecated_name;
#endif
	void *internal_data;
} duckdb_column;

//! A vector to a specified column in a data chunk. Lives as long as the
//! data chunk lives, i.e., must not be destroyed.
typedef struct _duckdb_vector {
	void *__vctr;
} * duckdb_vector;

//===--------------------------------------------------------------------===//
// Types (explicit freeing/destroying)
//===--------------------------------------------------------------------===//

//! Strings are composed of a char pointer and a size. You must free string.data
//! with `duckdb_free`.
typedef struct {
	char *data;
	idx_t size;
} duckdb_string;

//! BLOBs are composed of a byte pointer and a size. You must free blob.data
//! with `duckdb_free`.
typedef struct {
	void *data;
	idx_t size;
} duckdb_blob;

//! A query result consists of a pointer to its internal data.
//! Must be freed with 'duckdb_destroy_result'.
typedef struct {
#if DUCKDB_API_VERSION < DUCKDB_API_0_3_2
	idx_t column_count;
	idx_t row_count;
	idx_t rows_changed;
	duckdb_column *columns;
	char *error_message;
#else
	// deprecated, use duckdb_column_count
	idx_t __deprecated_column_count;
	// deprecated, use duckdb_row_count
	idx_t __deprecated_row_count;
	// deprecated, use duckdb_rows_changed
	idx_t __deprecated_rows_changed;
	// deprecated, use duckdb_column_*-family of functions
	duckdb_column *__deprecated_columns;
	// deprecated, use duckdb_result_error
	char *__deprecated_error_message;
#endif
	void *internal_data;
} duckdb_result;

//! A database object. Should be closed with `duckdb_close`.
typedef struct _duckdb_database {
	void *__db;
} * duckdb_database;

//! A connection to a duckdb database. Must be closed with `duckdb_disconnect`.
typedef struct _duckdb_connection {
	void *__conn;
} * duckdb_connection;

//! A prepared statement is a parameterized query that allows you to bind parameters to it.
//! Must be destroyed with `duckdb_destroy_prepare`.
typedef struct _duckdb_prepared_statement {
	void *__prep;
} * duckdb_prepared_statement;

//! Extracted statements. Must be destroyed with `duckdb_destroy_extracted`.
typedef struct _duckdb_extracted_statements {
	void *__extrac;
} * duckdb_extracted_statements;

//! The pending result represents an intermediate structure for a query that is not yet fully executed.
//! Must be destroyed with `duckdb_destroy_pending`.
typedef struct _duckdb_pending_result {
	void *__pend;
} * duckdb_pending_result;

//! The appender enables fast data loading into DuckDB.
//! Must be destroyed with `duckdb_appender_destroy`.
typedef struct _duckdb_appender {
	void *__appn;
} * duckdb_appender;

//! Can be used to provide start-up options for the DuckDB instance.
//! Must be destroyed with `duckdb_destroy_config`.
typedef struct _duckdb_config {
	void *__cnfg;
} * duckdb_config;

//! Holds an internal logical type.
//! Must be destroyed with `duckdb_destroy_logical_type`.
typedef struct _duckdb_logical_type {
	void *__lglt;
} * duckdb_logical_type;

//! Contains a data chunk from a duckdb_result.
//! Must be destroyed with `duckdb_destroy_data_chunk`.
typedef struct _duckdb_data_chunk {
	void *__dtck;
} * duckdb_data_chunk;

//! Holds a DuckDB value, which wraps a type.
//! Must be destroyed with `duckdb_destroy_value`.
typedef struct _duckdb_value {
	void *__val;
} * duckdb_value;

//===--------------------------------------------------------------------===//
// Function types
//===--------------------------------------------------------------------===//
//! Additional function info. When setting this info, it is necessary to pass a destroy-callback function.
typedef struct _duckdb_function_info {
	void *__val;
} * duckdb_function_info;

//===--------------------------------------------------------------------===//
// Scalar function types
//===--------------------------------------------------------------------===//
//! A scalar function. Must be destroyed with `duckdb_destroy_scalar_function`.
typedef struct _duckdb_scalar_function {
	void *__val;
} * duckdb_scalar_function;

//! The main function of the scalar function.
typedef void (*duckdb_scalar_function_t)(duckdb_function_info info, duckdb_data_chunk input, duckdb_vector output);

//===--------------------------------------------------------------------===//
// Table function types
//===--------------------------------------------------------------------===//

#ifndef DUCKDB_NO_EXTENSION_FUNCTIONS
//! A table function. Must be destroyed with `duckdb_destroy_table_function`.
typedef struct _duckdb_table_function {
	void *__val;
} * duckdb_table_function;

//! The bind info of the function. When setting this info, it is necessary to pass a destroy-callback function.
typedef struct _duckdb_bind_info {
	void *__val;
} * duckdb_bind_info;

//! Additional function init info. When setting this info, it is necessary to pass a destroy-callback function.
typedef struct _duckdb_init_info {
	void *__val;
} * duckdb_init_info;

//! The bind function of the table function.
typedef void (*duckdb_table_function_bind_t)(duckdb_bind_info info);

//! The (possibly thread-local) init function of the table function.
typedef void (*duckdb_table_function_init_t)(duckdb_init_info info);

//! The main function of the table function.
typedef void (*duckdb_table_function_t)(duckdb_function_info info, duckdb_data_chunk output);

//===--------------------------------------------------------------------===//
// Replacement scan types
//===--------------------------------------------------------------------===//

//! Additional replacement scan info. When setting this info, it is necessary to pass a destroy-callback function.
typedef struct _duckdb_replacement_scan_info {
	void *__val;
} * duckdb_replacement_scan_info;

//! A replacement scan function that can be added to a database.
typedef void (*duckdb_replacement_callback_t)(duckdb_replacement_scan_info info, const char *table_name, void *data);
#endif

//===--------------------------------------------------------------------===//
// Arrow-related types
//===--------------------------------------------------------------------===//

//! Holds an arrow query result. Must be destroyed with `duckdb_destroy_arrow`.
typedef struct _duckdb_arrow {
	void *__arrw;
} * duckdb_arrow;

//! Holds an arrow array stream. Must be destroyed with `duckdb_destroy_arrow_stream`.
typedef struct _duckdb_arrow_stream {
	void *__arrwstr;
} * duckdb_arrow_stream;

//! Holds an arrow schema. Remember to release the respective ArrowSchema object.
typedef struct _duckdb_arrow_schema {
	void *__arrs;
} * duckdb_arrow_schema;

//! Holds an arrow array. Remember to release the respective ArrowArray object.
typedef struct _duckdb_arrow_array {
	void *__arra;
} * duckdb_arrow_array;

//===--------------------------------------------------------------------===//
// Functions
//===--------------------------------------------------------------------===//

typedef struct {
	// Version v0.0.1
	duckdb_state (*duckdb_open)(const char *path, duckdb_database *out_database);
	duckdb_state (*duckdb_open_ext)(const char *path, duckdb_database *out_database, duckdb_config config,
	                                char **out_error);
	void (*duckdb_close)(duckdb_database *database);
	duckdb_state (*duckdb_connect)(duckdb_database database, duckdb_connection *out_connection);
	void (*duckdb_interrupt)(duckdb_connection connection);
	duckdb_query_progress_type (*duckdb_query_progress)(duckdb_connection connection);
	void (*duckdb_disconnect)(duckdb_connection *connection);
	const char *(*duckdb_library_version)();
	duckdb_state (*duckdb_create_config)(duckdb_config *out_config);
	size_t (*duckdb_config_count)();
	duckdb_state (*duckdb_get_config_flag)(size_t index, const char **out_name, const char **out_description);
	duckdb_state (*duckdb_set_config)(duckdb_config config, const char *name, const char *option);
	void (*duckdb_destroy_config)(duckdb_config *config);
	duckdb_state (*duckdb_query)(duckdb_connection connection, const char *query, duckdb_result *out_result);
	void (*duckdb_destroy_result)(duckdb_result *result);
	const char *(*duckdb_column_name)(duckdb_result *result, idx_t col);
	duckdb_type (*duckdb_column_type)(duckdb_result *result, idx_t col);
	duckdb_statement_type (*duckdb_result_statement_type)(duckdb_result result);
	duckdb_logical_type (*duckdb_column_logical_type)(duckdb_result *result, idx_t col);
	idx_t (*duckdb_column_count)(duckdb_result *result);
	idx_t (*duckdb_row_count)(duckdb_result *result);
	idx_t (*duckdb_rows_changed)(duckdb_result *result);
	void *(*duckdb_column_data)(duckdb_result *result, idx_t col);
	bool *(*duckdb_nullmask_data)(duckdb_result *result, idx_t col);
	const char *(*duckdb_result_error)(duckdb_result *result);
	duckdb_data_chunk (*duckdb_result_get_chunk)(duckdb_result result, idx_t chunk_index);
	bool (*duckdb_result_is_streaming)(duckdb_result result);
	idx_t (*duckdb_result_chunk_count)(duckdb_result result);
	duckdb_result_type (*duckdb_result_return_type)(duckdb_result result);
	bool (*duckdb_value_boolean)(duckdb_result *result, idx_t col, idx_t row);
	int8_t (*duckdb_value_int8)(duckdb_result *result, idx_t col, idx_t row);
	int16_t (*duckdb_value_int16)(duckdb_result *result, idx_t col, idx_t row);
	int32_t (*duckdb_value_int32)(duckdb_result *result, idx_t col, idx_t row);
	int64_t (*duckdb_value_int64)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_hugeint (*duckdb_value_hugeint)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_uhugeint (*duckdb_value_uhugeint)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_decimal (*duckdb_value_decimal)(duckdb_result *result, idx_t col, idx_t row);
	uint8_t (*duckdb_value_uint8)(duckdb_result *result, idx_t col, idx_t row);
	uint16_t (*duckdb_value_uint16)(duckdb_result *result, idx_t col, idx_t row);
	uint32_t (*duckdb_value_uint32)(duckdb_result *result, idx_t col, idx_t row);
	uint64_t (*duckdb_value_uint64)(duckdb_result *result, idx_t col, idx_t row);
	float (*duckdb_value_float)(duckdb_result *result, idx_t col, idx_t row);
	double (*duckdb_value_double)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_date (*duckdb_value_date)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_time (*duckdb_value_time)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_timestamp (*duckdb_value_timestamp)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_interval (*duckdb_value_interval)(duckdb_result *result, idx_t col, idx_t row);
	char *(*duckdb_value_varchar)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_string (*duckdb_value_string)(duckdb_result *result, idx_t col, idx_t row);
	char *(*duckdb_value_varchar_internal)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_string (*duckdb_value_string_internal)(duckdb_result *result, idx_t col, idx_t row);
	duckdb_blob (*duckdb_value_blob)(duckdb_result *result, idx_t col, idx_t row);
	bool (*duckdb_value_is_null)(duckdb_result *result, idx_t col, idx_t row);
	void *(*duckdb_malloc)(size_t size);
	void (*duckdb_free)(void *ptr);
	idx_t (*duckdb_vector_size)();
	bool (*duckdb_string_is_inlined)(duckdb_string_t string);
	duckdb_date_struct (*duckdb_from_date)(duckdb_date date);
	duckdb_date (*duckdb_to_date)(duckdb_date_struct date);
	bool (*duckdb_is_finite_date)(duckdb_date date);
	duckdb_time_struct (*duckdb_from_time)(duckdb_time time);
	duckdb_time_tz (*duckdb_create_time_tz)(int64_t micros, int32_t offset);
	duckdb_time_tz_struct (*duckdb_from_time_tz)(duckdb_time_tz micros);
	duckdb_time (*duckdb_to_time)(duckdb_time_struct time);
	duckdb_timestamp_struct (*duckdb_from_timestamp)(duckdb_timestamp ts);
	duckdb_timestamp (*duckdb_to_timestamp)(duckdb_timestamp_struct ts);
	bool (*duckdb_is_finite_timestamp)(duckdb_timestamp ts);
	double (*duckdb_hugeint_to_double)(duckdb_hugeint val);
	duckdb_hugeint (*duckdb_double_to_hugeint)(double val);
	double (*duckdb_uhugeint_to_double)(duckdb_uhugeint val);
	duckdb_uhugeint (*duckdb_double_to_uhugeint)(double val);
	duckdb_decimal (*duckdb_double_to_decimal)(double val, uint8_t width, uint8_t scale);
	double (*duckdb_decimal_to_double)(duckdb_decimal val);
	duckdb_state (*duckdb_prepare)(duckdb_connection connection, const char *query,
	                               duckdb_prepared_statement *out_prepared_statement);
	void (*duckdb_destroy_prepare)(duckdb_prepared_statement *prepared_statement);
	const char *(*duckdb_prepare_error)(duckdb_prepared_statement prepared_statement);
	idx_t (*duckdb_nparams)(duckdb_prepared_statement prepared_statement);
	const char *(*duckdb_parameter_name)(duckdb_prepared_statement prepared_statement, idx_t index);
	duckdb_type (*duckdb_param_type)(duckdb_prepared_statement prepared_statement, idx_t param_idx);
	duckdb_state (*duckdb_clear_bindings)(duckdb_prepared_statement prepared_statement);
	duckdb_statement_type (*duckdb_prepared_statement_type)(duckdb_prepared_statement statement);
	duckdb_state (*duckdb_bind_value)(duckdb_prepared_statement prepared_statement, idx_t param_idx, duckdb_value val);
	duckdb_state (*duckdb_bind_parameter_index)(duckdb_prepared_statement prepared_statement, idx_t *param_idx_out,
	                                            const char *name);
	duckdb_state (*duckdb_bind_boolean)(duckdb_prepared_statement prepared_statement, idx_t param_idx, bool val);
	duckdb_state (*duckdb_bind_int8)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int8_t val);
	duckdb_state (*duckdb_bind_int16)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int16_t val);
	duckdb_state (*duckdb_bind_int32)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int32_t val);
	duckdb_state (*duckdb_bind_int64)(duckdb_prepared_statement prepared_statement, idx_t param_idx, int64_t val);
	duckdb_state (*duckdb_bind_hugeint)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                    duckdb_hugeint val);
	duckdb_state (*duckdb_bind_uhugeint)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                     duckdb_uhugeint val);
	duckdb_state (*duckdb_bind_decimal)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                    duckdb_decimal val);
	duckdb_state (*duckdb_bind_uint8)(duckdb_prepared_statement prepared_statement, idx_t param_idx, uint8_t val);
	duckdb_state (*duckdb_bind_uint16)(duckdb_prepared_statement prepared_statement, idx_t param_idx, uint16_t val);
	duckdb_state (*duckdb_bind_uint32)(duckdb_prepared_statement prepared_statement, idx_t param_idx, uint32_t val);
	duckdb_state (*duckdb_bind_uint64)(duckdb_prepared_statement prepared_statement, idx_t param_idx, uint64_t val);
	duckdb_state (*duckdb_bind_float)(duckdb_prepared_statement prepared_statement, idx_t param_idx, float val);
	duckdb_state (*duckdb_bind_double)(duckdb_prepared_statement prepared_statement, idx_t param_idx, double val);
	duckdb_state (*duckdb_bind_date)(duckdb_prepared_statement prepared_statement, idx_t param_idx, duckdb_date val);
	duckdb_state (*duckdb_bind_time)(duckdb_prepared_statement prepared_statement, idx_t param_idx, duckdb_time val);
	duckdb_state (*duckdb_bind_timestamp)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                      duckdb_timestamp val);
	duckdb_state (*duckdb_bind_timestamp_tz)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                         duckdb_timestamp val);
	duckdb_state (*duckdb_bind_interval)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                     duckdb_interval val);
	duckdb_state (*duckdb_bind_varchar)(duckdb_prepared_statement prepared_statement, idx_t param_idx, const char *val);
	duckdb_state (*duckdb_bind_varchar_length)(duckdb_prepared_statement prepared_statement, idx_t param_idx,
	                                           const char *val, idx_t length);
	duckdb_state (*duckdb_bind_blob)(duckdb_prepared_statement prepared_statement, idx_t param_idx, const void *data,
	                                 idx_t length);
	duckdb_state (*duckdb_bind_null)(duckdb_prepared_statement prepared_statement, idx_t param_idx);
	duckdb_state (*duckdb_execute_prepared)(duckdb_prepared_statement prepared_statement, duckdb_result *out_result);
	duckdb_state (*duckdb_execute_prepared_streaming)(duckdb_prepared_statement prepared_statement,
	                                                  duckdb_result *out_result);
	idx_t (*duckdb_extract_statements)(duckdb_connection connection, const char *query,
	                                   duckdb_extracted_statements *out_extracted_statements);
	duckdb_state (*duckdb_prepare_extracted_statement)(duckdb_connection connection,
	                                                   duckdb_extracted_statements extracted_statements, idx_t index,
	                                                   duckdb_prepared_statement *out_prepared_statement);
	const char *(*duckdb_extract_statements_error)(duckdb_extracted_statements extracted_statements);
	void (*duckdb_destroy_extracted)(duckdb_extracted_statements *extracted_statements);
	duckdb_state (*duckdb_pending_prepared)(duckdb_prepared_statement prepared_statement,
	                                        duckdb_pending_result *out_result);
	duckdb_state (*duckdb_pending_prepared_streaming)(duckdb_prepared_statement prepared_statement,
	                                                  duckdb_pending_result *out_result);
	void (*duckdb_destroy_pending)(duckdb_pending_result *pending_result);
	const char *(*duckdb_pending_error)(duckdb_pending_result pending_result);
	duckdb_pending_state (*duckdb_pending_execute_task)(duckdb_pending_result pending_result);
	duckdb_pending_state (*duckdb_pending_execute_check_state)(duckdb_pending_result pending_result);
	duckdb_state (*duckdb_execute_pending)(duckdb_pending_result pending_result, duckdb_result *out_result);
	bool (*duckdb_pending_execution_is_finished)(duckdb_pending_state pending_state);
	void (*duckdb_destroy_value)(duckdb_value *value);
	duckdb_value (*duckdb_create_varchar)(const char *text);
	duckdb_value (*duckdb_create_varchar_length)(const char *text, idx_t length);
	duckdb_value (*duckdb_create_int64)(int64_t val);
	duckdb_value (*duckdb_create_struct_value)(duckdb_logical_type type, duckdb_value *values);
	duckdb_value (*duckdb_create_list_value)(duckdb_logical_type type, duckdb_value *values, idx_t value_count);
	duckdb_value (*duckdb_create_array_value)(duckdb_logical_type type, duckdb_value *values, idx_t value_count);
	char *(*duckdb_get_varchar)(duckdb_value value);
	int64_t (*duckdb_get_int64)(duckdb_value value);
	duckdb_logical_type (*duckdb_create_logical_type)(duckdb_type type);
	char *(*duckdb_logical_type_get_alias)(duckdb_logical_type type);
	duckdb_logical_type (*duckdb_create_list_type)(duckdb_logical_type type);
	duckdb_logical_type (*duckdb_create_array_type)(duckdb_logical_type type, idx_t array_size);
	duckdb_logical_type (*duckdb_create_map_type)(duckdb_logical_type key_type, duckdb_logical_type value_type);
	duckdb_logical_type (*duckdb_create_union_type)(duckdb_logical_type *member_types, const char **member_names,
	                                                idx_t member_count);
	duckdb_logical_type (*duckdb_create_struct_type)(duckdb_logical_type *member_types, const char **member_names,
	                                                 idx_t member_count);
	duckdb_logical_type (*duckdb_create_enum_type)(const char **member_names, idx_t member_count);
	duckdb_logical_type (*duckdb_create_decimal_type)(uint8_t width, uint8_t scale);
	duckdb_type (*duckdb_get_type_id)(duckdb_logical_type type);
	uint8_t (*duckdb_decimal_width)(duckdb_logical_type type);
	uint8_t (*duckdb_decimal_scale)(duckdb_logical_type type);
	duckdb_type (*duckdb_decimal_internal_type)(duckdb_logical_type type);
	duckdb_type (*duckdb_enum_internal_type)(duckdb_logical_type type);
	uint32_t (*duckdb_enum_dictionary_size)(duckdb_logical_type type);
	char *(*duckdb_enum_dictionary_value)(duckdb_logical_type type, idx_t index);
	duckdb_logical_type (*duckdb_list_type_child_type)(duckdb_logical_type type);
	duckdb_logical_type (*duckdb_array_type_child_type)(duckdb_logical_type type);
	idx_t (*duckdb_array_type_array_size)(duckdb_logical_type type);
	duckdb_logical_type (*duckdb_map_type_key_type)(duckdb_logical_type type);
	duckdb_logical_type (*duckdb_map_type_value_type)(duckdb_logical_type type);
	idx_t (*duckdb_struct_type_child_count)(duckdb_logical_type type);
	char *(*duckdb_struct_type_child_name)(duckdb_logical_type type, idx_t index);
	duckdb_logical_type (*duckdb_struct_type_child_type)(duckdb_logical_type type, idx_t index);
	idx_t (*duckdb_union_type_member_count)(duckdb_logical_type type);
	char *(*duckdb_union_type_member_name)(duckdb_logical_type type, idx_t index);
	duckdb_logical_type (*duckdb_union_type_member_type)(duckdb_logical_type type, idx_t index);
	void (*duckdb_destroy_logical_type)(duckdb_logical_type *type);
	duckdb_data_chunk (*duckdb_create_data_chunk)(duckdb_logical_type *types, idx_t column_count);
	void (*duckdb_destroy_data_chunk)(duckdb_data_chunk *chunk);
	void (*duckdb_data_chunk_reset)(duckdb_data_chunk chunk);
	idx_t (*duckdb_data_chunk_get_column_count)(duckdb_data_chunk chunk);
	duckdb_vector (*duckdb_data_chunk_get_vector)(duckdb_data_chunk chunk, idx_t col_idx);
	idx_t (*duckdb_data_chunk_get_size)(duckdb_data_chunk chunk);
	void (*duckdb_data_chunk_set_size)(duckdb_data_chunk chunk, idx_t size);
	duckdb_logical_type (*duckdb_vector_get_column_type)(duckdb_vector vector);
	void *(*duckdb_vector_get_data)(duckdb_vector vector);
	uint64_t *(*duckdb_vector_get_validity)(duckdb_vector vector);
	void (*duckdb_vector_ensure_validity_writable)(duckdb_vector vector);
	void (*duckdb_vector_assign_string_element)(duckdb_vector vector, idx_t index, const char *str);
	void (*duckdb_vector_assign_string_element_len)(duckdb_vector vector, idx_t index, const char *str, idx_t str_len);
	duckdb_vector (*duckdb_list_vector_get_child)(duckdb_vector vector);
	idx_t (*duckdb_list_vector_get_size)(duckdb_vector vector);
	duckdb_state (*duckdb_list_vector_set_size)(duckdb_vector vector, idx_t size);
	duckdb_state (*duckdb_list_vector_reserve)(duckdb_vector vector, idx_t required_capacity);
	duckdb_vector (*duckdb_struct_vector_get_child)(duckdb_vector vector, idx_t index);
	duckdb_vector (*duckdb_array_vector_get_child)(duckdb_vector vector);
	bool (*duckdb_validity_row_is_valid)(uint64_t *validity, idx_t row);
	void (*duckdb_validity_set_row_validity)(uint64_t *validity, idx_t row, bool valid);
	void (*duckdb_validity_set_row_invalid)(uint64_t *validity, idx_t row);
	void (*duckdb_validity_set_row_valid)(uint64_t *validity, idx_t row);
	duckdb_scalar_function (*duckdb_create_scalar_function)();
	void (*duckdb_destroy_scalar_function)(duckdb_scalar_function *scalar_function);
	void (*duckdb_scalar_function_set_name)(duckdb_scalar_function scalar_function, const char *name);
	void (*duckdb_scalar_function_add_parameter)(duckdb_scalar_function scalar_function, duckdb_logical_type type);
	void (*duckdb_scalar_function_set_return_type)(duckdb_scalar_function scalar_function, duckdb_logical_type type);
	void (*duckdb_scalar_function_set_extra_info)(duckdb_scalar_function scalar_function, void *extra_info,
	                                              duckdb_delete_callback_t destroy);
	void (*duckdb_scalar_function_set_function)(duckdb_scalar_function scalar_function,
	                                            duckdb_scalar_function_t function);
	duckdb_state (*duckdb_register_scalar_function)(duckdb_connection con, duckdb_scalar_function scalar_function);
	duckdb_table_function (*duckdb_create_table_function)();
	void (*duckdb_destroy_table_function)(duckdb_table_function *table_function);
	void (*duckdb_table_function_set_name)(duckdb_table_function table_function, const char *name);
	void (*duckdb_table_function_add_parameter)(duckdb_table_function table_function, duckdb_logical_type type);
	void (*duckdb_table_function_add_named_parameter)(duckdb_table_function table_function, const char *name,
	                                                  duckdb_logical_type type);
	void (*duckdb_table_function_set_extra_info)(duckdb_table_function table_function, void *extra_info,
	                                             duckdb_delete_callback_t destroy);
	void (*duckdb_table_function_set_bind)(duckdb_table_function table_function, duckdb_table_function_bind_t bind);
	void (*duckdb_table_function_set_init)(duckdb_table_function table_function, duckdb_table_function_init_t init);
	void (*duckdb_table_function_set_local_init)(duckdb_table_function table_function,
	                                             duckdb_table_function_init_t init);
	void (*duckdb_table_function_set_function)(duckdb_table_function table_function, duckdb_table_function_t function);
	void (*duckdb_table_function_supports_projection_pushdown)(duckdb_table_function table_function, bool pushdown);
	duckdb_state (*duckdb_register_table_function)(duckdb_connection con, duckdb_table_function function);
	void *(*duckdb_bind_get_extra_info)(duckdb_bind_info info);
	void (*duckdb_bind_add_result_column)(duckdb_bind_info info, const char *name, duckdb_logical_type type);
	idx_t (*duckdb_bind_get_parameter_count)(duckdb_bind_info info);
	duckdb_value (*duckdb_bind_get_parameter)(duckdb_bind_info info, idx_t index);
	duckdb_value (*duckdb_bind_get_named_parameter)(duckdb_bind_info info, const char *name);
	void (*duckdb_bind_set_bind_data)(duckdb_bind_info info, void *bind_data, duckdb_delete_callback_t destroy);
	void (*duckdb_bind_set_cardinality)(duckdb_bind_info info, idx_t cardinality, bool is_exact);
	void (*duckdb_bind_set_error)(duckdb_bind_info info, const char *error);
	void *(*duckdb_init_get_extra_info)(duckdb_init_info info);
	void *(*duckdb_init_get_bind_data)(duckdb_init_info info);
	void (*duckdb_init_set_init_data)(duckdb_init_info info, void *init_data, duckdb_delete_callback_t destroy);
	idx_t (*duckdb_init_get_column_count)(duckdb_init_info info);
	idx_t (*duckdb_init_get_column_index)(duckdb_init_info info, idx_t column_index);
	void (*duckdb_init_set_max_threads)(duckdb_init_info info, idx_t max_threads);
	void (*duckdb_init_set_error)(duckdb_init_info info, const char *error);
	void *(*duckdb_function_get_extra_info)(duckdb_function_info info);
	void *(*duckdb_function_get_bind_data)(duckdb_function_info info);
	void *(*duckdb_function_get_init_data)(duckdb_function_info info);
	void *(*duckdb_function_get_local_init_data)(duckdb_function_info info);
	void (*duckdb_function_set_error)(duckdb_function_info info, const char *error);
	void (*duckdb_add_replacement_scan)(duckdb_database db, duckdb_replacement_callback_t replacement, void *extra_data,
	                                    duckdb_delete_callback_t delete_callback);
	void (*duckdb_replacement_scan_set_function_name)(duckdb_replacement_scan_info info, const char *function_name);
	void (*duckdb_replacement_scan_add_parameter)(duckdb_replacement_scan_info info, duckdb_value parameter);
	void (*duckdb_replacement_scan_set_error)(duckdb_replacement_scan_info info, const char *error);
	duckdb_state (*duckdb_appender_create)(duckdb_connection connection, const char *schema, const char *table,
	                                       duckdb_appender *out_appender);
	idx_t (*duckdb_appender_column_count)(duckdb_appender appender);
	duckdb_logical_type (*duckdb_appender_column_type)(duckdb_appender appender, idx_t col_idx);
	const char *(*duckdb_appender_error)(duckdb_appender appender);
	duckdb_state (*duckdb_appender_flush)(duckdb_appender appender);
	duckdb_state (*duckdb_appender_close)(duckdb_appender appender);
	duckdb_state (*duckdb_appender_destroy)(duckdb_appender *appender);
	duckdb_state (*duckdb_appender_begin_row)(duckdb_appender appender);
	duckdb_state (*duckdb_appender_end_row)(duckdb_appender appender);
	duckdb_state (*duckdb_append_default)(duckdb_appender appender);
	duckdb_state (*duckdb_append_bool)(duckdb_appender appender, bool value);
	duckdb_state (*duckdb_append_int8)(duckdb_appender appender, int8_t value);
	duckdb_state (*duckdb_append_int16)(duckdb_appender appender, int16_t value);
	duckdb_state (*duckdb_append_int32)(duckdb_appender appender, int32_t value);
	duckdb_state (*duckdb_append_int64)(duckdb_appender appender, int64_t value);
	duckdb_state (*duckdb_append_hugeint)(duckdb_appender appender, duckdb_hugeint value);
	duckdb_state (*duckdb_append_uint8)(duckdb_appender appender, uint8_t value);
	duckdb_state (*duckdb_append_uint16)(duckdb_appender appender, uint16_t value);
	duckdb_state (*duckdb_append_uint32)(duckdb_appender appender, uint32_t value);
	duckdb_state (*duckdb_append_uint64)(duckdb_appender appender, uint64_t value);
	duckdb_state (*duckdb_append_uhugeint)(duckdb_appender appender, duckdb_uhugeint value);
	duckdb_state (*duckdb_append_float)(duckdb_appender appender, float value);
	duckdb_state (*duckdb_append_double)(duckdb_appender appender, double value);
	duckdb_state (*duckdb_append_date)(duckdb_appender appender, duckdb_date value);
	duckdb_state (*duckdb_append_time)(duckdb_appender appender, duckdb_time value);
	duckdb_state (*duckdb_append_timestamp)(duckdb_appender appender, duckdb_timestamp value);
	duckdb_state (*duckdb_append_interval)(duckdb_appender appender, duckdb_interval value);
	duckdb_state (*duckdb_append_varchar)(duckdb_appender appender, const char *val);
	duckdb_state (*duckdb_append_varchar_length)(duckdb_appender appender, const char *val, idx_t length);
	duckdb_state (*duckdb_append_blob)(duckdb_appender appender, const void *data, idx_t length);
	duckdb_state (*duckdb_append_null)(duckdb_appender appender);
	duckdb_state (*duckdb_append_data_chunk)(duckdb_appender appender, duckdb_data_chunk chunk);
	duckdb_state (*duckdb_query_arrow)(duckdb_connection connection, const char *query, duckdb_arrow *out_result);
	duckdb_state (*duckdb_query_arrow_schema)(duckdb_arrow result, duckdb_arrow_schema *out_schema);
	duckdb_state (*duckdb_prepared_arrow_schema)(duckdb_prepared_statement prepared, duckdb_arrow_schema *out_schema);
	void (*duckdb_result_arrow_array)(duckdb_result result, duckdb_data_chunk chunk, duckdb_arrow_array *out_array);
	duckdb_state (*duckdb_query_arrow_array)(duckdb_arrow result, duckdb_arrow_array *out_array);
	idx_t (*duckdb_arrow_column_count)(duckdb_arrow result);
	idx_t (*duckdb_arrow_row_count)(duckdb_arrow result);
	idx_t (*duckdb_arrow_rows_changed)(duckdb_arrow result);
	const char *(*duckdb_query_arrow_error)(duckdb_arrow result);
	void (*duckdb_destroy_arrow)(duckdb_arrow *result);
	void (*duckdb_destroy_arrow_stream)(duckdb_arrow_stream *stream_p);
	duckdb_state (*duckdb_execute_prepared_arrow)(duckdb_prepared_statement prepared_statement,
	                                              duckdb_arrow *out_result);
	duckdb_state (*duckdb_arrow_scan)(duckdb_connection connection, const char *table_name, duckdb_arrow_stream arrow);
	duckdb_state (*duckdb_arrow_array_scan)(duckdb_connection connection, const char *table_name,
	                                        duckdb_arrow_schema arrow_schema, duckdb_arrow_array arrow_array,
	                                        duckdb_arrow_stream *out_stream);
	void (*duckdb_execute_tasks)(duckdb_database database, idx_t max_tasks);
	duckdb_task_state (*duckdb_create_task_state)(duckdb_database database);
	void (*duckdb_execute_tasks_state)(duckdb_task_state state);
	idx_t (*duckdb_execute_n_tasks_state)(duckdb_task_state state, idx_t max_tasks);
	void (*duckdb_finish_execution)(duckdb_task_state state);
	bool (*duckdb_task_state_is_finished)(duckdb_task_state state);
	void (*duckdb_destroy_task_state)(duckdb_task_state state);
	bool (*duckdb_execution_is_finished)(duckdb_connection con);
	duckdb_data_chunk (*duckdb_stream_fetch_chunk)(duckdb_result result);
	duckdb_data_chunk (*duckdb_fetch_chunk)(duckdb_result result);
} duckdb_ext_api_v0;

//! table_functions
#define duckdb_create_table_function              duckdb_ext_api->duckdb_create_table_function
#define duckdb_destroy_table_function             duckdb_ext_api->duckdb_destroy_table_function
#define duckdb_table_function_set_name            duckdb_ext_api->duckdb_table_function_set_name
#define duckdb_table_function_add_parameter       duckdb_ext_api->duckdb_table_function_add_parameter
#define duckdb_table_function_add_named_parameter duckdb_ext_api->duckdb_table_function_add_named_parameter
#define duckdb_table_function_set_extra_info      duckdb_ext_api->duckdb_table_function_set_extra_info
#define duckdb_table_function_set_bind            duckdb_ext_api->duckdb_table_function_set_bind
#define duckdb_table_function_set_init            duckdb_ext_api->duckdb_table_function_set_init
#define duckdb_table_function_set_local_init      duckdb_ext_api->duckdb_table_function_set_local_init
#define duckdb_table_function_set_function        duckdb_ext_api->duckdb_table_function_set_function
#define duckdb_table_function_supports_projection_pushdown                                                             \
	duckdb_ext_api->duckdb_table_function_supports_projection_pushdown
#define duckdb_register_table_function duckdb_ext_api->duckdb_register_table_function

//! pending_result_interface
#define duckdb_pending_prepared duckdb_ext_api->duckdb_pending_prepared
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_pending_prepared_streaming duckdb_ext_api->duckdb_pending_prepared_streaming
#endif
#define duckdb_destroy_pending               duckdb_ext_api->duckdb_destroy_pending
#define duckdb_pending_error                 duckdb_ext_api->duckdb_pending_error
#define duckdb_pending_execute_task          duckdb_ext_api->duckdb_pending_execute_task
#define duckdb_pending_execute_check_state   duckdb_ext_api->duckdb_pending_execute_check_state
#define duckdb_execute_pending               duckdb_ext_api->duckdb_execute_pending
#define duckdb_pending_execution_is_finished duckdb_ext_api->duckdb_pending_execution_is_finished

//! prepared_statements
#define duckdb_prepare                 duckdb_ext_api->duckdb_prepare
#define duckdb_destroy_prepare         duckdb_ext_api->duckdb_destroy_prepare
#define duckdb_prepare_error           duckdb_ext_api->duckdb_prepare_error
#define duckdb_nparams                 duckdb_ext_api->duckdb_nparams
#define duckdb_parameter_name          duckdb_ext_api->duckdb_parameter_name
#define duckdb_param_type              duckdb_ext_api->duckdb_param_type
#define duckdb_clear_bindings          duckdb_ext_api->duckdb_clear_bindings
#define duckdb_prepared_statement_type duckdb_ext_api->duckdb_prepared_statement_type

//! safe_fetch_functions
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_value_boolean          duckdb_ext_api->duckdb_value_boolean
#define duckdb_value_int8             duckdb_ext_api->duckdb_value_int8
#define duckdb_value_int16            duckdb_ext_api->duckdb_value_int16
#define duckdb_value_int32            duckdb_ext_api->duckdb_value_int32
#define duckdb_value_int64            duckdb_ext_api->duckdb_value_int64
#define duckdb_value_hugeint          duckdb_ext_api->duckdb_value_hugeint
#define duckdb_value_uhugeint         duckdb_ext_api->duckdb_value_uhugeint
#define duckdb_value_decimal          duckdb_ext_api->duckdb_value_decimal
#define duckdb_value_uint8            duckdb_ext_api->duckdb_value_uint8
#define duckdb_value_uint16           duckdb_ext_api->duckdb_value_uint16
#define duckdb_value_uint32           duckdb_ext_api->duckdb_value_uint32
#define duckdb_value_uint64           duckdb_ext_api->duckdb_value_uint64
#define duckdb_value_float            duckdb_ext_api->duckdb_value_float
#define duckdb_value_double           duckdb_ext_api->duckdb_value_double
#define duckdb_value_date             duckdb_ext_api->duckdb_value_date
#define duckdb_value_time             duckdb_ext_api->duckdb_value_time
#define duckdb_value_timestamp        duckdb_ext_api->duckdb_value_timestamp
#define duckdb_value_interval         duckdb_ext_api->duckdb_value_interval
#define duckdb_value_varchar          duckdb_ext_api->duckdb_value_varchar
#define duckdb_value_string           duckdb_ext_api->duckdb_value_string
#define duckdb_value_varchar_internal duckdb_ext_api->duckdb_value_varchar_internal
#define duckdb_value_string_internal  duckdb_ext_api->duckdb_value_string_internal
#define duckdb_value_blob             duckdb_ext_api->duckdb_value_blob
#define duckdb_value_is_null          duckdb_ext_api->duckdb_value_is_null
#endif

//! value_interface
#define duckdb_destroy_value         duckdb_ext_api->duckdb_destroy_value
#define duckdb_create_varchar        duckdb_ext_api->duckdb_create_varchar
#define duckdb_create_varchar_length duckdb_ext_api->duckdb_create_varchar_length
#define duckdb_create_int64          duckdb_ext_api->duckdb_create_int64
#define duckdb_create_struct_value   duckdb_ext_api->duckdb_create_struct_value
#define duckdb_create_list_value     duckdb_ext_api->duckdb_create_list_value
#define duckdb_create_array_value    duckdb_ext_api->duckdb_create_array_value
#define duckdb_get_varchar           duckdb_ext_api->duckdb_get_varchar
#define duckdb_get_int64             duckdb_ext_api->duckdb_get_int64

//! date_time_timestamp_helpers
#define duckdb_from_date           duckdb_ext_api->duckdb_from_date
#define duckdb_to_date             duckdb_ext_api->duckdb_to_date
#define duckdb_is_finite_date      duckdb_ext_api->duckdb_is_finite_date
#define duckdb_from_time           duckdb_ext_api->duckdb_from_time
#define duckdb_create_time_tz      duckdb_ext_api->duckdb_create_time_tz
#define duckdb_from_time_tz        duckdb_ext_api->duckdb_from_time_tz
#define duckdb_to_time             duckdb_ext_api->duckdb_to_time
#define duckdb_from_timestamp      duckdb_ext_api->duckdb_from_timestamp
#define duckdb_to_timestamp        duckdb_ext_api->duckdb_to_timestamp
#define duckdb_is_finite_timestamp duckdb_ext_api->duckdb_is_finite_timestamp

//! streaming_result_interface
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_stream_fetch_chunk duckdb_ext_api->duckdb_stream_fetch_chunk
#endif
#define duckdb_fetch_chunk duckdb_ext_api->duckdb_fetch_chunk

//! table_function_init
#define duckdb_init_get_extra_info   duckdb_ext_api->duckdb_init_get_extra_info
#define duckdb_init_get_bind_data    duckdb_ext_api->duckdb_init_get_bind_data
#define duckdb_init_set_init_data    duckdb_ext_api->duckdb_init_set_init_data
#define duckdb_init_get_column_count duckdb_ext_api->duckdb_init_get_column_count
#define duckdb_init_get_column_index duckdb_ext_api->duckdb_init_get_column_index
#define duckdb_init_set_max_threads  duckdb_ext_api->duckdb_init_set_max_threads
#define duckdb_init_set_error        duckdb_ext_api->duckdb_init_set_error

//! result_functions
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_result_get_chunk duckdb_ext_api->duckdb_result_get_chunk
#endif
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_result_is_streaming duckdb_ext_api->duckdb_result_is_streaming
#endif
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_result_chunk_count duckdb_ext_api->duckdb_result_chunk_count
#endif
#define duckdb_result_return_type duckdb_ext_api->duckdb_result_return_type

//! execute_prepared_statements
#define duckdb_execute_prepared duckdb_ext_api->duckdb_execute_prepared
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_execute_prepared_streaming duckdb_ext_api->duckdb_execute_prepared_streaming
#endif

//! arrow_interface
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_query_arrow            duckdb_ext_api->duckdb_query_arrow
#define duckdb_query_arrow_schema     duckdb_ext_api->duckdb_query_arrow_schema
#define duckdb_prepared_arrow_schema  duckdb_ext_api->duckdb_prepared_arrow_schema
#define duckdb_result_arrow_array     duckdb_ext_api->duckdb_result_arrow_array
#define duckdb_query_arrow_array      duckdb_ext_api->duckdb_query_arrow_array
#define duckdb_arrow_column_count     duckdb_ext_api->duckdb_arrow_column_count
#define duckdb_arrow_row_count        duckdb_ext_api->duckdb_arrow_row_count
#define duckdb_arrow_rows_changed     duckdb_ext_api->duckdb_arrow_rows_changed
#define duckdb_query_arrow_error      duckdb_ext_api->duckdb_query_arrow_error
#define duckdb_destroy_arrow          duckdb_ext_api->duckdb_destroy_arrow
#define duckdb_destroy_arrow_stream   duckdb_ext_api->duckdb_destroy_arrow_stream
#define duckdb_execute_prepared_arrow duckdb_ext_api->duckdb_execute_prepared_arrow
#define duckdb_arrow_scan             duckdb_ext_api->duckdb_arrow_scan
#define duckdb_arrow_array_scan       duckdb_ext_api->duckdb_arrow_array_scan
#endif

//! table_function_bind
#define duckdb_bind_get_extra_info      duckdb_ext_api->duckdb_bind_get_extra_info
#define duckdb_bind_add_result_column   duckdb_ext_api->duckdb_bind_add_result_column
#define duckdb_bind_get_parameter_count duckdb_ext_api->duckdb_bind_get_parameter_count
#define duckdb_bind_get_parameter       duckdb_ext_api->duckdb_bind_get_parameter
#define duckdb_bind_get_named_parameter duckdb_ext_api->duckdb_bind_get_named_parameter
#define duckdb_bind_set_bind_data       duckdb_ext_api->duckdb_bind_set_bind_data
#define duckdb_bind_set_cardinality     duckdb_ext_api->duckdb_bind_set_cardinality
#define duckdb_bind_set_error           duckdb_ext_api->duckdb_bind_set_error

//! table_function
#define duckdb_function_get_extra_info      duckdb_ext_api->duckdb_function_get_extra_info
#define duckdb_function_get_bind_data       duckdb_ext_api->duckdb_function_get_bind_data
#define duckdb_function_get_init_data       duckdb_ext_api->duckdb_function_get_init_data
#define duckdb_function_get_local_init_data duckdb_ext_api->duckdb_function_get_local_init_data
#define duckdb_function_set_error           duckdb_ext_api->duckdb_function_set_error

//! data_chunk_interface
#define duckdb_create_data_chunk           duckdb_ext_api->duckdb_create_data_chunk
#define duckdb_destroy_data_chunk          duckdb_ext_api->duckdb_destroy_data_chunk
#define duckdb_data_chunk_reset            duckdb_ext_api->duckdb_data_chunk_reset
#define duckdb_data_chunk_get_column_count duckdb_ext_api->duckdb_data_chunk_get_column_count
#define duckdb_data_chunk_get_vector       duckdb_ext_api->duckdb_data_chunk_get_vector
#define duckdb_data_chunk_get_size         duckdb_ext_api->duckdb_data_chunk_get_size
#define duckdb_data_chunk_set_size         duckdb_ext_api->duckdb_data_chunk_set_size

//! logical_type_interface
#define duckdb_create_logical_type     duckdb_ext_api->duckdb_create_logical_type
#define duckdb_logical_type_get_alias  duckdb_ext_api->duckdb_logical_type_get_alias
#define duckdb_create_list_type        duckdb_ext_api->duckdb_create_list_type
#define duckdb_create_array_type       duckdb_ext_api->duckdb_create_array_type
#define duckdb_create_map_type         duckdb_ext_api->duckdb_create_map_type
#define duckdb_create_union_type       duckdb_ext_api->duckdb_create_union_type
#define duckdb_create_struct_type      duckdb_ext_api->duckdb_create_struct_type
#define duckdb_create_enum_type        duckdb_ext_api->duckdb_create_enum_type
#define duckdb_create_decimal_type     duckdb_ext_api->duckdb_create_decimal_type
#define duckdb_get_type_id             duckdb_ext_api->duckdb_get_type_id
#define duckdb_decimal_width           duckdb_ext_api->duckdb_decimal_width
#define duckdb_decimal_scale           duckdb_ext_api->duckdb_decimal_scale
#define duckdb_decimal_internal_type   duckdb_ext_api->duckdb_decimal_internal_type
#define duckdb_enum_internal_type      duckdb_ext_api->duckdb_enum_internal_type
#define duckdb_enum_dictionary_size    duckdb_ext_api->duckdb_enum_dictionary_size
#define duckdb_enum_dictionary_value   duckdb_ext_api->duckdb_enum_dictionary_value
#define duckdb_list_type_child_type    duckdb_ext_api->duckdb_list_type_child_type
#define duckdb_array_type_child_type   duckdb_ext_api->duckdb_array_type_child_type
#define duckdb_array_type_array_size   duckdb_ext_api->duckdb_array_type_array_size
#define duckdb_map_type_key_type       duckdb_ext_api->duckdb_map_type_key_type
#define duckdb_map_type_value_type     duckdb_ext_api->duckdb_map_type_value_type
#define duckdb_struct_type_child_count duckdb_ext_api->duckdb_struct_type_child_count
#define duckdb_struct_type_child_name  duckdb_ext_api->duckdb_struct_type_child_name
#define duckdb_struct_type_child_type  duckdb_ext_api->duckdb_struct_type_child_type
#define duckdb_union_type_member_count duckdb_ext_api->duckdb_union_type_member_count
#define duckdb_union_type_member_name  duckdb_ext_api->duckdb_union_type_member_name
#define duckdb_union_type_member_type  duckdb_ext_api->duckdb_union_type_member_type
#define duckdb_destroy_logical_type    duckdb_ext_api->duckdb_destroy_logical_type

//! vector_interface
#define duckdb_vector_get_column_type           duckdb_ext_api->duckdb_vector_get_column_type
#define duckdb_vector_get_data                  duckdb_ext_api->duckdb_vector_get_data
#define duckdb_vector_get_validity              duckdb_ext_api->duckdb_vector_get_validity
#define duckdb_vector_ensure_validity_writable  duckdb_ext_api->duckdb_vector_ensure_validity_writable
#define duckdb_vector_assign_string_element     duckdb_ext_api->duckdb_vector_assign_string_element
#define duckdb_vector_assign_string_element_len duckdb_ext_api->duckdb_vector_assign_string_element_len
#define duckdb_list_vector_get_child            duckdb_ext_api->duckdb_list_vector_get_child
#define duckdb_list_vector_get_size             duckdb_ext_api->duckdb_list_vector_get_size
#define duckdb_list_vector_set_size             duckdb_ext_api->duckdb_list_vector_set_size
#define duckdb_list_vector_reserve              duckdb_ext_api->duckdb_list_vector_reserve
#define duckdb_struct_vector_get_child          duckdb_ext_api->duckdb_struct_vector_get_child
#define duckdb_array_vector_get_child           duckdb_ext_api->duckdb_array_vector_get_child

//! unsigned_hugeint_helpers
#define duckdb_uhugeint_to_double duckdb_ext_api->duckdb_uhugeint_to_double
#define duckdb_double_to_uhugeint duckdb_ext_api->duckdb_double_to_uhugeint

//! query_execution
#define duckdb_query                 duckdb_ext_api->duckdb_query
#define duckdb_destroy_result        duckdb_ext_api->duckdb_destroy_result
#define duckdb_column_name           duckdb_ext_api->duckdb_column_name
#define duckdb_column_type           duckdb_ext_api->duckdb_column_type
#define duckdb_result_statement_type duckdb_ext_api->duckdb_result_statement_type
#define duckdb_column_logical_type   duckdb_ext_api->duckdb_column_logical_type
#define duckdb_column_count          duckdb_ext_api->duckdb_column_count
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_row_count duckdb_ext_api->duckdb_row_count
#endif
#define duckdb_rows_changed duckdb_ext_api->duckdb_rows_changed
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_column_data duckdb_ext_api->duckdb_column_data
#endif
#ifndef DUCKDB_API_NO_DEPRECATED
#define duckdb_nullmask_data duckdb_ext_api->duckdb_nullmask_data
#endif
#define duckdb_result_error duckdb_ext_api->duckdb_result_error

//! threading_information
#define duckdb_execute_tasks          duckdb_ext_api->duckdb_execute_tasks
#define duckdb_create_task_state      duckdb_ext_api->duckdb_create_task_state
#define duckdb_execute_tasks_state    duckdb_ext_api->duckdb_execute_tasks_state
#define duckdb_execute_n_tasks_state  duckdb_ext_api->duckdb_execute_n_tasks_state
#define duckdb_finish_execution       duckdb_ext_api->duckdb_finish_execution
#define duckdb_task_state_is_finished duckdb_ext_api->duckdb_task_state_is_finished
#define duckdb_destroy_task_state     duckdb_ext_api->duckdb_destroy_task_state
#define duckdb_execution_is_finished  duckdb_ext_api->duckdb_execution_is_finished

//! bind_values_to_prepared_statements
#define duckdb_bind_value           duckdb_ext_api->duckdb_bind_value
#define duckdb_bind_parameter_index duckdb_ext_api->duckdb_bind_parameter_index
#define duckdb_bind_boolean         duckdb_ext_api->duckdb_bind_boolean
#define duckdb_bind_int8            duckdb_ext_api->duckdb_bind_int8
#define duckdb_bind_int16           duckdb_ext_api->duckdb_bind_int16
#define duckdb_bind_int32           duckdb_ext_api->duckdb_bind_int32
#define duckdb_bind_int64           duckdb_ext_api->duckdb_bind_int64
#define duckdb_bind_hugeint         duckdb_ext_api->duckdb_bind_hugeint
#define duckdb_bind_uhugeint        duckdb_ext_api->duckdb_bind_uhugeint
#define duckdb_bind_decimal         duckdb_ext_api->duckdb_bind_decimal
#define duckdb_bind_uint8           duckdb_ext_api->duckdb_bind_uint8
#define duckdb_bind_uint16          duckdb_ext_api->duckdb_bind_uint16
#define duckdb_bind_uint32          duckdb_ext_api->duckdb_bind_uint32
#define duckdb_bind_uint64          duckdb_ext_api->duckdb_bind_uint64
#define duckdb_bind_float           duckdb_ext_api->duckdb_bind_float
#define duckdb_bind_double          duckdb_ext_api->duckdb_bind_double
#define duckdb_bind_date            duckdb_ext_api->duckdb_bind_date
#define duckdb_bind_time            duckdb_ext_api->duckdb_bind_time
#define duckdb_bind_timestamp       duckdb_ext_api->duckdb_bind_timestamp
#define duckdb_bind_timestamp_tz    duckdb_ext_api->duckdb_bind_timestamp_tz
#define duckdb_bind_interval        duckdb_ext_api->duckdb_bind_interval
#define duckdb_bind_varchar         duckdb_ext_api->duckdb_bind_varchar
#define duckdb_bind_varchar_length  duckdb_ext_api->duckdb_bind_varchar_length
#define duckdb_bind_blob            duckdb_ext_api->duckdb_bind_blob
#define duckdb_bind_null            duckdb_ext_api->duckdb_bind_null

//! configuration
#define duckdb_create_config   duckdb_ext_api->duckdb_create_config
#define duckdb_config_count    duckdb_ext_api->duckdb_config_count
#define duckdb_get_config_flag duckdb_ext_api->duckdb_get_config_flag
#define duckdb_set_config      duckdb_ext_api->duckdb_set_config
#define duckdb_destroy_config  duckdb_ext_api->duckdb_destroy_config

//! extract_statements
#define duckdb_extract_statements          duckdb_ext_api->duckdb_extract_statements
#define duckdb_prepare_extracted_statement duckdb_ext_api->duckdb_prepare_extracted_statement
#define duckdb_extract_statements_error    duckdb_ext_api->duckdb_extract_statements_error
#define duckdb_destroy_extracted           duckdb_ext_api->duckdb_destroy_extracted

//! hugeint_helpers
#define duckdb_hugeint_to_double duckdb_ext_api->duckdb_hugeint_to_double
#define duckdb_double_to_hugeint duckdb_ext_api->duckdb_double_to_hugeint

//! replacement_scans
#define duckdb_add_replacement_scan               duckdb_ext_api->duckdb_add_replacement_scan
#define duckdb_replacement_scan_set_function_name duckdb_ext_api->duckdb_replacement_scan_set_function_name
#define duckdb_replacement_scan_add_parameter     duckdb_ext_api->duckdb_replacement_scan_add_parameter
#define duckdb_replacement_scan_set_error         duckdb_ext_api->duckdb_replacement_scan_set_error

//! decimal_helpers
#define duckdb_double_to_decimal duckdb_ext_api->duckdb_double_to_decimal
#define duckdb_decimal_to_double duckdb_ext_api->duckdb_decimal_to_double

//! appender
#define duckdb_appender_create       duckdb_ext_api->duckdb_appender_create
#define duckdb_appender_column_count duckdb_ext_api->duckdb_appender_column_count
#define duckdb_appender_column_type  duckdb_ext_api->duckdb_appender_column_type
#define duckdb_appender_error        duckdb_ext_api->duckdb_appender_error
#define duckdb_appender_flush        duckdb_ext_api->duckdb_appender_flush
#define duckdb_appender_close        duckdb_ext_api->duckdb_appender_close
#define duckdb_appender_destroy      duckdb_ext_api->duckdb_appender_destroy
#define duckdb_appender_begin_row    duckdb_ext_api->duckdb_appender_begin_row
#define duckdb_appender_end_row      duckdb_ext_api->duckdb_appender_end_row
#define duckdb_append_default        duckdb_ext_api->duckdb_append_default
#define duckdb_append_bool           duckdb_ext_api->duckdb_append_bool
#define duckdb_append_int8           duckdb_ext_api->duckdb_append_int8
#define duckdb_append_int16          duckdb_ext_api->duckdb_append_int16
#define duckdb_append_int32          duckdb_ext_api->duckdb_append_int32
#define duckdb_append_int64          duckdb_ext_api->duckdb_append_int64
#define duckdb_append_hugeint        duckdb_ext_api->duckdb_append_hugeint
#define duckdb_append_uint8          duckdb_ext_api->duckdb_append_uint8
#define duckdb_append_uint16         duckdb_ext_api->duckdb_append_uint16
#define duckdb_append_uint32         duckdb_ext_api->duckdb_append_uint32
#define duckdb_append_uint64         duckdb_ext_api->duckdb_append_uint64
#define duckdb_append_uhugeint       duckdb_ext_api->duckdb_append_uhugeint
#define duckdb_append_float          duckdb_ext_api->duckdb_append_float
#define duckdb_append_double         duckdb_ext_api->duckdb_append_double
#define duckdb_append_date           duckdb_ext_api->duckdb_append_date
#define duckdb_append_time           duckdb_ext_api->duckdb_append_time
#define duckdb_append_timestamp      duckdb_ext_api->duckdb_append_timestamp
#define duckdb_append_interval       duckdb_ext_api->duckdb_append_interval
#define duckdb_append_varchar        duckdb_ext_api->duckdb_append_varchar
#define duckdb_append_varchar_length duckdb_ext_api->duckdb_append_varchar_length
#define duckdb_append_blob           duckdb_ext_api->duckdb_append_blob
#define duckdb_append_null           duckdb_ext_api->duckdb_append_null
#define duckdb_append_data_chunk     duckdb_ext_api->duckdb_append_data_chunk

//! open_connect
#define duckdb_open            duckdb_ext_api->duckdb_open
#define duckdb_open_ext        duckdb_ext_api->duckdb_open_ext
#define duckdb_close           duckdb_ext_api->duckdb_close
#define duckdb_connect         duckdb_ext_api->duckdb_connect
#define duckdb_interrupt       duckdb_ext_api->duckdb_interrupt
#define duckdb_query_progress  duckdb_ext_api->duckdb_query_progress
#define duckdb_disconnect      duckdb_ext_api->duckdb_disconnect
#define duckdb_library_version duckdb_ext_api->duckdb_library_version

//! helpers
#define duckdb_malloc            duckdb_ext_api->duckdb_malloc
#define duckdb_free              duckdb_ext_api->duckdb_free
#define duckdb_vector_size       duckdb_ext_api->duckdb_vector_size
#define duckdb_string_is_inlined duckdb_ext_api->duckdb_string_is_inlined

//! validity_mask_functions
#define duckdb_validity_row_is_valid     duckdb_ext_api->duckdb_validity_row_is_valid
#define duckdb_validity_set_row_validity duckdb_ext_api->duckdb_validity_set_row_validity
#define duckdb_validity_set_row_invalid  duckdb_ext_api->duckdb_validity_set_row_invalid
#define duckdb_validity_set_row_valid    duckdb_ext_api->duckdb_validity_set_row_valid

//! scalar_functions
#define duckdb_create_scalar_function          duckdb_ext_api->duckdb_create_scalar_function
#define duckdb_destroy_scalar_function         duckdb_ext_api->duckdb_destroy_scalar_function
#define duckdb_scalar_function_set_name        duckdb_ext_api->duckdb_scalar_function_set_name
#define duckdb_scalar_function_add_parameter   duckdb_ext_api->duckdb_scalar_function_add_parameter
#define duckdb_scalar_function_set_return_type duckdb_ext_api->duckdb_scalar_function_set_return_type
#define duckdb_scalar_function_set_extra_info  duckdb_ext_api->duckdb_scalar_function_set_extra_info
#define duckdb_scalar_function_set_function    duckdb_ext_api->duckdb_scalar_function_set_function
#define duckdb_register_scalar_function        duckdb_ext_api->duckdb_register_scalar_function

// Place in global scope of C/C++ file that contains the `init` function
#define DUCKDB_EXTENSION_MAIN const duckdb_ext_api_v0 *duckdb_ext_api = 0;
// First line in the `init` function should load the api struct using this function
#define DUCKDB_EXTENSION_LOAD_API(v) duckdb_ext_api = v;
// Place in global scope of any C/C++ file that needs to access the extension API
#define DUCKDB_EXTENSION_EXTERN extern const duckdb_ext_api_v0 *duckdb_ext_api;

#ifdef __cplusplus
}
#endif
