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
#define LINE_LEN 40
#define LINE_LEN_EXT 200
#define MB_MAX 4 // 4 bytes in UTF-8
#define AMEND_NAME 2
#define AMEND_NOTE 3

// Types & enums
typedef enum {
    NullCmd = 0,
    ListCmd,
    InfoCmd,
    PushCmd,
    AmendCmd,
    DropCmd,
    LocalCmd,
} eCommandType;

typedef struct {
    eCommandType type;
    char* arg;
} Command;

struct Config {
    bool confirm;
};

// error output
#ifndef NDEBUG
#define error(format, ...)                                          \
    fprintf(stderr, "td: (%s:%d) " format, __FILE_NAME__, __LINE__, \
            ##__VA_ARGS__)
#else
#define error(format, ...) fprintf(stderr, "td: " format, ##__VA_ARGS__)
#endif

#define defer(rc, res) \
    do {               \
        rc = res;      \
        goto defer;    \
    } while (0)
#endif
