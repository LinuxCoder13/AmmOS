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



; I really dont want to write
; my own asembly. So lowlevel_Ammkernel its high level of Nasm in AmmOS 2025 - ... 

; we need only funcs[] and funcs in. also see funcs in ../Ammkernel.c
; #include "../Ammkernel.h" 


section .text
    extern Ammkernel
    global calc

calc:

    mov rax, 1
    mov rdi, 1
    mov rsi, msg1
    mov rdx, len1
    syscall ; write()

    
    mov rax, 0
    mov rdi, 0
    mov rsi, num1
    mov rdx, 12
    syscall ; read()
    mov byte [num1 + rax], 0 ; '\0'

    mov rax, 1
    mov rdi, 1
    mov rsi, msg2
    mov rdx, len2
    syscall ; write()

    mov rax, 0
    mov rdi, 0
    mov rsi, num2
    mov rdx, 12
    syscall ; read()
    mov byte [num2 + rax], 0 ; '\0'

    mov rax, 1
    mov rdi, 1
    mov rsi, choise1
    mov rdx, len3
    syscall ; write()

    mov rax, 0
    mov rdi, 0
    mov rsi, operand
    mov rdx, 1
    syscall ; read()
    
    ; eat leftover \n  I WAS FIXING THIS BUG 2 DAYS :( with out ai :)
    mov rax, 0
    mov rdi, 0
    mov rsi, dummy
    mov rdx, 1
    syscall

    ; atoi()
    mov rax, 26
    mov rdi, num1
    call Ammkernel
    mov rbx, rax

    ; atoi()
    mov rax, 26
    mov rdi, num2
    call Ammkernel
    mov r8, rax

    mov al, byte [operand] 

    cmp al, '+' ; I made only this
    je .summ

    cmp al, '-' ; and this
    je .diference

    cmp al, '*'
    je .imull

    cmp al, '/'
    je .dev
    

    ; rbx = atoi(num1)
    ; r8 = atoi(num2)

    mov rax, 1 ; не ну а че
    ret



.summ:

    add r8, rbx ; r8 += rbx

    ; int_ascii()
    mov rax, 5
    mov rdi, r8
    mov rsi, res ; char res[20];
    call Ammkernel 

    ; puts()
    mov rax, 24
    mov rdi, res
    call Ammkernel

    ret
    
.diference:

    sub rbx, r8 ; rbx -= r8 

    ; int_ascii()
    mov rax, 5
    mov rdi, rbx
    mov rsi, res ; char res[100];
    call Ammkernel 

    ; puts()
    mov rax, 24
    mov rdi, res
    call Ammkernel  

    ret

.imull:
    imul rbx, r8

    ; int_ascii()
    mov rax, 5
    mov rdi, rbx
    mov rsi, res ; char res[100];
    call Ammkernel

    ; puts()
    mov rax, 24
    mov rdi, res
    call Ammkernel
    
    ret


.dev:
    test r8, r8        
    jz .div_by_zero    

    mov rax, rbx        
    cqo         ;  <------- rdx:rax 
    idiv r8     ; int part = rax; float part = rdx 

    ; int_ascii()
    mov rdi, rax        
    mov rsi, res        
    mov rax, 5          
    call Ammkernel

   ;  write()
    mov rax, 1          
    mov rdi, 1         
    mov rsi, res        
    mov rdx, 20         
    syscall

    ret                 

.div_by_zero:

    ; kernel_panic()
    mov rax, 27         
    call Ammkernel
    

section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw

section .rodata
    msg1 db "Enter num1: ", 
    len1 equ $ - msg1
    msg2 db "Enter num2: ",
    len2 equ $ - msg2 
    choise1 db "Enter oper(+, -, *, /): "
    len3 equ $ - choise1



section .bss
    num1 resb 20   ; len of num1 as string
    num2 resb 20   ; len of num2 as string
    operand resb 1 ; char operand

    res resb 100 ; char res[100];
    dummy resb 1 ; char dummy;
    

; sorry for my english :(
 


