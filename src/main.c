#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "defs.h"
#include "str.h"
#include "task.h"

int db_init(sqlite3** db, const char* db_name) {
    int rc = sqlite3_open(db_name, db);
    if (rc != SQLITE_OK) {
        error("sqlite3 error: %s\n", sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 1;
    }
    char* sql =
        "CREATE TABLE IF NOT EXISTS tasks"
        "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "note TEXT);";
    char* errmsg = NULL;
	rc = sqlite3_exec(*db, sql, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
		error("sqlite3 error: %s\n", errmsg);
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
    if (strcmp(command, "push") == 0) {
        PASS
    } else if (strcmp(command, "info") == 0) {
        char buf[LINE_SIZE] = {0};
        while (true) {
            str_readline(buf, LINE_SIZE, "Task id: ");
            if (info_task(db, buf)) {
                fprintf(stderr, "td: invalid id '%s'", buf);
            } else
                break;
        }
    } else if (strcmp(command, "amend") == 0) {
        PASS
    } else if (strcmp(command, "drop") == 0) {
        PASS
    } else {
        error("unrecognized command '%s'\n", command);
    }
}

int main(int argc, char** argv) {
    sqlite3* db = NULL;
	
	// c and strings...
	const char* home = getenv("HOME");
	char* db_path = malloc(strlen(home) + strlen("/.td") + 1);
	strcat(db_path, home);
	strcat(db_path, "/.td");
	char* db_pathname = malloc(strlen(db_path) + strlen("/td_data.db") + 1);
	strcpy(db_pathname, db_path);
	strcat(db_pathname, "/td_data.db");
	
    // check for ~/.td directory
    struct stat st = {0};
    if (stat(db_path, &st) == -1) {
        int rc = mkdir(db_path, S_IRWXU);
        if (rc != 0) {
            error("could not create directory %s\n", db_path);
            return 1;
        }
    }

    if (db_init(&db, db_pathname)) return 1;
    do_input(argc, argv, db);
    sqlite3_close(db);
	free(db_path);
	free(db_pathname);
    return 0;
}
