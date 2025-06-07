#ifndef DB_H
#define DB_H

int locate_db(char** db_pathname);
int create_td_dir(const char* pathname);
int local_db_init();

#endif
