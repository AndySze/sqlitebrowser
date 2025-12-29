#include "simple_tokenizer.h"

#ifndef SQLITE_CORE
#define SQLITE_CORE 1
#endif

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5 1
#endif

int simpleRegisterJiebaModes(const fts5_api* pApi)
{
    int rc = simpleRegisterTokenizer(pApi, "jieba", SIMPLE_TOKEN_MODE_JIEBA);
    if(rc == SQLITE_OK)
        rc = simpleRegisterTokenizer(pApi, "jieba_query", SIMPLE_TOKEN_MODE_JIEBA_QUERY);
    return rc;
}

