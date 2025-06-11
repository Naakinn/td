CC = gcc
TARGET_EXEC = td
SRC_DIR = src
BUILD_DIR = build
INC_DIR = include
CURRENT_MAKEFILE := $(lastword $(MAKEFILE_LIST))

INCFLAGS := -I./include
CFLAGS := -Wall -Werror -Wextra -std=c99 -MMD
SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:%.o=%.d)

EXTERNAL_SQLITE3 ?= OFF
ifneq ($(EXTERNAL_SQLITE3), ON)
	LDFLAGS := -lsqlite3
endif

all: release
debug: CFLAGS += -g -O0
debug: LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
release: CFLAGS += -DNDEBUG
	
debug: $(TARGET_EXEC)
release: $(TARGET_EXEC)

$(TARGET_EXEC): $(OBJS) $(CURRENT_MAKEFILE)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	
$(BUILD_DIR)/%.o: %.c $(CURRENT_MAKEFILE) | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@
	
$(BUILD_DIR)/$(SRC_DIR):
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET_EXEC)
	
-include $(DEPS)
