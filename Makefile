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
ifeq ($(SANITIZE),1)
BUILD_DIR := build/sanitize
endif

OBJ_DIR := $(BUILD_DIR)/obj
TEST_DIR := $(BUILD_DIR)/tests
TOOL_DIR := $(BUILD_DIR)/tools
SRC_DIR := src

LIB_SOURCES := kvm.c
CLI_SOURCES := main.c

LIB_OBJECTS := $(LIB_SOURCES:%.c=$(OBJ_DIR)/%.o)
CLI_OBJECTS := $(CLI_SOURCES:%.c=$(OBJ_DIR)/%.o)
OBJECTS := $(CLI_OBJECTS) $(LIB_OBJECTS)
DEPS := $(OBJECTS:.o=.d)

TEST_BINS := $(TEST_DIR)/test_smoke $(TEST_DIR)/test_vm
TEST_DEPS := $(TEST_BINS:=.d)

TOOL_BINS := $(TOOL_DIR)/kvm_probe
TOOL_DEPS := $(TOOL_BINS:=.d)

.PHONY: all test check tools clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(TEST_DIR)/%: tests/unit/%.c $(LIB_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -Itests $(CFLAGS) -o $@ $< $(LIB_OBJECTS)

$(TOOL_DIR)/%: tools/%.c $(LIB_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LIB_OBJECTS)

test: $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "== $$test"; \
		./$$test || exit 1; \
	done
	@echo "All unit tests passed."

check: test

tools: $(TOOL_BINS)

clean:
	rm -rf build $(TARGET)

-include $(DEPS) $(TEST_DEPS) $(TOOL_DEPS)

