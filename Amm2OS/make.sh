#!/bin/bash
#
# AmmOS - Minimal Modular Operating System
# Copyright (C) 2025 Ammar Najafli
# Copyright (C) 2026 Ayano4ka1338
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

# ------------------------------------------------------------
#  Збірка проекту – компіляція, компонування та створення ISO.
#  Використовується NASM, GCC, LD та genisoimage.
# ------------------------------------------------------------

set -e

BUILD_DIR=build
mkdir -p $BUILD_DIR

# Компіляція завантажувача (NASM)
nasm -f elf boot/boot.asm -o $BUILD_DIR/bootloader.o

# Компіляція ядра (GCC) – freestanding, без стандартних бібліотек
gcc -w -m32 -ffreestanding -fno-pic -c kernel/kernel.c -o $BUILD_DIR/kernel.o

# Компонування об'єктних файлів у виконуваний ELF
ld -m elf_i386 -T linker.ld -o $BUILD_DIR/kernel.elf \
    $BUILD_DIR/bootloader.o \
    $BUILD_DIR/kernel.o

# Перетворення ELF у плоский бінарний образ (для запису на диск)
objcopy -O binary $BUILD_DIR/kernel.elf $BUILD_DIR/bootloader.bin

# Автоматичне визначення розміру та кількості секторів
SIZE=$(stat -c %s $BUILD_DIR/bootloader.bin)
SECTORS=$(( (SIZE + 511) / 512 ))
echo "bootloader.bin size = $SIZE bytes, sectors = $SECTORS"

# Створення завантажувального ISO-образу
genisoimage -R -b bootloader.bin -no-emul-boot -boot-load-size $SECTORS -o Amm2OS.iso $BUILD_DIR/

# Очищення тимчасових файлів
rm -rf $BUILD_DIR

# Запуск у емуляторі QEMU
qemu-system-i386 -cdrom Amm2OS.iso
