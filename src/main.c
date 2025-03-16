#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>

#include "defs.h"
#include "str.h"
#include "task.h"

int db_init(sqlite3* db) {
    char* sql =
        "CREATE TABLE IF NOT EXISTS tasks"
        "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "note TEXT);";
    char* errmsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg)) {
        fprintf(stderr, "td: sqlite3 error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return 1;
    }
    return 0;
}

void do_input(int argc, char** argv, sqlite3* db) {
    if (argc < 2) {
        list_tasks(db);
        return;
    }

    char* command = argv[1];
    if (str_cmp(command, "push")) {
        PASS
    } else if (str_cmp(command, "info")) {
        char buf[LINE_SIZE] = {0};
        while (true) {
            str_readline(buf, LINE_SIZE, "Task id: ");
            if (info_task(db, buf)) {
                fprintf(stderr, "td: invalid id '%s'", buf);
            } else
                break;
        }
    } else if (str_cmp(command, "amend")) {
        PASS
    } else if (str_cmp(command, "drop")) {
        PASS
    } else {
        error("unrecognized command '%s'\n", command);
    }
}

int main(int argc, char** argv) {
    sqlite3* db;
    const char* db_name = "data.db";

    int rc = sqlite3_open(db_name, &db);
    if (rc != SQLITE_OK) {
        error("sqlite3 error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    if (db_init(db)) {
        sqlite3_close(db);
        return 1;
    }
    do_input(argc, argv, db);

    sqlite3_close(db);
    return 0;
}
