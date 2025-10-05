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



; made for high speed
section .text
global astrcmp 

astrcmp:
    test rdi, rdi
    jz .null
    test rsi, rsi
    jz .null

    xor rcx, rcx

.loop:
    mov al, [rdi + rcx]
    mov bl, [rsi + rcx]
    cmp al, bl
    jne .done
    test al, al
    je .done
    inc rcx
    jmp .loop

.done:
    movzx eax, al
    movzx ebx, bl
    sub eax, ebx
    ret

.null:
    mov eax, -1
    ret



section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw
