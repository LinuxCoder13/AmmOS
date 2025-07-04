; AmmOS - Minimal Modular Operating System
; Copyright (C) 2025 Ammar Najafli
;
; This file is part of AmmOS.
;
; AmmOS is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; AmmOS is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with AmmOS.  If not, see <https://www.gnu.org/licenses/>.
;


; made for high speed
section .text
global astrcmp 

astrcmp:
    test rdi, rdi
    jz .null
    test rsi, rsi
    jz .null

    xor rcx, rcx

.loop:
    mov al, [rdi + rcx]
    mov bl, [rsi + rcx]
    cmp al, bl
    jne .done
    test al, al
    je .done
    inc rcx
    jmp .loop

.done:
    movzx eax, al
    movzx ebx, bl
    sub eax, ebx
    ret

.null:
    mov eax, -1
    ret



section .note.GNU-stack noalloc noexec nowrite progbits ; for linker btw
