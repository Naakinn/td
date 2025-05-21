#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>

// developer's convenience
#ifdef __GNUC__
#define UNUSED(x) UNUSED_##x __attribute__((__unused__))
#else
#define UNUSED(x) UNUSED_##x
#endif

// configs
#define LINE_SIZE 40
#define LINE_SIZE_EXT 200
#define SQL_SIZE (LINE_SIZE_EXT + LINE_SIZE + 10)
#define AMEND_NAME 2
#define AMEND_NOTE 3

// Types & enums
typedef enum {
    HelpCmd = 1,
    ListCmd,
    InfoCmd,
    PushCmd,
    AmendCmd,
    DropCmd,
    NullCmd,
} Command;

// error output
#ifndef NDEBUG
#define error(format, ...)                                          \
    fprintf(stderr, "td: (%s:%d) " format, __FILE_NAME__, __LINE__, \
            ##__VA_ARGS__)
#else
#define error(format, ...) fprintf(stderr, "td: " format, ##__VA_ARGS__)
#endif

#define defer(rc, res)  \
    do {            \
        rc = res; \
        goto defer;  \
    } while (0)
#endif
