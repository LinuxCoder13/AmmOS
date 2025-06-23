section .multiboot
align 4
dd 0x1BADB002               ; Magic number
dd 0x00                      ; Flags
dd -(0x1BADB002 + 0x00)      ; Checksum

section .text
global _start
extern kmain                ; Объявляем внешнюю функцию из C

_start:
    cli                     ; Отключить прерывания
    mov esp, stack_top      ; Установить стек
    
    push eax                ; Передаем магический номер Multiboot
    push ebx                ; Передаем адрес структуры Multiboot
    
    call kmain              ; Вызываем главную функцию на C
    
    hlt                     ; Остановка процессора после возврата

section .bss
align 16
stack_bottom:
resb 16384                  ; 16KB стека
stack_top: