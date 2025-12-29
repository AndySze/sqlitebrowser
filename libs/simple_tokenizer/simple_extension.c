#include "simple_tokenizer.h"

#include <sqlite3ext.h>

#ifndef SQLITE_CORE
#define SQLITE_CORE 1
#endif

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5 1
#endif

SQLITE_EXTENSION_INIT1

int sqlite3_simple_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    SQLITE_EXTENSION_INIT2(pApi);

    fts5_api* api = (fts5_api*)sqlite3_fts5_api_from_db(db);

    if(!api)
    {
        if(pzErrMsg)
            *pzErrMsg = sqlite3_mprintf("FTS5 is not available in this SQLite build");
        return SQLITE_ERROR;
    }

    return simpleRegisterJiebaModes(api);
}

// Default entry point used by sqlite3_load_extension when no entry symbol is specified.
int sqlite3_extension_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    return sqlite3_simple_init(db, pzErrMsg, pApi);
}

