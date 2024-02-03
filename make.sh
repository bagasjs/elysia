#!/bin/sh

CC="/usr/bin/gcc"
BUILD_DIR="./build/"
TARGET="elysia"
CFLAGS="-Wall -Wextra -Wpedantic -ggdb"
LFLAGS="-L build"
SOURCES=(
    "./src/elysia.c"
    "./src/elysia_ast.c"
    "./src/elysia_parser.c"
    "./src/elysia_types.c"
    "./src/elysia_lexer.c"
    "./src/elysia_compiler.c"
    "./src/elysia_compiler_x86_64_nasm.c"

    "./src/main.c"
)

compile_sources() {
    local sources=("$@")

    if [ ! -d $BUILD_DIR/cache ]; then
        mkdir $BUILD_DIR/cache
    fi

    local objs=()
    for source_file in "${sources[@]}"; do
        file_name=$(basename "$source_file")
        file_name_without_ext="${file_name%.*}"
        echo "Compiling $file_name"
        $CC $CFLAGS -c -o "$BUILD_DIR/cache/$file_name_without_ext.o" $source_file
        objs+=("$BUILD_DIR/cache/$file_name_without_ext.o")
    done

    echo "Linking $TARGET"
    gcc $LFLAGS -o "$BUILD_DIR/$TARGET" "${objs[@]}"
}

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

compile_sources "${SOURCES[@]}"


