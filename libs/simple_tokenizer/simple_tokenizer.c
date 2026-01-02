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

static int isSpaceOrControl(unsigned char c)
{
    return isspace((int)c) || iscntrl((int)c);
}

static int isAsciiAlpha(unsigned char c)
{
    return (c < 0x80) && isalpha((int)c);
}

static int isAsciiDigit(unsigned char c)
{
    return (c < 0x80) && isdigit((int)c);
}

static int utf8CharLen(unsigned char c)
{
    if(c < 0x80)
        return 1;
    if((c & 0xE0) == 0xC0)
        return 2;
    if((c & 0xF0) == 0xE0)
        return 3;
    if((c & 0xF8) == 0xF0)
        return 4;
    // Invalid UTF-8 lead byte (or continuation byte). Treat as a single byte.
    return 1;
}

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

static int simpleTokenize(Fts5Tokenizer* pTok, void* pCtx, int flags, const char* pText, int nText,
                          int (*xToken)(void*, int, const char*, int, int, int))
{
    SimpleTokenizer* p = (SimpleTokenizer*)pTok;
    int tokenFlags = 0;

    if(p->mode == SIMPLE_TOKEN_MODE_JIEBA_QUERY && (flags & FTS5_TOKENIZE_QUERY))
        tokenFlags |= FTS5_TOKEN_COLOCATED;

    // This aims to be compatible with the tokenizer used by https://github.com/wangfenjin/simple:
    // - ASCII alphabetic runs are grouped and lowercased
    // - ASCII digit runs are grouped
    // - Non-ASCII UTF-8 characters are tokenized per codepoint (not per byte)
    // - ASCII punctuation is tokenized as single-character tokens
    int i = 0;
    while(i < nText)
    {
        unsigned char c = (unsigned char)pText[i];

        if(isSpaceOrControl(c))
        {
            i++;
            continue;
        }

        int start = i;
        int end = i;

        if(isAsciiAlpha(c))
        {
            end++;
            while(end < nText && isAsciiAlpha((unsigned char)pText[end]))
                end++;

            const int nTok = end - start;
            char* token = (char*)sqlite3_malloc((size_t)nTok + 1);
            if(!token)
                return SQLITE_NOMEM;
            for(int j = 0; j < nTok; j++)
                token[j] = (char)tolower((unsigned char)pText[start + j]);
            token[nTok] = '\0';

            const int rc = xToken(pCtx, tokenFlags, token, nTok, start, end);
            sqlite3_free(token);
            if(rc != SQLITE_OK)
                return rc;
            i = end;
            continue;
        }

        if(isAsciiDigit(c))
        {
            end++;
            while(end < nText && isAsciiDigit((unsigned char)pText[end]))
                end++;
            const int rc = xToken(pCtx, tokenFlags, &pText[start], end - start, start, end);
            if(rc != SQLITE_OK)
                return rc;
            i = end;
            continue;
        }

        if(c < 0x80)
        {
            // ASCII non-space, non-alnum: tokenized as a single character.
            end = start + 1;
            const int rc = xToken(pCtx, tokenFlags, &pText[start], 1, start, end);
            if(rc != SQLITE_OK)
                return rc;
            i = end;
            continue;
        }

        // Non-ASCII: tokenize per UTF-8 codepoint.
        const int len = utf8CharLen(c);
        end = start + len;
        if(end > nText)
            end = nText;

        const int rc = xToken(pCtx, tokenFlags, &pText[start], end - start, start, end);
        if(rc != SQLITE_OK)
            return rc;
        i = end;
    }

    return SQLITE_OK;
}

int simpleRegisterTokenizer(fts5_api* pApi, const char* zName, SimpleTokenizerMode mode)
{
    static fts5_tokenizer tokenizer = { simpleCreate, simpleDelete, simpleTokenize };
    return pApi->xCreateTokenizer(pApi, zName, (void*)(intptr_t)mode, &tokenizer, NULL);
}

int simpleRegisterTokenizers(fts5_api* pApi)
{
    int rc = simpleRegisterTokenizer(pApi, "simple", SIMPLE_TOKEN_MODE_BASIC);

    if(rc == SQLITE_OK)
        rc = simpleRegisterJiebaModes(pApi);

    return rc;
}
