cmake_minimum_required(VERSION 3.0)

project(elysia)

add_executable(
    elysia
    ./src/main.c
    ./src/lexer.c
    ./src/parser.c
    ./src/compiler.c
)

target_compile_options(elysia PUBLIC -Wall -g)
