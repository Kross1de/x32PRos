CC = gcc
LD = ld -m elf_i386

CFLAGS = -Wall -Wextra -pedantic -m32 -O0 -std=c99 -finline-functions -nostdinc -ffreestanding -fno-stack-protector
NASM = nasm -f elf
