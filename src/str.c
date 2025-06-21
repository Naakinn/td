#include "str.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wctype.h>

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

/* Convert string `s` to integer. Warning: This function assumes that `s`
 * comprises only digits: '0123456789'. */
int str_toi(const char* s) {
    int res = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        res = res * 10 + (s[i] - '0');
    }
    return res;
}

/* Removes characters of string `s` one by one (including delimiter) (starting
 * from the end of `s`) until it meets delimiter `d`. */
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
    mblen(NULL, 0);  // reset the conversion state
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

/* Determine the length of multibyte UTF-8 string `s` */
size_t mbstr_len(const char* s) {
    size_t res = 0;
    mblen(NULL, 0);  // reset the conversion state
    const char* end = s + strlen(s);
    while (s < end) {
        int bytes = mbc_len(*s);
        if (bytes == 0) return res;
        s += bytes;
        ++res;
    }
    return res;
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

/* Returns `true` if all characters of multibyte UTF-8 string `s` are those, for
 * which `iswgraph` returns zero, `false` otherwise. */
bool mbstr_isempty(const char* s) {
    if (s == NULL) return true;
    const char* end = s + strlen(s);
    while (s < end) {
        wchar_t wc;
        int bytes = mbc_len(*s);
        mbtowc(&wc, s, end - s);
        if (iswgraph(wc) != 0) return false;
        s += bytes;
    }
    return true;
}

/* Returns `true` if all characters of multibyte UTF-8 string `s` are those, for
 * which `iswdigit` returns non-zero, `false` otherwise. */
bool mbstr_isnumeric(const char* s) {
    if (s == NULL) return false;
    const char* end = s + strlen(s);
    while (s < end) {
        wchar_t wc;
        int bytes = mbc_len(*s);
        mbtowc(&wc, s, end - s);
        if (iswdigit(wc) == 0) return false;
        s += bytes;
    }
    return true;
}

/* Convert multibyte UTF-8 string `s` to integer. Warning: this function assumes
 * that `s` comprises only digits '0123456789'. This function does not support
 * any non-ASCII digit characters. Therefore it's equivalent to `str_toi`. */
int mbstr_toi(const char* s) { return str_toi(s); }

/* Removes characters of multibyte UTF-8 string `s` one by one (including
 * delimiter) (starting from the end of `s`) until it meets delimiter `d`. This
 * function is equivalent to `str_delimt_right`. */
void mbstr_delim_right(char* s, char d) { str_delim_right(s, d); }
