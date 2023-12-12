BUILD_DIR := build

all: non_optimized assembly_testing

assembly_testing: $(BUILD_DIR)
	nasm -felf64 -o $(BUILD_DIR)/main.o ./main.asm
	ld -o $(BUILD_DIR)/a.out $(BUILD_DIR)/main.o
	rm $(BUILD_DIR)/main.o

non_optimized: $(BUILD_DIR)
	clang -S -o $(BUILD_DIR)/c-clang-main.s ./main.c
	gcc -S -o $(BUILD_DIR)/c-gcc-main.s ./main.c
	rustc --emit asm -o $(BUILD_DIR)/rust-main.s ./main.rs

optimized: builddir
	clang -O3 -S -o $(BUILD_DIR)/c-clang-main.s ./main.c
	gcc -O3 -S -o $(BUILD_DIR)/c-gcc-main.s ./main.c
	rustc -O --emit asm -o $(BUILD_DIR)/rust-main.s ./main.rs

clear: builddir
	rm $(BUILD_DIR)/*.s

$(BUILD_DIR):
	mkdir $@
