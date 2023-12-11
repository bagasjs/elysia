#!/bin/sh

CC="/usr/bin/gcc"
BUILD_DIR="./build/"
CFLAGS="-Wall -Wextra -Wpedantic"
SOURCES="
./src/elysia_compiler.c
./src/elysia_parser.c
./src/elysia_lexer.c
./src/main.c
./src/elysia_ast.c
"

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

$CC $CFLAGS -o $BUILD_DIR/elysia $SOURCES $LFLAGS


