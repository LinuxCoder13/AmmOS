BITS 16
global _start
extern kmain

section .text

_start:
    cli
    mov ax, 0x3
    int 0x10

    lgdt [gdt_pointer]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm


gdt_start:
    dq 0x0         
gdt_code:
    dw 0xFFFF   
    dw 0x0       
    db 0x0
    db 10011010b     
    db 11001111b    
    db 0x0
gdt_data:
    dw 0xFFFF                
    dw 0x0              
    db 0x0
    db 10010010b          
    db 11001111b  
    db 0x0
gdt_end:

gdt_pointer:
    dw gdt_end - gdt_start - 1    
    dd gdt_start    

CODE_SEG equ gdt_code - gdt_start 
DATA_SEG equ gdt_data - gdt_start  

BITS 32
init_pm:
    cli
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x10000
    call kmain             

hang:
    hlt
    jmp hang

times 510-($-$$) db 0
dw 0xaa55
