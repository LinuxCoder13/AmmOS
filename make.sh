#!/usr/bin/bash
set -e # не ну а че

rm -f objfiles/*.o
mkdir -p objfiles

nasm -f elf64 opens/asm/api.asm -o objfiles/api.o
nasm -f elf64 opens/asm/calc.asm -o objfiles/calc.o
nasm -f elf64 opens/asm/fib.asm -o objfiles/fib.o

# compile all .c to objects for creating 1 full binary ;)
gcc -c opens/Ammkernel.c -o objfiles/Ammkernel.o
gcc -c opens/Ammshell.c -o objfiles/Ammshell.o
gcc -c opens/AmmFS.c -o objfiles/AmmFS.o
gcc -c opens/Ammgpu.c -o objfiles/Ammgpu.o

# make OS
gcc objfiles/Ammshell.o \
 objfiles/Ammkernel.o \
 objfiles/AmmFS.o \
 objfiles/calc.o \
 objfiles/api.o \
 objfiles/fib.o \
 objfiles/Ammgpu.o \
 -o opens/Ammshell -no-pie  # I did somesing

# bash-script > Makefile
 
