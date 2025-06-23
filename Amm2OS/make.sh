#!/bin/bash

# Компиляция ASM
nasm -f elf32 kernel.asm -o kernel_asm.o

# Компиляция C
gcc -m32 -c kernel.c -o kernel_c.o -nostdlib -ffreestanding -O2

# Линковка
ld -m elf_i386 -T linker.ld -o kernel.bin kernel_asm.o kernel_c.o

# Создание загрузочного образа
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub/grub.cfg iso/boot/grub/
grub-mkrescue -o Amm2OS.iso iso/

# Запуск QEMU
qemu-system-i386 -cdrom Amm2OS.iso