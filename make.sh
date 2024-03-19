#!/bin/sh

CC="/usr/bin/gcc"
BUILD_DIR="./build/"
TARGET="elysia"
CFLAGS="-Wall -Wextra -Wpedantic -g -I./vendors/qbe/include/"
LFLAGS="-L build"

ELYSIA_SOURCES_BACKEND_NASM=(
    "./src/elysia.c"
    "./src/elysia_ast.c"
    "./src/elysia_parser.c"
    "./src/elysia_types.c"
    "./src/elysia_lexer.c"
    "./src/elysia_compiler.c"
    "./src/elysia_compiler_backend_x86_64_nasm.c"

    "./src/main.c"
)

QBE_SOURCES=(
    "./vendors/qbe/src/abi.c"
    "./vendors/qbe/src/alias.c"
    "./vendors/qbe/src/cfg.c"
    "./vendors/qbe/src/copy.c"
    "./vendors/qbe/src/emit.c"
    "./vendors/qbe/src/fold.c"
    "./vendors/qbe/src/libqbe.c"
    "./vendors/qbe/src/live.c"
    "./vendors/qbe/src/load.c"
    "./vendors/qbe/src/mem.c"
    "./vendors/qbe/src/parse.c"
    "./vendors/qbe/src/rega.c"
    "./vendors/qbe/src/simpl.c"
    "./vendors/qbe/src/spill.c"
    "./vendors/qbe/src/ssa.c"
    "./vendors/qbe/src/util.c"

    "./vendors/qbe/src/amd64/amd64_emit.c"
    "./vendors/qbe/src/amd64/amd64_isel.c"
    "./vendors/qbe/src/amd64/amd64_sysv.c"
    "./vendors/qbe/src/amd64/amd64_targ.c"

    "./vendors/qbe/src/arm64/arm64_emit.c"
    "./vendors/qbe/src/arm64/arm64_isel.c"
    "./vendors/qbe/src/arm64/arm64_abi.c"
    "./vendors/qbe/src/arm64/arm64_targ.c"

    "./vendors/qbe/src/rv64/rv64_emit.c"
    "./vendors/qbe/src/rv64/rv64_isel.c"
    "./vendors/qbe/src/rv64/rv64_abi.c"
    "./vendors/qbe/src/rv64/rv64_targ.c"
)

ELYSIA_SOURCES_BACKEND_QBE=(

    "./src/elysia.c"
    "./src/elysia_ast.c"
    "./src/elysia_parser.c"
    "./src/elysia_types.c"
    "./src/elysia_lexer.c"
    "./src/elysia_compiler.c"
    "./src/elysia_compiler_backend_qbe.c"
    "./src/plenary.c"
    "./src/main.c"
)

OBJECTS=()

compile_sources() {
    local cflags="$1"
    local cache_dir="$2"
    shift 2
    local sources=("$@")

    for source_file in "${sources[@]}"; do
        file_name=$(basename "$source_file")
        file_name_without_ext="${file_name%.*}"
        echo "Compiling $file_name"
        $CC $cflags -c -o "$cache_dir/$file_name_without_ext.o" $source_file

        OBJECTS+=("$cache_dir/$file_name_without_ext.o")
    done
}

link_executable() {
    local name="$1"
    local location="$2"
    local lflags="$3"
    shift 3
    local objects=("$@")

    echo "Linking executable $name"
    $CC $lflags -o "$location/$name" "${objects[@]}"
}

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

if [ ! -d $BUILD_DIR/cache ]; then
    mkdir $BUILD_DIR/cache
fi

if [ ! -d $BUILD_DIR/cache/qbe ]; then
    mkdir $BUILD_DIR/cache/qbe
fi

echo "--- Compiling Elysia With QBE Backend ---"
compile_sources "$CFLAGS" "$BUILD_DIR/cache" "${ELYSIA_SOURCES_BACKEND_QBE[@]}"
# compile_sources "-std=c99 -g -Wall -Wextra -Wpedantic" "$BUILD_DIR/cache/qbe" "${QBE_SOURCES[@]}"
link_executable "${TARGET}-qbe" "$BUILD_DIR" "$LFLAGS" "${OBJECTS[@]}"

