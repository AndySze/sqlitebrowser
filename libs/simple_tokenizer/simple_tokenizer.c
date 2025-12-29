#include "simple_tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <sqlite3.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SQLITE_CORE
#define SQLITE_CORE 1
#endif

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5 1
#endif

struct SimpleTokenizer {
    SimpleTokenizerMode mode;
};

typedef struct SimpleTokenizer SimpleTokenizer;

static int simpleCreate(void* pCtx, const char** azArg, int nArg, Fts5Tokenizer** ppOut)
{
    (void)azArg;
    (void)nArg;

    SimpleTokenizer* p = (SimpleTokenizer*)sqlite3_malloc(sizeof(SimpleTokenizer));
    if(!p)
        return SQLITE_NOMEM;

    p->mode = (SimpleTokenizerMode)(intptr_t)pCtx;
    *ppOut = (Fts5Tokenizer*)p;

    return SQLITE_OK;
}

static void simpleDelete(Fts5Tokenizer* pTok)
{
    sqlite3_free(pTok);
}

static int isTokenChar(int c)
{
    return isalnum(c) || (c & 0x80);
}

static int simpleTokenize(Fts5Tokenizer* pTok, void* pCtx, int flags, const char* pText, int nText,
                          int (*xToken)(void*, int, const char*, int, int, int))
{
    SimpleTokenizer* p = (SimpleTokenizer*)pTok;
    int i = 0;
    int start = -1;
    int tokenFlags = 0;

    if(p->mode == SIMPLE_TOKEN_MODE_JIEBA_QUERY && (flags & FTS5_TOKENIZE_QUERY))
        tokenFlags |= FTS5_TOKEN_COLOCATED;

    while(i <= nText)
    {
        unsigned char c = (unsigned char)(i < nText ? pText[i] : ' ');

        if(start >= 0 && !isTokenChar(c))
        {
            int rc = xToken(pCtx, tokenFlags, &pText[start], i - start, start, i);
            if(rc != SQLITE_OK)
                return rc;
            start = -1;
        }
        else if(start < 0 && isTokenChar(c))
        {
            start = i;
        }
        i++;
    }

    return SQLITE_OK;
}

int simpleRegisterTokenizer(const fts5_api* pApi, const char* zName, SimpleTokenizerMode mode)
{
    static const fts5_tokenizer tokenizer = { simpleCreate, simpleDelete, simpleTokenize };
    return pApi->xCreateTokenizer(pApi, zName, (void*)(intptr_t)mode, &tokenizer, nullptr);
}

