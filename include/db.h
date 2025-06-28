#ifndef DB_H
#define DB_H

#include "sqlite3.h"

int locate_db(char** db_pathname);
int create_td_dir(const char* pathname);
int local_db_init();
int handle_rc(int rc, sqlite3* db);
int handle_exec_rc(int rc, char* errmsg);

#endif
