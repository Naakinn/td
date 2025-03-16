CC = gcc
TARGET_EXEC = td
SRC_DIR = src
BUILD_DIR = build
INC_DIR = include
CURRENT_MAKEFILE := $(lastword $(MAKEFILE_LIST))

INCFLAGS := -I./include
LDFLAGS := -lsqlite3
CFLAGS := -Wall -Wextra -std=c99 -g -MMD

SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS := $(OBJS:%.o=%.d)

all: $(TARGET_EXEC) 

$(TARGET_EXEC): $(OBJS) Makefile
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
	
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)/$(SRC_DIR)
	$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $@
	
$(BUILD_DIR)/$(SRC_DIR):
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET_EXEC)
	
-include $(DEPS)
