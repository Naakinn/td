#include <defs.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "str.h"

int create_td_dir(const char* pathname) {
    if (mkdir(pathname, S_IRWXU) != 0) {
        error("could not create directory '%s'\n", pathname);
        return 1;
    }
    return 0;
}

int locate_db(char** db_pathname) {
    int rc = 0;
    char* td_dir = "/.td";
    char* td_db = "/td_data.db";
    char cwd[PATH_MAX + 1] = {};
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
    
    if (create_td_dir(cwd) != 0) defer(rc, 1);
defer:
    return rc;
}

int local_db_init() {
    char cwd[PATH_MAX + 1] = {};
    char* td_dir = "/.td";
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        error("Couldn't get current directory\n");
        return 1;
    }
    strncat(cwd, td_dir, strlen(td_dir));
    if (create_td_dir(cwd) != 0) return 1;
    printf("Created directory '%s'\n", cwd);
    return 0;
}
