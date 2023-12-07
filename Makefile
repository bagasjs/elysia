CC := /usr/bin/gcc
BUILD_DIR := ./build/
CFLAGS := -Wall -Wextra -Wpedantic
LFLAGS :=
SOURCES := ./src/elysia_compiler.c
SOURCES += ./src/elysia_parser.c
SOURCES += ./src/elysia_lexer.c
SOURCES += ./src/main.c

all: build

build: $(SOURCES) $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/elysia $(SOURCES) $(LFLAGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
