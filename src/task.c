#include "task.h"

#include <stdio.h>

#include "defs.h"
#include "sqlite3.h"
#include "str.h"

int sanitize(const char* s) {
    if (s == NULL) return 1;
    for (int i = 0; s[i] != '\0'; ++i) {
        if (!((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z') ||
              (s[i] == ',') || (s[i] == '.') || (s[i] == ' ') ||
              (s[i] >= '0' && s[i] <= '9') || (s[i] == '!'))) {
            return 1;
        }
    }
    return 0;
}
int handle_rc(int rc, char* errmsg) {
    if (rc != SQLITE_OK) {
        error("sqlite3 error: %s, rc = %d\n", errmsg != NULL ? errmsg : "NULL",
              rc);
        sqlite3_free(errmsg);
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
    char* sql = "SELECT * FROM tasks;";
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, list_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

int info_callback(void* UNUSED(arg), int colnr, char** columns,
                  char** UNUSED(colnames)) {
    if (colnr < 1) return 0;
    printf("{%s} %s: %s\n", columns[0], columns[1],
           columns[2] != NULL ? columns[2] : "NULL");
    return 0;
}

int info_task(sqlite3* db, const char* s) {
    if (str_isempty(s)) return 0;
    if (!str_isnumeric(s)) return 1;

    // TODO: handle id range, for e.g. 1-3
    int id = str_toi(s);
    char sql[SQL_SIZE + 1] = {0};
    sprintf(sql, "SELECT * FROM tasks WHERE id=%d", id);
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, info_callback, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

int push_task(sqlite3* db, const char* name, const char* note) {
    bool has_note = true;
    if (str_isempty(note)) has_note = false;
    if (sanitize(name)) return 1;
    if (sanitize(note)) return 1;

    char sql[SQL_SIZE + 1] = {0};
    sprintf(sql, "INSERT INTO tasks (name, note) VALUES ('%s', '%s');", name,
            has_note ? note : "NULL");
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

int drop_task(sqlite3* db, const char* id) {
    if (!str_isnumeric(id)) return 1;

    char sql[SQL_SIZE + 1] = {0};
    sprintf(sql, "DELETE FROM tasks WHERE id=%s;", id);
    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}

int amend_task(sqlite3* db, int __mode, const char* id, const char* s) {
    if (!str_isnumeric(id)) return 1;
    if (sanitize(s)) return 1;

    char sql[SQL_SIZE + 1] = {0};
    if (__mode == AMEND_NAME) {
        sprintf(sql, "UPDATE tasks SET name='%s' WHERE id=%s;", s, id);
    } else if (__mode == AMEND_NOTE) {
        sprintf(sql, "UPDATE tasks SET note='%s' WHERE id=%s;", s, id);
    } else
        return 1;

    char* errmsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (handle_rc(rc, errmsg)) return 1;
    return 0;
}
