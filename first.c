#include <stdio.h>
#define FIRST_IMPLEMENTATION
#include "src/first.h"

#define CC "clang"
#define BUILD_DIR "build"
const char *elysia_sources[] = {
    "./src/elysia.c",
    "./src/elysia_ast.c",
    "./src/elysia_parser.c",
    "./src/elysia_types.c",
    "./src/elysia_lexer.c",
    "./src/elysia_compiler.c",
    "./src/elysia_compiler_backend_qbe.c",
    "./src/main.c",
};

const char *qbe_sources[] = {
    "./vendors/qbe/src/abi.c",
    "./vendors/qbe/src/alias.c",
    "./vendors/qbe/src/cfg.c",
    "./vendors/qbe/src/copy.c",
    "./vendors/qbe/src/emit.c",
    "./vendors/qbe/src/fold.c",
    "./vendors/qbe/src/main.c",
    "./vendors/qbe/src/live.c",
    "./vendors/qbe/src/load.c",
    "./vendors/qbe/src/mem.c",
    "./vendors/qbe/src/parse.c",
    "./vendors/qbe/src/rega.c",
    "./vendors/qbe/src/simpl.c",
    "./vendors/qbe/src/spill.c",
    "./vendors/qbe/src/ssa.c",
    "./vendors/qbe/src/util.c",

    "./vendors/qbe/src/amd64/amd64_emit.c",
    "./vendors/qbe/src/amd64/amd64_isel.c",
    "./vendors/qbe/src/amd64/amd64_sysv.c",
    "./vendors/qbe/src/amd64/amd64_targ.c",

    "./vendors/qbe/src/arm64/arm64_emit.c",
    "./vendors/qbe/src/arm64/arm64_isel.c",
    "./vendors/qbe/src/arm64/arm64_abi.c",
    "./vendors/qbe/src/arm64/arm64_targ.c",

    "./vendors/qbe/src/rv64/rv64_emit.c",
    "./vendors/qbe/src/rv64/rv64_isel.c",
    "./vendors/qbe/src/rv64/rv64_abi.c",
    "./vendors/qbe/src/rv64/rv64_targ.c",
};

int main(int argc, char **argv)
{
    REBUILD_URSELF(argc, argv);

    CMD cmd = {0};

    if(!fs_is_exists(BUILD_DIR)) fs_mkdir(BUILD_DIR, 0755);

    cmd_append(&cmd, CC);
    cmd_append(&cmd, "-Wall", "-Wextra");
    cmd_append(&cmd, "-o", BUILD_DIR"/elysia");

    for(size_t i = 0; i < ARRAY_LENGTH(elysia_sources); ++i) {
        da_append(&cmd, elysia_sources[i]);
    }

    shell_exec_cmd(cmd);
    da_reset(&cmd);

    if(!fs_is_exists(BUILD_DIR"/qbe")) {
        cmd_append(&cmd, CC);
        cmd_append(&cmd, "-Wall", "-Wextra", "-I./vendors/qbe/include/", "-std=c99");
        cmd_append(&cmd, "-o", BUILD_DIR"/qbe");
        for(size_t i = 0; i < ARRAY_LENGTH(qbe_sources); ++i) {
            da_append(&cmd, qbe_sources[i]);
        }
        shell_exec_cmd(cmd);
    }


    da_free(&cmd);
}
