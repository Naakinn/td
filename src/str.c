#include "str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Read at most `limit` characters of line obtained from standard input and
 * store it in `buf` null-terminating `\0` it. Prints `prompt` before fetching
 * input if `prompt` is not `NULL`. `buf` must have size at least `limit` + 1.
 * This function also flushes all input after writing to `buf`. */
void str_readline(char* buf, int limit, const char* prompt) {
    if (prompt != NULL) printf("%s", prompt);
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

/* Read at most `limit` multibyte characters of line obtained from standard
 * input and store it in `buf` null-terminating `\0` it. Prints `prompt` before
 * fetching input if `prompt` is not `NULL`. `buf` must have size at least
 * `limit` * 4 (max number of bytes in UTF-8 multibyte character) + 1 to handle
 * all possible multibyte characters. This function also flushes all input after
 * writing to `buf`. */
void mbstr_readline(char* buf, size_t limit, const char* prompt) {
    if (prompt != NULL) printf("%s", prompt);
    char c;
    size_t len = 0;
    size_t i = 0;
    int bytes = 1;
    while ((c = getchar()) != EOF && c != '\n' && len < limit) {
        buf[i] = c;

        int next = mbc_len(c);
        if (next == 0)
            break;  // not sure whether it will occur or not
        else if (next != -1)
            bytes = next;

        if (--bytes <= 0) len++;
        ++i;
    }

    buf[i] = '\0';
    // flush input
    if (c != '\n' && c != EOF)
        while ((c = getchar()) != '\n' && c != EOF);
}

/* Determine the size, in bytes, of the multibyte character whose first byte is
 * `mbc`. Returns the number of bytes in multibyte character. If `mbc` is `\0`,
 * 0 is returned. If `mbc` do not form a valid multibyte character, -1 is
 * returned. Note that this function works only with UTF-8 encoding and assumes
 * that a multibyte character comprises from one to four bytes. */
int mbc_len(char mbc) {
    if (mbc == 0) return 0;               // '\0'
    if ((mbc & (1 << 7)) == 0) return 1;  // ascii

    int len = 0;
    for (int i = 7; i >= 3; --i) {
        if ((mbc & (1 << i)) == 0) break;
        ++len;
    }
    if (len <= 1 || len > 4) return -1;
    return len;
}

/* Returns `true` if all characters of string `s` are those, for which `isgraph`
 * returns zero, `false` otherwise. */
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
