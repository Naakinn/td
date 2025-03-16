#include <stdio.h>

#include "defs.h"
#include "sqlite3.h"
#include "str.h"

int handle_rc(int rc, char* errmsg) {
    if (rc != SQLITE_OK) {
        if (errmsg != NULL) {
            error("sqlite3 error: %s\n", errmsg);
            sqlite3_free(errmsg);
        } else {
            error("sqlite3 error (error message is not provided)\n");
        }
        return 1;
    }
    return 0;
}

int list_callback(void* UNUSED(arg), int colnr, char** columns,
                  char** UNUSED(colnames)) {
    if (colnr > 1) {
        printf("{%s} %s", columns[0], columns[1]);
        putchar('\n');
    }
    return 0;
}

int list_tasks(sqlite3* db) {
    char* sql = "SELECT * from tasks;";
    char* errmsg;
    int rc = sqlite3_exec(db, sql, list_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

int info_callback(void* UNUSED(arg), int colnr, char** columns,
                  char** UNUSED(colnames)) {
    if (colnr < 1) return 0;
    printf("{%s} %s: %s", columns[0], columns[1],
           columns[2] != NULL ? columns[2] : "NULL");
    putchar('\n');
    return 0;
}

int info_task(sqlite3* db, const char* s) {
    if (str_isempty(s)) return 0;
    if (!str_isnumeric(s)) return 1;

    // TODO: handle id range, for e.g. 1-3
    int id = str_toi(s);
    char sql[SQL_SIZE] = {0};
    sprintf(sql, "SELECT * from tasks WHERE id=%d", id);
    char* errmsg;
    int rc = sqlite3_exec(db, sql, info_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}
