all:
	clang -S -o ./c-clang-main.s ./main.c
	gcc -S -o ./c-gcc-main.s ./main.c
	rustc --emit asm -o ./rust-main.s ./main.rs

optimized:
	clang -O3 -S -o ./c-clang-main.s ./main.c
	gcc -O3 -S -o ./c-gcc-main.s ./main.c
	rustc -O --emit asm -o ./rust-main.s ./main.rs

clear:
	rm *.s
