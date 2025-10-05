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


; this file made for finding fib number, wrote in asembly for High speed
; include "../Ammkernel"

section .data 
    msg1 db "Enter number of iterations: "
    len1 equ $ - msg1
    msg2 db "To big number, max '185'", 10, 10, 0
    len2 equ $ - msg2

section .bss
    num_s resb 4 ; 1 - 9999
    null resb 1 ; for \n

section .text
extern Ammkernel
global fib


fib:
    push r8
    push r9
    push r10
    push r11


    ; write()
    mov rax, 1
    mov rdi, 1
    mov rsi, msg1
    mov rdx, len1
    syscall
    
    ; read()
    mov rax, 0
    mov rdi, 0
    mov rsi, num_s
    mov rdx, 4
    syscall


    ; atoi()
    mov rax, 26
    mov rdi, num_s
    call Ammkernel

    mov rcx, rax
    cmp rcx, 0 ; Do you think I am stupid ?!
    jle .done
    cmp rcx, 185
    jge .toooo_big

    mov r9, 1
    mov r8, 0
; must be O(n)

; 128bit fib
.next_num:
    cmp rcx, 1
    jbe .printnum

    ; tmp = curr
    mov r9, r8
    mov rsi, rdx

    ; curr = curr + prev
    add r8, rax
    adc rdx, rbx

    ; prev = tmp
    mov rax, r9
    mov rbx, rsi

    dec rcx
    jnz .next_num

.printnum:
    
    ; int_ascii()
    mov rax, 5
    mov rdi, r9
    mov rsi, num_s
    call Ammkernel

    mov rax, 24
    mov rdi, num_s
    call Ammkernel

    jmp .done

.toooo_big:

    mov rax, 1
    mov rdi, 1
    mov rsi, msg2
    mov rdx, len2
    syscall

    jmp .done

.done:
    pop r11
    pop r10
    pop r9
    pop r8

    mov rax, 1
    ret


section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw