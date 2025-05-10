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
