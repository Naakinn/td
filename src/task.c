#include "task.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "sqlite3.h"
#include "str.h"

/* Check if character string `s` contains only allowed symbols.
WARNING: It will be deprecated in future versions. */
int sanitize(const char* s) {
    if (s == NULL) return 1;
    for (int i = 0; s[i] != '\0'; ++i) {
        if (!((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z') ||
              (s[i] == ',') || (s[i] == '.') || (s[i] == ' ') ||
              (s[i] >= '0' && s[i] <= '9') || (s[i] == '!') || (s[i] == '&'))) {
            return 1;
        }
    }
    return 0;
}

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

/* Allocate string for sql request defined by `fmt` and format it using
 * variable-length array of arguments and function `vsnprintf`. `len` is the
 * maximum possible allocated string size(including `\0`), which should be at
 * least `strlen(fmt) + strlen(<va_args>) + 1.`.
 * WARNING: This function allocates memory, and caller must return it. */
char* prepare_request(const char* fmt, size_t len, ...) {
    va_list ap;
    va_start(ap, len);
    char* buf = calloc(len, sizeof(char));
    char* rc = buf;
    if (buf == NULL) {
        error("Couldn't allocate memory\n");
        defer(rc, NULL);
    }
    size_t bytes = vsnprintf(buf, len, fmt, ap);
    if (bytes <= 0 || bytes >= len) {
        error(
            "Couldn't prepare sql request. Expected at most %ld symbols, got "
            "%ld\n",
            len, bytes);
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
    char* sql = prepare_request(fmt, strlen(fmt) + strlen(s) + 1, id);
    if (sql == NULL) defer(res, 1);

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, info_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) free(sql);
    return res;
}

/* Push new task to the `db` with name `name` and note `note`. If `note` is
 * empty (`str_isempty` return `true`) or NULL, tasks's note is "NULL". Returns non-zero value on error, zero
 * otherwise. */
int push_task(sqlite3* db, const char* name, const char* note) {
    bool has_note = true;
    int res = 0;
    if (str_isempty(note)) has_note = false;
    if (sanitize(name)) return 1;
    if (sanitize(note)) return 1;

    char* fmt = "INSERT INTO tasks (name, note) VALUES ('%s', '%s');";
    size_t len = strlen(fmt) + strlen(name) + (has_note ? strlen(note) : 0);
    char* sql = prepare_request(fmt, len + 1, name, has_note ? note : "NULL");
    if (sql == NULL) defer(res, 1);

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) free(sql);
    return res;
}

/* Delete a task from the `db` obtained by numeric string `id`. Returns non-zero
 * value on error, zero otherwise. */
int drop_task(sqlite3* db, const char* id) {
    int res = 0;
    if (!str_isnumeric(id)) return 1;

    char* fmt = "DELETE FROM tasks WHERE id=%s;";
    char* sql = prepare_request(fmt, strlen(fmt) + strlen(id) + 1, id);
    if (sql == NULL) defer(res, 1);
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) defer(res, 1);

defer:
    if (sql != NULL) free(sql);
    return res;
}

/* Change attributes of a task obtained by numeric string `id`. If `mode` is
 * `AMEND_NAME`, task's name is changed to `s`. If `mode` is `AMEND_NOTE`,
 * tasks's note is changed to `s`. Returns non-zero value on error, zero
 * otherwise. */
int amend_task(sqlite3* db, int mode, const char* id, const char* s) {
    if (!str_isnumeric(id)) return 1;
    if (sanitize(s)) return 1;
    int res = 0;

    char* sql = NULL;
    char* fmt = NULL;
    switch (mode) {
        case AMEND_NAME:
            fmt = "UPDATE tasks SET name='%s' WHERE id=%s;";
            sql = prepare_request(fmt, strlen(fmt) + strlen(s) + strlen(id) + 1,
                                  s, id);
            if (sql == NULL) defer(res, 1);
            break;
        case AMEND_NOTE:
            fmt = "UPDATE tasks SET note='%s' WHERE id=%s;";
            sql = prepare_request(fmt, strlen(fmt) + strlen(s) + strlen(id) + 1,
                                  s, id);
            if (sql == NULL) defer(res, 1);
            break;
        default:
            defer(res, 1);
    }

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;

defer:
    if (sql != NULL) free(sql);
    return res;
}
