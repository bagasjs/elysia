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
target_include_directories(elysia PUBLIC ./src)

add_executable(LexerTest ./tests/test_lexer.c ./src/lexer.c ./src/compiler.c)
target_compile_options(LexerTest PUBLIC -Wall -g)
target_include_directories(LexerTest PUBLIC ./src)
add_test(NAME LexerTest COMMAND LexerTest)
