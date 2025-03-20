#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include <stddef.h>

void str_readline(char* buf, const int limit, const char* prompt);
bool str_isempty(const char* s);
bool str_isnumeric(const char* s);
int str_toi(const char* s);

#endif
