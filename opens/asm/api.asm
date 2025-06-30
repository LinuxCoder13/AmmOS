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
