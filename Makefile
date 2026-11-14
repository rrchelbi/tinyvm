CC ?= cc
CSTD ?= -std=c23
CFLAGS ?= -O2 -g
CFLAGS += $(CSTD) -Wall -Wextra -Wpedantic
CPPFLAGS += -D_GNU_SOURCE -Iinclude -MMD -MP

WERROR ?= 0
SANITIZE ?= 0

ifeq ($(WERROR),1)
CFLAGS += -Werror
endif

ifeq ($(SANITIZE),1)
CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

TARGET := tinyvm
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TEST_DIR := $(BUILD_DIR)/tests
SRC_DIR := src

SOURCES := main.c
OBJECTS := $(SOURCES:%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

TEST_BINS := $(TEST_DIR)/test_smoke
TEST_DEPS := $(TEST_BINS:=.d)

.PHONY: all test check tools clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_DIR)/%: tests/unit/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) -o $@ $<

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "== $$test"; \
		./$$test || exit 1; \
	done
	@echo "All unit tests passed."

check: test

tools:
	@mkdir -p $(BUILD_DIR)/tools

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS) $(TEST_DEPS)

