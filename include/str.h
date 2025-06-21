#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include <stddef.h>

void str_readline(char* buf, const int limit, const char* prompt);

bool str_isempty(const char* s);
bool str_isnumeric(const char* s);
int str_toi(const char* s);
void str_delim_right(char* s, char d);

void mbstr_readline(char* buf, size_t limit, const char* prompt);

size_t mbstr_len(const char* s);
int mbc_len(char mbc);
bool mbstr_isempty(const char* s);
bool mbstr_isnumeric(const char* s);
int mbstr_toi(const char* s);
void mbstr_delim_right(char* s, char d);

#endif
