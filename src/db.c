#include <defs.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "str.h"

/* Create directory with read-write owner permissions specified by `pathname`.
 * Returns non-zero value on error, and zero otherwise. */
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
