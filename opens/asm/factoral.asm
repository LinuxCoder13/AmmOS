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



; this file made for finding factoral number, wrote in asembly for High speed
; include "../Ammkernel"

section .data
    msg1 db "Please enter unsigned number: "
    len1 equ $ - msg1
    erorr1 db "Number is unsigned!", 0xa ; \n
    len2 equ $ - erorr1

section .bss
    num1 resb 10 ; read the num1 as string
    res resb 100

section .text
global factoral
extern Ammkernel


factoral:
    push r8
    push r9
    push r10
    
    push rbp
    mov rbp, rsp ; зачем линкер? я есть линкер!

    ; write()
    mov rax, 1
    mov rdi, 1
    mov rsi, msg1
    mov rdx, len1
    syscall

    ; read()
    mov rax, 0
    mov rdi, 0
    mov rsi, num1
    mov rdx, 10
    syscall

    ; atoi()
    mov rax, 26
    mov rdi, num1
    call Ammkernel

    cmp rax, -1
    jle .erorr

    mov rdi, rax
    call .imull

    jmp .done 

.erorr:
    ; write()
    mov rax, 1
    mov rdi, 2 ; stderr
    mov rsi, erorr1
    mov rdx, len2
    syscall

    ret

.imull:

    mov rcx, rdi       ; rcx = счётчик
    mov rbx, 1         ; rbx = результат

.factorial_loop:
    imul rbx, rcx      ; rbx *= rcx
    loop .factorial_loop  ; rcx--; if rcx != 0 -> continue

    ; int_ascii(rbx, res)
    mov rax, 5
    mov rdi, rbx
    mov rsi, res
    call Ammkernel

    ; puts(res)
    mov rax, 24
    mov rdi, res
    call Ammkernel

    ret



.done:
    pop rbp
    pop r10
    pop r9
    pop r8

    ret
    

section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw