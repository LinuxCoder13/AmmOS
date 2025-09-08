#!/usr/bin/bash
set -e # не ну а че

rm -f objfiles/*.o
mkdir -p objfiles

nasm -f elf64 opens/asm/api.asm -o objfiles/api.o
nasm -f elf64 opens/asm/calc.asm -o objfiles/calc.o
nasm -f elf64 opens/asm/fib.asm -o objfiles/fib.o
nasm -f elf64 opens/asm/factoral.asm -o objfiles/factoral.o
nasm -f elf64 opens/asm/strcmp.asm -o objfiles/strcmp.o

# compile all .c to objects for creating 1 full binary ;)
gcc -c opens/Ammkernel.c     -o objfiles/Ammkernel.o
gcc -c opens/AmmFS.c         -o objfiles/AmmFS.o
gcc -c opens/Ammgpu.c        -o objfiles/Ammgpu.o
gcc -c opens/AmmSH.c         -o objfiles/AmmSH.o
gcc -c opens/Ammsystemd.c    -o objfiles/Ammsystemd.o
gcc -c opens/configs/demon.c -o objfiles/demon.o
gcc -c opens/configs/user.c  -o objfiles/user.o

# make OS
gcc -g -O2 objfiles/Ammkernel.o \
 objfiles/AmmFS.o \
 objfiles/calc.o \
 objfiles/api.o \
 objfiles/fib.o \
 objfiles/Ammgpu.o \
 objfiles/AmmSH.o \
 objfiles/Ammsystemd.o \
 objfiles/factoral.o \
 objfiles/strcmp.o \
 objfiles/user.o \
 objfiles/demon.o \
 -o opens/AmmOS -no-pie  # I did somesing

# bash-script > Makefile
 
