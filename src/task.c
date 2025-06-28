#include "task.h"

#include <stdarg.h>
#include <stdio.h>

#include "db.h"
#include "defs.h"
#include "sqlite3.h"
#include "str.h"

/* Fetch and print id and name for all tasks in the `db`. Returns non-zero error
 * code if an error occurs, zero otherwise. */
int list_tasks(sqlite3* db) {
    int res = 0;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, name FROM tasks;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (handle_rc(rc, db)) defer(res, 1);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // the only error of sqlite3_column_text is out of memory.
        const unsigned char* id = sqlite3_column_text(stmt, 0);
        if (sqlite3_errcode(db) == SQLITE_NOMEM) defer(res, 1);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        if (sqlite3_errcode(db) == SQLITE_NOMEM) defer(res, 1);
        printf("{%s} %s\n", id, name);
    }

    if (rc != SQLITE_DONE) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        defer(res, 1);
    }
defer:
    sqlite3_finalize(stmt);
    return res;
}

/* Fetch id, name and note for each task in the `db` and print them. The id is
 * obtained from numeric string `id`. Returns non-zero error code if an error
 * occurs, zero otherwise. */
int info_task(sqlite3* db, const char* id) {
    int res = 0;
    if (mbstr_isempty(id)) return 0;
    if (!mbstr_isnumeric(id)) return 1;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT id, name, note FROM tasks WHERE id=?1;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        defer(res, 1);
    }
    const unsigned char* r_id = sqlite3_column_text(stmt, 0);
    if (sqlite3_errcode(db) == SQLITE_NOMEM) defer(res, 1);
    const unsigned char* r_name = sqlite3_column_text(stmt, 1);
    if (sqlite3_errcode(db) == SQLITE_NOMEM) defer(res, 1);
    const unsigned char* r_note = sqlite3_column_text(stmt, 2);
    if (sqlite3_errcode(db) == SQLITE_NOMEM) defer(res, 1);
    printf("{%s} %s: %s\n", r_id, r_name, r_note);

defer:
    sqlite3_finalize(stmt);
    return res;
}

/* Push new task to the `db` with name `name` and note `note`. If `note` is
 * NULL, tasks's note is SQL NULL. Returns non-zero value on error, zero
 * otherwise. */
int push_task(sqlite3* db, const char* name, const char* note) {
    int res = 0;
    if (name == NULL) return 1;

    sqlite3_stmt* stmt;
    char* sql = "INSERT INTO tasks (name, note) VALUES (?1, ?2);";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    if (handle_rc(rc, db)) defer(res, 1);
    rc = note == NULL ? sqlite3_bind_null(stmt, 2)
                      : sqlite3_bind_text(stmt, 2, note, -1, SQLITE_STATIC);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        defer(res, 1);
    }
defer:
    sqlite3_finalize(stmt);
    return res;
}

/* Delete a task from the `db` obtained by numeric string `s`. Returns non-zero
 * value on error, zero otherwise. */
int drop_task(sqlite3* db, const char* id) {
    int res = 0;
    if (!mbstr_isnumeric(id)) return 1;

    sqlite3_stmt* stmt;
    char* sql = "DELETE FROM tasks WHERE id=?1;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_bind_text(stmt, 1, id, -1, SQLITE_STATIC);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        defer(res, 1);
    }
defer:
    sqlite3_finalize(stmt);
    return res;
}

/* Change attributes of a task obtained by numeric string `id`. If `mode` is
 * `AMEND_NAME`, task's name is changed to `s`. If `mode` is `AMEND_NOTE`,
 * tasks's note is changed to `s`. Returns non-zero value on error, zero
 * otherwise. */
int amend_task(sqlite3* db, int mode, const char* id, const char* s) {
    int res = 0;
    if (!mbstr_isnumeric(id)) return 1;

    sqlite3_stmt* stmt;
    char* sql = NULL;
    switch (mode) {
        case AMEND_NAME:
            sql = "UPDATE tasks SET name=?1 WHERE id=?2;";
            break;
        case AMEND_NOTE:
            sql = "UPDATE tasks SET note=?1 WHERE id=?2;";
            break;
        default:
            return 1;
    }

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_bind_text(stmt, 1, s, -1, SQLITE_STATIC);  // bind s
    if (handle_rc(rc, db)) defer(res, 1);
    rc = sqlite3_bind_text(stmt, 2, id, -1, SQLITE_STATIC);  // bind id
    if (handle_rc(rc, db)) defer(res, 1);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        defer(res, 1);
    }

defer:
    sqlite3_finalize(stmt);
    return res;
}
