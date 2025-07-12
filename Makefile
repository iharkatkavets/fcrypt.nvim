BUILD ?= release

LUAJIT_INCLUDE ?= $(shell pkg-config --cflags luajit)
LUAJIT_LIB ?= $(shell pkg-config --libs luajit)

CC = gcc

ifeq ($(BUILD), debug)
	CFLAGS = -Iinclude -Wall -Wextra -g0 -DDEBUG
else ifeq ($(BUILD), release)
	CFLAGS = -Iinclude -Wall -Wextra -g0 -O2
else
    $(error Unknown build type "$(BUILD)". Use "debug" or "release".)
endif

SRC_DIR = src
TEST_DIR := test
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin
LOG_DIR := test_logs
TEST_LOGS_DIR := $(patsubst %, $(LOG_DIR)/%.log, $(notdir $(TEST_BINS)))

COMMON_SRC = $(SRC_DIR)/common_utils.c $(SRC_DIR)/convert_utils.c $(SRC_DIR)/input.c $(SRC_DIR)/sha256.c $(SRC_DIR)/xchacha20.c $(SRC_DIR)/verbose.c $(SRC_DIR)/opts_utils.c $(SRC_DIR)/encrypt.c $(SRC_DIR)/decrypt.c $(SRC_DIR)/file_utils.c $(SRC_DIR)/random.c
TOOL_SRC = $(SRC_DIR)/main.c
LUA_SRC = $(SRC_DIR)/lua_binding.c

TEST_SRCS := $(wildcard $(TEST_DIR)/test_*.c)
TEST_BINS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%)

COMMON_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(COMMON_SRC))
TOOL_OBJ = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(TOOL_SRC))

TOOL = fcrypt
TOOL_PATH = $(BIN_DIR)/$(TOOL)
LUA_MODULE = fcrypt_lua_mod.so
LUA_MODULE_PATH = lua/fcrypt/$(LUA_MODULE)

$(shell mkdir -p $(BUILD_DIR) $(BIN_DIR))

all: $(TOOL_PATH) $(LUA_MODULE_PATH)

$(TOOL): $(TOOL_PATH)

$(TOOL_PATH): $(TOOL_OBJ) $(COMMON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LUA_MODULE_PATH): $(LUA_SRC) $(COMMON_SRC)
	@$(MAKE) check_luajit
	$(CC) $(CFLAGS) -fPIC -shared $(LUAJIT_INCLUDE) $(LUAJIT_LIB) -o $@ $^

lua: check_luajit $(LUA_MODULE_PATH)

check_luajit:
	@if ! pkg-config --exists luajit; then \
		echo "LuaJIT development files not found. Please install LuaJIT dev files: libluajit-5.1-dev (Linux) or brew install luajit (macOS)."; \
		exit 1; \
		fi

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(TEST_LOGS_DIR) $(LUA_MODULE_PATH)

debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release

$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(COMMON_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test: $(TEST_BINS)
	@mkdir -p $(LOG_DIR)
	@for bin in $(TEST_BINS); do \
		log="$(LOG_DIR)/$$(basename $$bin).log"; \
		printf "Running $$bin... "; \
		if $$bin > "$$log" 2>&1; then \
			echo " OK"; \
		else \
			echo " FAIL (see $$log)"; \
			tail -n 10 "$$log"; \
			exit 1; \
		fi \
	done

.PHONY: all clean lua test check_luajit
