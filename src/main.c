#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "db.h"
#include "defs.h"
#include "sqlite3.h"
#include "str.h"
#include "task.h"

// change this on every release((
#define VERSION "v1.2.0"

static struct Config gconfig = {.confirm = true};

void help() {
    // clang-format off
    printf("Usage: td [options]\n");
    printf("Simple ToDo task manager. With no command lists all tasks.\n");
    printf("td relies on task database, which is by default located in $HOME/.td directory. "
            "When td is invoked, it recursively finds nearest to the current directory task database.\n");
    printf("Starting from version v1.2.0 td supports UTF-8 string format. The only exception is confirmation. "
            "See --no-confirm below.\n\n");
    printf("COMMANDS:\n");
    printf("\t-p --push Push a task to database.\n");
    printf("\t-i --info <ID> Get information about specific task, such as note.\n");
    printf("\t-d --drop <ID> Delete task.\n");
    printf("\t-a --amend <ID> Amend a task's name or note.\n");
    printf("\t-l --local Initialize task database in the current directory.\n");
    printf("OPTIONS & HELPERS:\n");
    printf("\t-n --no-confirm Do not confirm user before amending or deleting a task.\n");
    printf("\t-v --version Print td's version\n");
    printf("\t-h --help Display this help page.\n");
    // clang-format on
}

int confirm(const char* prompt) {
    char choice[2] = {};
    // cause we accept only 1 byte. mbstr_readline won't take any effect
    str_readline(choice, 1, prompt);
    // str_isempty will fail on incopmlete UTF-8 character
    if (str_isempty(choice)) return 1;
    if (choice[0] == 'y' || choice[0] == 'Y')
        return 0;
    else
        return 1;
}

int db_init(sqlite3** db, const char* db_name) {
    int rc = sqlite3_open_v2(db_name, db,
                             SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (handle_rc(rc, *db)) return 1;
    char* sql =
        "CREATE TABLE IF NOT EXISTS tasks"
        "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "note TEXT);";
    char* errmsg = NULL;
    rc = sqlite3_exec(*db, sql, NULL, NULL, &errmsg);
    return handle_exec_rc(rc, errmsg);
}

void push(sqlite3* db) {
    char name[LINE_LEN * MB_MAX + 1] = {0};
    char note[LINE_LEN_EXT * MB_MAX + 1] = {0};
    char* note_ptr = note;
    while (true) {
        mbstr_readline(name, LINE_LEN, "Enter a name(skip to abort): ");
        if (mbstr_isempty(name)) break;
        mbstr_readline(note, LINE_LEN_EXT, "Enter a note(skip for NULL): ");
        if (mbstr_isempty(note)) note_ptr = NULL;

        if (push_task(db, name, note_ptr)) {
            error("Couldn't create task, please check your name and note\n");
            break;
        } else {
            printf("Created task '%s'\n", name);
            break;
        }
    }
}

void amend(sqlite3* db, Command* cmd) {
    char choice[2] = {};
    char* id = cmd->arg;

    if (info_task(db, id) != 0) {
        error("Couldn't amend task with id '%s'\n", id);
        return;
    }
    while (true) {
        str_readline(choice, 1, "What to change: n(A)me/n(O)te: ");
        if (str_isempty(choice)) break;

        if (choice[0] == 'a' || choice[0] == 'A') {
            char name[LINE_LEN + 1] = {0};
            mbstr_readline(name, LINE_LEN, "New name: ");

            if (gconfig.confirm) {
                if (confirm("Amend task? (y/n) ") != 0) return;
            }

            if (amend_task(db, AMEND_NAME, id, name)) {
                error("Couldn't amend task with id '%s'\n", id);
                return;
            }
            printf("Task amended\n");
            break;

        } else if (choice[0] == 'o' || choice[0] == 'O') {
            char note[LINE_LEN_EXT + 1] = {0};
            mbstr_readline(note, LINE_LEN_EXT, "New note: ");

            if (gconfig.confirm) {
                if (confirm("Amend task? (y/n) ") != 0) return;
            }

            if (amend_task(db, AMEND_NOTE, id, note)) {
                error("Couldn't amend task with id '%s'\n", id);
                break;
            }
            printf("Task amended\n");
            break;

        } else {
            error("Invalid choice\n");
            return;
        }
    }
}

void drop(sqlite3* db, Command* cmd) {
    if (gconfig.confirm) {
        if (confirm("Delete task? (y/n) ") != 0) return;
    }
    if (drop_task(db, cmd->arg)) {
        error("Couldn't delete task with id '%s'\n", cmd->arg);
        return;
    }
    printf("Task deleted\n");
}

void parse_args(Command* cmd, int argc, char** argv) {
    int c;
    const char* short_options = "hpi:d:a:vnl";

    // clang-format off
    struct option long_options[] = {
        {"no-confirm", no_argument, 0, 'n'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"push", no_argument, 0, 'p'},
        {"info", required_argument, 0, 'i'},
        {"amend", required_argument, 0, 'a'},
        {"drop", required_argument, 0, 'd'},
        {"local", no_argument, 0, 'l'},
        {0, 0, 0, 0}
    };
    // clang-format on

    cmd->type = ListCmd;
    while (true) {
        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) break;

        switch (c) {
            // helper/util commands & options
            case 'h':
                cmd->type = NullCmd;
                help();
                return;
            case 'v':  // version
                cmd->type = NullCmd;
                printf("td " VERSION "\n");
                printf("run 'td --help' to get help\n");
                return;
            case 'n':
                gconfig.confirm = false;
                break;

            // UX commands
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
            case 'l':
                cmd->type = LocalCmd;
                break;

            // error-handling cases
            case '?':
                cmd->type = NullCmd;
                break;
            default:
                cmd->type = NullCmd;
                error(
                    "Getopt returned unknown character code '%d'. please check "
                    "your command line arguments\n",
                    c);
                break;
        }
    }
}

int dispatch_command(Command* cmd) {
    int rc = 0;
    sqlite3* db = NULL;
    char* db_pathname = NULL;
    if (locate_db(&db_pathname)) defer(rc, 1);
    if (db_init(&db, db_pathname)) defer(rc, 1);

    switch (cmd->type) {
        case ListCmd:
            if (list_tasks(db) != 0) {
                error("Couldn't get information about tasks\n");
                defer(rc, 1);
            }
            break;
        case InfoCmd:
            if (info_task(db, cmd->arg) != 0) {
                error("Couldn't get information about task with id='%s'\n",
                      cmd->arg);
                defer(rc, 1);
            }
            break;
        case PushCmd:
            push(db);
            break;
        case AmendCmd:
            amend(db, cmd);
            break;
        case DropCmd:
            drop(db, cmd);
            break;
        case LocalCmd:
            if (local_db_init() != 0) {
                error("Couldn't initialize local database\n");
                defer(rc, 1);
            }
            break;
        default:
            error("Unexpected command type\n");
            defer(rc, 1);
    }
defer:
    if (db != NULL) sqlite3_close(db);
    if (db_pathname != NULL) free(db_pathname);
    return rc;
}

int main(int argc, char** argv) {
    // ensure UTF-8
    if (setlocale(LC_ALL, "en_US.utf8") == NULL) {
        error("Couldn't set localte to en_US.utf8\n");
        return 1;
    }
    Command cmd = {0};
    parse_args(&cmd, argc, argv);
    if (cmd.type == NullCmd) return 1;
    if (dispatch_command(&cmd)) return 1;
    return 0;
}
