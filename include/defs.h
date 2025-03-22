#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>

#define PASS  // python-like pass operator
#define LINE_SIZE 40
#define LINE_SIZE_EXT 200
#define SQL_SIZE (LINE_SIZE_EXT + LINE_SIZE + 10)

#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

#define error(format, ...)                                          \
    fprintf(stderr, "td: (%s:%d) " format, __FILE_NAME__, __LINE__, \
            ##__VA_ARGS__)

#endif
