#!/usr/bin/env bash

# Usage: acc myfile.c -o myfile

gcc -nostdlib -static -fno-pie -fno-builtin -no-pie $1 ~/.local/ANG/alibc/*.c -o $2

