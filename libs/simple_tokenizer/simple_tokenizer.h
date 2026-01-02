#pragma once

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5 1
#endif

#ifndef SQLITE_CORE
#define SQLITE_CORE 1
#endif

#include <sqlite3ext.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SimpleTokenizerMode {
    SIMPLE_TOKEN_MODE_BASIC = 0,
    SIMPLE_TOKEN_MODE_JIEBA = 1,
    SIMPLE_TOKEN_MODE_JIEBA_QUERY = 2
} SimpleTokenizerMode;

int simpleRegisterTokenizer(fts5_api* pApi, const char* zName, SimpleTokenizerMode mode);
int simpleRegisterTokenizers(fts5_api* pApi);
int simpleRegisterJiebaModes(fts5_api* pApi);

// Extension entry points (exported symbols)
int sqlite3_simple_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi);
int sqlite3_extension_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi);

#ifdef __cplusplus
}
#endif
