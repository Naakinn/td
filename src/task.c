#include "task.h"

#include <stdarg.h>
#include <stdio.h>

#include "defs.h"
#include "sqlite3.h"
#include "str.h"

/* Check sqlite3 return code `rc`. If not success, print error mesage with
 * `errmsg`. */
int handle_rc(int rc, char* errmsg) {
    if (rc != SQLITE_OK) {
        error("Sqlite3 error: %s, rc = %d\n", errmsg != NULL ? errmsg : "NULL",
              rc);
        sqlite3_free(errmsg);
        return 1;
    }
    return 0;
}

/* Format sql request string `fmt` with variable-length list of arguments using
 * `sqlite3_vmprintf` function and write the result to an allocated output
 * buffer. Returns output buffer on success and NULL on error.
 * Warning: This function allocates memory which must be returned using
 * `sqlite3_free`. */
char* prepare_request(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char* rc = sqlite3_vmprintf(fmt, ap);
    if (rc == NULL) {
        error("Couldn't allocate memory for sql request\n");
        defer(rc, NULL);
    }
defer:
    va_end(ap);
    return rc;
}

/* Callback function for `list_tasks`. The only use of this function is to pass
 * it as a function pointer to `sqlite3_exec`.*/
int list_callback(void* UNUSED(arg), int colnr, char** columns,
                  char** UNUSED(colnames)) {
    if (colnr > 1) {
        printf("{%s} %s", columns[0], columns[1]);
        putchar('\n');
    }
    return 0;
}

/* Fetch id and name for all tasks in the `db`. Returns non-zero error code if
 * an error occurs, zero otherwise. */
int list_tasks(sqlite3* db) {
    char* sql = "SELECT * FROM tasks;";
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, list_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

/* Callback function for `info_task`. The only use of this function is to pass
 * it as a function pointer to `sqlite3_exec`. */
int info_callback(void* UNUSED(arg), int colnr, char** columns,
                  char** UNUSED(colnames)) {
    if (colnr < 1) return 0;
    printf("{%s} %s: %s\n", columns[0], columns[1],
           columns[2] != NULL ? columns[2] : "NULL");
    return 0;
}

/* Fetch id, name and note for each task in the `db`. The id is obtained from
 * numeric string `s`. Returns non-zero error code if an error occurs, zero
 * otherwise. */
int info_task(sqlite3* db, const char* s) {
    int res = 0;
    if (str_isempty(s)) return 0;
    if (!str_isnumeric(s)) return 1;

    // TODO: handle id range, for e.g. 1-3
    int id = str_toi(s);
    char* fmt = "SELECT * FROM tasks WHERE id=%d;";
    char* sql = prepare_request(fmt, id);
    if (sql == NULL) defer(res, 1);

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, info_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) sqlite3_free(sql);
    return res;
}

/* Push new task to the `db` with name `name` and note `note`. If `note` is
 * empty (`str_isempty` return `true`) or NULL, tasks's note is "NULL". Returns
 * non-zero value on error, zero otherwise. */
int push_task(sqlite3* db, const char* name, const char* note) {
    int res = 0;

    char* fmt = "INSERT INTO tasks (name, note) VALUES (%Q, %Q);";
    char* sql = prepare_request(fmt, name, note);
    if (sql == NULL) defer(res, 1);

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) sqlite3_free(sql);
    return res;
}

/* Delete a task from the `db` obtained by numeric string `s`. Returns non-zero
 * value on error, zero otherwise. */
int drop_task(sqlite3* db, const char* s) {
    int res = 0;
    if (!str_isnumeric(s)) return 1;

    int id = str_toi(s);
    char* fmt = "DELETE FROM tasks WHERE id=%d;";
    char* sql = prepare_request(fmt, id);
    if (sql == NULL) defer(res, 1);
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) sqlite3_free(sql);
    return res;
}

/* Change attributes of a task obtained by numeric string `sid`. If `mode` is
 * `AMEND_NAME`, task's name is changed to `s`. If `mode` is `AMEND_NOTE`,
 * tasks's note is changed to `s`. Returns non-zero value on error, zero
 * otherwise. */
int amend_task(sqlite3* db, int mode, const char* sid, const char* s) {
    if (!str_isnumeric(sid)) return 1;
    int res = 0;

    int id = str_toi(sid);
    char* sql = NULL;
    char* fmt = NULL;
    switch (mode) {
        case AMEND_NAME:
            fmt = "UPDATE tasks SET name=%Q WHERE id=%d;";
            sql = prepare_request(fmt, s, id);
            if (sql == NULL) defer(res, 1);
            break;
        case AMEND_NOTE:
            fmt = "UPDATE tasks SET note=%Q WHERE id=%d;";
            sql = prepare_request(fmt, s, id);
            if (sql == NULL) defer(res, 1);
            break;
        default:
            defer(res, 1);
    }

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;

defer:
    if (sql != NULL) sqlite3_free(sql);
    return res;
}
