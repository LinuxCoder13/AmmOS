!section .text
astrcmp:
    // rdi = char*, rsi = char*
    test %rdi, %rdi
    j! .null
    test %rsi, %rsi
    j! .null

    xor %rcx, %rcx

.loop:
    mov %al, [%rdi + %rcx]
    mov %bl, [%rsi + %rcx]
    cmp %al, %bl
    j!= .done
    test %al, %al
    j== .done
    inc %rcx
    jmp .loop

.done:
    movzx %eax, %al
    movzx %ebx, %bl
    sub %eax, %ebx
    
    // syscall exit
    mov rdi, eax
    mov rax, 60
    syscall


.null:
    mov %eax, -1
    ret