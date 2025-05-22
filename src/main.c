#include <getopt.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "defs.h"
#include "str.h"
#include "task.h"

static struct option long_options[] = {
    // TODO future cli args
    // {"no-confirm", no_argument, 0, 'n'},
    // {"verbose", no_argument, 0, 'v'},
    // {"silent", no_argument, 0, 's'},
    {"help", no_argument, 0, 'h'},       {"push", no_argument, 0, 'p'},
    {"info", required_argument, 0, 'i'}, {"amend", required_argument, 0, 'a'},
    {"drop", required_argument, 0, 'd'}, {0, 0, 0, 0}};

static const char* short_options = "hpi:d:a:";

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

void push(sqlite3* db) {
    char name[LINE_SIZE + 1] = {0};
    char note[LINE_SIZE_EXT + 1] = {0};
    while (true) {
        str_readline(name, LINE_SIZE, "Enter a name(skip to abort): ");
        if (str_isempty(name)) break;
        str_readline(note, LINE_SIZE_EXT, "Enter a note(skip for NULL): ");

        if (push_task(db, name, note)) {
            error("cannot create task, please check your name and note\n");
            error(
                "allowed characters are a-z, A-Z, comma, period, space, "
                "ampersand and !\n");
        } else {
            break;
        }
    }
}

void help() {
    // clang-format off
    printf("Usage: td [options]\n");
    printf("Simple ToDo task manager. With no command lists all tasks.\n");
    printf("COMMANDS:\n");
    printf("\t -p --push Push a task to database.\n");
    printf("\t -i --info <ID> Get information about specific task, such as note.\n");
    printf("\t -d --drop <ID> Delete task.\n");
    printf("\t -a --amend <ID> Amend a task's name or note.\n");
    printf("\t -h --help  Display this help page.\n");
    // clang-format on
}

void amend(sqlite3* db, Command* cmd) {
    char choice[2] = {0};
    char* id = cmd->arg; 

    while (true) {
        str_readline(choice, 1, "What to change: n(A)me/n(O)te: ");
        if (str_isempty(choice)) break;

        if (choice[0] == 'a' || choice[0] == 'A') {
            char name[LINE_SIZE + 1] = {0};
            str_readline(name, LINE_SIZE, "New name: ");
            if (amend_task(db, AMEND_NAME, id, name)) {
                error("cannot amend task '%s'\n", id);
            } else
                break;

        } else if (choice[0] == 'o' || choice[0] == 'O') {
            char note[LINE_SIZE_EXT + 1] = {0};
            str_readline(note, LINE_SIZE_EXT, "New note: ");
            if (amend_task(db, AMEND_NOTE, id, note)) {
                error("cannot amend task '%s'\n", id);
            } else
                break;
        } else
            error("invalid choice\n");
    }
}

void parse_args(Command* cmd, int argc, char** argv) {
    int c;
    cmd->type = ListCmd;
    while (true) {
        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) break;

        switch (c) {
            case 'h':
                cmd->type = HelpCmd;
                break;
            case 'p':
                cmd->type = PushCmd;
                break;
            case 'i':
                cmd->type = InfoCmd;
                cmd->arg = optarg;
                break;
            case 'a':
                cmd->type = AmendCmd;
                cmd->arg = optarg;
                break;
            case 'd':
                cmd->type = DropCmd;
                cmd->arg = optarg;
                break;
            case '?':
                cmd->type = NullCmd;
                break;
            default:
                cmd->type = NullCmd;
                error(
                    "getopt returned unknown character code '%d'. please check "
                    "your command line arguments\n",
                    c);
                break;
        }
    }
}

int locate_db(char** db_pathname) {
    int rc = 0;
    const char* home = getenv("HOME");
    char* td_dir = "/.td";
    char* td_db = "/td_data.db";
    *db_pathname =
        calloc(strlen(home) + strlen(td_dir) + strlen(td_db) + 1, sizeof(char));
    char* db_dir = calloc(strlen(home) + strlen(td_dir) + 1, sizeof(char));
    if (db_pathname == NULL || db_dir == NULL) {
        error("could not allocate memory\n");
        defer(rc, 1);
    }
    strncat(*db_pathname, home, strlen(home));
    strncat(*db_pathname, td_dir, strlen(td_dir));
    strncat(*db_pathname, td_db, strlen(td_db));

    strncat(db_dir, home, strlen(home));
    strncat(db_dir, td_dir, strlen(td_dir));

    struct stat st = {0};
    if (stat(db_dir, &st) == -1) {
        int rc = mkdir(db_dir, S_IRWXU);
        if (rc != 0) {
            error("could not create directory %s\n", db_dir);
            defer(rc, 1);
        }
    }
defer:
    if (db_dir != NULL) free(db_dir);
    return rc;
}

int dispatch_command(Command* cmd) {
    if (cmd->type == HelpCmd) {
        help();
        return 0;
    }
    int result = 0;
    sqlite3* db = NULL;
    char* db_pathname = NULL;
    if (locate_db(&db_pathname)) defer(result, 1);
    if (db_init(&db, db_pathname)) defer(result, 1);

    switch (cmd->type) {
        case ListCmd:
            list_tasks(db);
            break;
        case InfoCmd:
            info_task(db, cmd->arg);
            break;
        case PushCmd:
            push(db);
            break;
        case AmendCmd:
            amend(db, cmd);
            break;
        case DropCmd:
            drop_task(db, cmd->arg);
            break;
        default:
            error("unexpected command type\n");
            defer(result, 1);
    }
defer:
    if (db != NULL) sqlite3_close(db);
    if (db_pathname != NULL) free(db_pathname);
    return result;
}

int main(int argc, char** argv) {
    Command cmd = {0};
    parse_args(&cmd, argc, argv);
    if (cmd.type == NullCmd) return 1;
    if (dispatch_command(&cmd)) return 1;
    return 0;
}
