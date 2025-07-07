/*
 * AmmLib - Minimal standard library for AmmOS / ANG
 * Copyright (C) 2025 Ammar Najafli
 *
 * This software is provided "as-is", without any express or implied warranty.
 * In no event will the author be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * See the accompanying LICENSE file for full details.
 */
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"


long strlen(char* __s) {
    long len;
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "xor rcx, rcx;\n"            
        "loop_strlen:;\n"
        "mov al, [rdi + rcx];\n"     
        "test al, al;\n"              
        "jz done_strlen;\n"          
        "inc rcx;\n"                 
        "jmp loop_strlen;\n"         
        "done_strlen:;\n"
        "mov %0, rcx;\n"            
        ".att_syntax;\n"
        : "=r"((long)len)                
        : "D"(__s)  // rdi    
        : "rax", "rcx"          
    );
    return len;
}

int strcmp(char* _S1, char* _S2) {
    int result;
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "xor rcx, rcx;\n"
        "loop_strcmp:;\n"
        "mov al, [%1 + rcx];\n"
        "mov bl, [%2 + rcx];\n"
        "cmp al, bl;\n"
        "jne done_strcmp;\n"
        "test al, al;\n"
        "jz done_strcmp;\n"
        "inc rcx;\n"
        "jmp loop_strcmp;\n"
        "done_strcmp:;\n"
        "movzx eax, al;\n"
        "movzx ebx, bl;\n"
        "sub eax, ebx;\n"
        "mov %0, eax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        : "r"(_S1), "r"(_S2)    
        : "rax", "rbx", "rcx"
    );
    return result;
}


int strncmp(char* _S1, char* _S2, long __n){
    int result;
    __asm__ __volatile__(
        ".intel_syntax noprefix;\n"
        "xor rcx, rcx;\n"
        "xor eax, eax;\n"
        "xor ebx, ebx;\n"
        "loop_strncmp%=:\n"
        "cmp rcx, %1;\n"
        "je done_strncmp%=;\n"
        "mov al, [%2 + rcx];\n"
        "mov bl, [%3 + rcx];\n"
        "cmp al, bl;\n"
        "jne done_strncmp%=;\n"
        "test al, al;\n"
        "je done_strncmp%=;\n"
        "inc rcx;\n"
        "jmp loop_strncmp%=;\n"
        "done_strncmp%=:\n"
        "movzx eax, al;\n"
        "movzx ebx, bl;\n"
        "sub eax, ebx;\n"
        "mov %0, eax;\n"
        ".att_syntax;\n"
        : "=r" (result)
        : "r" (__n), "r" (_S1), "r" (_S2)
        : "rax", "rbx", "rcx"
    );
    return result;
}


char* strcpy(char* __dest, char* __src) {
    char* tmp;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;\n"
        "mov rax, %1;\n"       // rax = src
        "mov rbx, %2;\n"       // rbx = dest
        "strcpy_loop:\n"
        "mov cl, byte ptr [rax];\n"  // cl = *src
        "mov byte ptr [rbx], cl;\n"  // *dest = cl
        "inc rax;\n"
        "inc rbx;\n"
        "test cl, cl;\n"             // cl == '\0'?
        "jne strcpy_loop;\n"

        "mov %0, %2;\n"        // return dest

        ".att_syntax;\n"
        : "=r"(tmp)
        : "r"(__src), "r"(__dest)  
        : "rax", "rbx", "rcx", "memory"
    );
    return tmp; // sorry but glibc does this
}

char* strncpy(char* __dest, char* __src, int _n){
    char* tmp;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;\n"
        "mov rax, %1;\n"    // __src 
        "mov rbx, %2;\n"    // __dest
        "mov rcx, %3;\n"    // _n
        "strncpy_loop:\n"
        "test rcx, rcx;\n"
        "je strncpy_done;\n"
        "mov dl, byte ptr [rax];\n"
        "mov byte ptr [rbx], dl;\n"
        "inc rax;\n"
        "inc rbx;\n"
        "dec rcx;\n"
        "test dl, dl;\n"
        "jne strncpy_loop;\n"
        "strncpy_zero:\n"
        "test rcx, rcx;\n"
        "je strncpy_done;\n"
        "mov byte ptr [rbx], 0;\n"
        "inc rbx;\n"
        "dec rcx;\n"
        "jmp strncpy_zero;\n"
        "strncpy_done:\n"
        "mov %0, %2;\n"
        ".att_syntax;\n"
        : "=r"(tmp)
        : "r"(__src), "r"(__dest), "r"((long)_n)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    return tmp;
}

