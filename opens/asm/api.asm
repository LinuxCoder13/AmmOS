; /*
;  * AmmOS - Minimal Modular Operating System
;  * Copyright (C) 2025 Ammar Najafli
;  *
;  * This file is part of AmmOS.
;  *
;  * Permission is hereby granted, free of charge, to any person obtaining a copy
;  * of this software and associated documentation files (the "Software"), to deal
;  * in the Software without restriction, including without limitation the rights
;  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;  * copies of the Software, and to permit persons to whom the Software is
;  * furnished to do so, subject to the following conditions:
;  *
;  * The above copyright notice and this permission notice shall be included in all
;  * copies or substantial portions of the Software.
;  *
;  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;  * SOFTWARE.
;  */





; This file is for calling C-functhions in nasm 
; call Ammkernel

section .data

section .bss

section .text
    extern funcs ; c-funcs
    global Ammkernel

Ammkernel:

    push rbx                
    lea  rbx, [rel funcs]   ; RBX = &funcs[0]
    mov  rcx, [rbx + rax*8] ; RCX = funcs[rax]
    call rcx               
    pop  rbx                ; восстановить
    ret

    ; I didnt add the rbx != NULL or rax > 29 to now where I made mistake

section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw
