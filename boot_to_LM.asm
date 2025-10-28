; x86_64 Long Mode Bootloader
; THIS FILE IS FREE! 
; YOU CAN COPY TO YOUR Project OS AND USE IT (this is not part of AmmOS or Amm2OS!)

[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    

    mov si, msg_loading
    call print_string
    

    call check_long_mode
    jc .no_long_mode

    call setup_paging
    

    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:protected_mode

.no_long_mode:
    mov si, msg_no_lm
    call print_string
    jmp $

print_string:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; check if WM or BM supporting Long Mode thought CPUID
check_long_mode:
    pushfd  ; push EFLAGS(32 bit) to stack
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    jz .no_cpuid
    
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    
    clc
    ret

.no_cpuid:
.no_long_mode:
    stc
    ret


setup_paging:
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd
    mov edi, cr3
    
    ; if you don't understand how virtual memory works then check: https://wiki.osdev.org/Paging
    mov dword [edi], 0x2003      ; PML4[0] -> PDPT
    add edi, 0x1000
    
    mov dword [edi], 0x3003      ; PDPT[0] -> PDT
    add edi, 0x1000
    
    mov dword [edi], 0x00000083  ; PDT[0] -> 2MB page
    add edi, 8
    mov dword [edi], 0x00200083  ; PDT[1] -> 2MB page
      
    
    ;  PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    

    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    
    ret

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; addr = segment * 16 + offset (x86-16)
    jmp 0x18:long_mode

[BITS 64]
long_mode:
    mov rax, 0xB8000
    mov byte [rax], 'L'
    mov byte [rax+1], 0x0F
    mov byte [rax+2], 'M'
    mov byte [rax+3], 0x0F
    mov byte [rax+4], '!'
    mov byte [rax+5], 0x0A
    
    hlt
    jmp $

; Global Descriptor Table
gdt_start:
    dq 0x0000000000000000    ; Null 

gdt_code_32:
    dw 0xFFFF                ; Limit (0-15)
    dw 0x0000                ; Base (0-15)
    db 0x00                  ; Base (16-23)
    db 10011010b             ; Access: present, ring 0, code
    db 11001111b             ; Flags + Limit (16-19)
    db 0x00                  ; Base (24-31)

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b             ; Access: present, ring 0, data
    db 11001111b
    db 0x00

gdt_code_64:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b             ; Access: present, ring 0, code
    db 10101111b             ; Flags: 64-bit, Limit
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


msg_loading: db 'Loading Long Mode...', 13, 10, 0
msg_no_lm: db 'Long Mode not supported!', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55