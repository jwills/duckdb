#pragma once

#include "duckdb/common/constants.hpp"
#include "duckdb/common/string.hpp"

#define MD5_HASH_LENGTH 64

/*
 * If compiled on a machine that doesn't have a 32-bit integer,
 * you just set "uint32" to the appropriate datatype for an
 * unsigned 32-bit integer.  For example:
 *
 *       cc -Duint32='unsigned long' md5.c
 *
 */
#ifndef uint32
#define uint32 unsigned int
#endif

namespace duckdb {

struct MD5Context {
	int isInit;
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

class MD5 {
    public:
    MD5();
    ~MD5() {}

    void Add(const string &z);
    void Add(const char *z);
    string Finish();

    private:
    unique_ptr<MD5Context> ctx;
};

} // namespace duckdb