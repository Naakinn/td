#include "str.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Read at most `limit` characters of line obtained from standard input and
 * store it in `buf` null-terminating `\0` it. Prints `prompt` before fetching
 * input if `prompt` is not `NULL`. `buf` must have size at least `limit` + 1.
 * This function also flushes all input after writing to `buf`. */
void str_readline(char* buf, int limit, const char* prompt) {
    if (prompt != NULL) write(STDOUT_FILENO, prompt, strlen(prompt));
    char c = 0;
    int i = 0;
    while ((c = getchar()) != EOF && c != '\n' && i < limit) {
        buf[i++] = c;
    }
    buf[i] = '\0';
    // flush input
    if (c != '\n' && c != EOF)
        while ((c = getchar()) != '\n' && c != EOF);
}

/* Returns `true` if all characters of string `s` are those, for which `isgraph`
 * returns non-zero, `false` otherwise. */
bool str_isempty(const char* s) {
    if (s == NULL) return true;
    for (size_t i = 0; i < strlen(s); ++i) {
        if (isgraph(s[i]) != 0) {
            return false;
        }
    }
    return true;
}

/* Returns `true` if all characters of string `s` are those, for which `isdigit`
 * returns non-zero, `false` otherwise. */
bool str_isnumeric(const char* s) {
    if (s == NULL) return false;
    for (size_t i = 0; i < strlen(s); ++i) {
        if (isdigit(s[i]) == 0) {
            return false;
        }
    }
    return true;
}

/* Convert string `s` to integer. */
int str_toi(const char* s) {
    int res = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        res = res * 10 + (s[i] - '0');
    }
    return res;
}
/* Remove characters of string `s` (including delimiter) until meet delimiter
 * `d`. */
void str_delim_right(char* s, char d) {
    ssize_t l = strlen(s);
    for (ssize_t i = l - 1; i >= 0; --i) {
        if (s[i] == d) {
            s[i] = '\0';
            memset(s + i, 0, l - i);
            break;
        }
    }
}
