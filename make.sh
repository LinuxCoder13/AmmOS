#!/usr/bin/bash
set -e # не ну а че

rm -f objfiles/*.o
mkdir -p objfiles

nasm -f elf64 opens/asm/api.asm -o objfiles/api.o
nasm -f elf64 opens/asm/lowlevel_Ammkernel.asm -o objfiles/lowlevel_Ammkernel.o

# compile all .c to objects for creating 1 full binary ;)
gcc -c opens/Ammkernel.c -o objfiles/Ammkernel.o
gcc -c opens/Ammshell.c -o objfiles/Ammshell.o
gcc -c opens/AmmFS.c -o objfiles/AmmFS.o

# make OS
gcc objfiles/Ammshell.o \
 objfiles/Ammkernel.o \
 objfiles/AmmFS.o \
 objfiles/lowlevel_Ammkernel.o \
 objfiles/api.o \
 -o opens/Ammshell -no-pie  # I did somesing

# bash-script > Makefile
 
