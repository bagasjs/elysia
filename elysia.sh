#/bin/sh

./build/elysia com $@
nasm -felf64 ./output.asm
ld -o output ./output.o ./runtime/elysiart_x86-64.o
rm ./output.o
