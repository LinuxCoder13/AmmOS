;
; AmmOS - Minimal Modular Operating System
; Copyright (C) 2025 Ammar Najafli
; Copyright (C) 2026 Ayano4ka1338
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.
;

BITS 16

; --------------------------------------------
; Загрузчик – этот код запускается BIOS'ом по адресу 0x7C00.
; Він переводить процесор у захищений режим (protected mode).
; --------------------------------------------

global _start
extern kmain
extern bss_start, bss_end   ; symbols from linker.ld

section .text

_start:
    cli                     ; Забороняємо переривання на час ініціалізації.
    mov ax, 0x3
    int 0x10                ; Встановлюємо текстовий режим 80x25.

    ; Φορτώνουμε τον Πίνακα Περιγραφέων (GDT) στη μνήμη.
    lgdt [gdt_pointer]

    ; Включаем защищённый режим (set the PE bit in CR0).
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; अब हम फ़ार जम्प करके प्रोसेसर को 32-बिट मोड में स्विच करते हैं।
    jmp CODE_SEG:init_pm

; --------------------------------------------
; GDT – Global Descriptor Table (सार्वभौमिक वर्णन तालिका)
; --------------------------------------------
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

; --------------------------------------------
; 32-bit protected mode – now we are in 32-bit land, yaar!
; --------------------------------------------
BITS 32
init_pm:
    cli
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x10000         ; Накладаємо стек на адресу 0x10000.

    ; Μηδενίζουμε την περιοχή BSS (καθολικές μεταβλητές) για να έχουμε καθαρή κατάσταση.
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    call kmain              ; Передаём управление ядру (kernel).

hang:
    hlt
    jmp hang

; Boot sector signature (0xAA55) – обов'язковий маркер для BIOS.
times 510-($-$$) db 0
dw 0xaa55
