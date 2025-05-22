#include "str.h"

#include <stdio.h>
#include <string.h>

void str_readline(char* buf, int limit, const char* prompt) {
    printf("%s", prompt);
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

bool str_isempty(const char* s) {
    if (s == NULL) return true;
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] >= '!') {  // first non-blank ascii char
            return false;
        }
    }
    return true;
}
bool str_isnumeric(const char* s) {
    if (s == NULL) return false;
    for (size_t i = 0; i < strlen(s); ++i) {
        if (s[i] <= '/' || s[i] >= ':') {  // not a digit
            return false;
        }
    }
    return true;
}

int str_toi(const char* s) {
    int res = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        res = res * 10 + (s[i] - '0');
    }
    return res;
}
