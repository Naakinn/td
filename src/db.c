#include "db.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "defs.h"
#include "sqlite3.h"
#include "str.h"

/* Check sqlite3 return code `rc`. Prints error message if error occurs. Return
 * non-zero on error, zero otherwise. */
int handle_rc(int rc, sqlite3* db) {
    if (rc != SQLITE_OK) {
        error("Sqlite3 error: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    return 0;
}

/* Check sqlite3 return code `rc` after calling `sqlite3_exec`(and other
 * functions that give back `errmsg` on error). Print error message `errmsg` and
 * free it if error occurs. Return non-zero in error, zero otherwise. */
int handle_exec_rc(int rc, char* errmsg) {
    if (rc != SQLITE_OK) {
        error("Sqlite3 error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return 1;
    }
    return 0;
}

/* Create directory with read-write owner permissions specified by
 * `pathname`. Returns non-zero value on error, and zero otherwise. */
int create_dir(const char* pathname) {
    if (mkdir(pathname, S_IRUSR + S_IWUSR) != 0) {
        error("Couldn't create directory '%s'\n", pathname);
        return 1;
    }
    return 0;
}

/* Find location of .td directory, which stands for td database, and write it as
 * a string to `db_pathname`. If directory is not found, this function creates
 * it at user's home directory. Returns non-zero value on error, and zero
 * otherwise. */
int locate_db(char** db_pathname) {
    int rc = 0;
    char* td_dir = "/.td";
    char* td_db = "/td_data.db";
    char cwd[PATH_MAX] = {};
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        error("Couldn't get current directory\n");
        defer(rc, 1);
    }

    const char* home = getenv("HOME");
    if (home == NULL) {
        error("Couldn't find 'HOME' environment variable\n");
        defer(rc, 1);
    }

    strncat(cwd, td_dir, strlen(td_dir));
    struct stat st;
    while (true) {
        if (stat(cwd, &st) == -1) {
            // move upwards
            str_delim_right(cwd, '/');
            if (strcmp(cwd, home) == 0) {
                // not found
                strncat(cwd, td_dir, strlen(td_dir));
                break;
            }
            str_delim_right(cwd, '/');
            strncat(cwd, td_dir, strlen(td_dir));
        } else {
            // found
            strncat(cwd, td_db, strlen(td_db));
            *db_pathname = calloc(strlen(cwd) + 1, sizeof(char));
            strncpy(*db_pathname, cwd, strlen(cwd));
            defer(rc, 0);
        }
    }

    if (create_dir(cwd) != 0) defer(rc, 1);
defer:
    return rc;
}

/* Create .td directory at the current directory. Returns non-zero on error, and
 * zero otherwise. */
int local_db_init() {
    char cwd[PATH_MAX + 1] = {};
    char* td_dir = "/.td";
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        error("Couldn't get current directory\n");
        return 1;
    }
    strncat(cwd, td_dir, strlen(td_dir));
    if (create_dir(cwd) != 0) return 1;
    printf("Created directory '%s'\n", cwd);
    return 0;
}
