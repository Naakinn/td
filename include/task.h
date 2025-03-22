#ifndef TASK_H
#define TASK_H

#include "sqlite3.h"

int list_tasks(sqlite3* db);
int info_task(sqlite3* db, const char* s);
int push_task(sqlite3* db, const char* name, const char* note);

#endif
