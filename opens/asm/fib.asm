; this file made for finding fib number, wrote in asembly for High speed
; include "../Ammkernel"

section .data 
    msg1 db "Enter number of iterations: "
    len1 equ $ - msg1
    msg2 db "To big number, max '92'", 10, 10, 0
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
    cmp rcx, 93
    jge .toooo_big
    
    ; // start
    mov r8, 0
    mov r9, 1

    ; rcx = number of iterations
    ; r8 = 0
    ; r9 = 1

; must be O(n)
.next_num:
    cmp rcx, 1
    jbe .printnum

    mov rax, r9     ; rax = r9 
    add r9, r8      ; r9 = r9 + r8
    mov r8, rax     ; r8 = old r9
    
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