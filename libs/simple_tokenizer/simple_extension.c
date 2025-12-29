#include "simple_tokenizer.h"

#include <sqlite3ext.h>
#include <stddef.h>

#ifndef SQLITE_CORE
#define SQLITE_CORE 1
#endif

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5 1
#endif

SQLITE_EXTENSION_INIT1

static int fts5ApiFromDb(sqlite3* db, fts5_api** ppApi)
{
    sqlite3_stmt* stmt = NULL;
    *ppApi = NULL;

    int rc = sqlite3_prepare_v2(db, "SELECT fts5(?1)", -1, &stmt, NULL);
    if(rc != SQLITE_OK)
        return rc;

    sqlite3_bind_pointer(stmt, 1, (void*)ppApi, "fts5_api_ptr", NULL);
    (void)sqlite3_step(stmt);
    rc = sqlite3_finalize(stmt);
    return rc;
}

int sqlite3_simple_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    SQLITE_EXTENSION_INIT2(pApi);

    fts5_api* api = NULL;
    const int rc = fts5ApiFromDb(db, &api);
    if(rc != SQLITE_OK)
        return rc;

    if(!api)
    {
        if(pzErrMsg)
            *pzErrMsg = sqlite3_mprintf("FTS5 is not available in this SQLite build");
        return SQLITE_ERROR;
    }

    return simpleRegisterTokenizers(api);
}

// Default entry point used by sqlite3_load_extension when no entry symbol is specified.
int sqlite3_extension_init(sqlite3* db, char** pzErrMsg, const sqlite3_api_routines* pApi)
{
    return sqlite3_simple_init(db, pzErrMsg, pApi);
}
