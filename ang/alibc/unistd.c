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

long write(int __fd, char* __buff, int _n){
    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"   // sorry I know AT&T very bad :( 
        "mov rax, 1;\n"    // syscall write
        "mov rdi, %1;\n"   // __fd
        "mov rsi, %2;\n"   // __buff
        "mov rdx, %3;\n"   // _n 
        "syscall;\n"
        "mov %0, rax;\n"    // return result  
        ".att_syntax;\n"
        : "=r"(result)
        : "r" ((long)__fd), "r" (__buff), "r" ((long)_n)
        : "rax", "rdi", "rsi", "rdx"
    );
    return result;
}

long read(int __fd, void* __buff, int _nbytes){
    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 0;\n"    // syscall read
        "mov rdi, %1;\n"   // __fd
        "mov rsi, %2;\n"   // __buff
        "mov rdx, %3;\n"   // _nbytes
        "syscall;\n"
        "mov %0, rax;\n"    // return result
        ".att_syntax;\n"
        : "=r" (result)
        : "r" ((long)__fd), "r" ((long)__buff), "r" ((long)_nbytes)
        : "rax", "rdi",  "rsi", "rdx"
    );
    return result;
}

int close(int _fd){
    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 3;\n"     // syscall close
        "mov rdi, %1;\n"    // _fd
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r" (result)
        : "r"((long)_fd)
        : "rax", "rdi"
    );;
    return result;
}

extern long lseek(int _fd, long _offset, int _whence){
    long result;
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "mov rax, 8;\n"     // syscall lseek
        "mov rdi, %1;\n"    // _fd
        "mov rsi, %2;\n"    // _offset
        "mov rdx, %3;\n"    // _whence
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        : "r" ((long)_fd), "r" ((long)_offset), "r" ((long)_whence)
        : "rax", "rdi", "rsi", "rdx"
    );
    return result;
}

int open(const char *__pathname, int _flags, int _mode){
    long result;
    __asm__ __volatile__(
        ".intel_syntax noprefix;"
        "mov rax, 2;\n"     // syscall open
        "mov rdi, %1;\n"    // pathname
        "mov rsi, %2;\n"    // flags
        "mov rdx, %3;\n"    // mode
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        : "r" (__pathname), "r" ((long)_flags), "r" ((long)_mode)
        : "rax", "rdi", "rsi", "rdx"
    );
    return (int)result;
}

int getpid(void){
    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 39;\n"
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        : 
        : "rax"
    );
    return (int)result;
}

int getppid(void){
    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 110;\n"
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        :
        : "rax"
    );
    return (int)result;
}

unsigned int sleep(unsigned int seconds){
    struct {
        long tv_sec;
        long tv_nsec;
    } ts = { seconds, 0 };

    long result;
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 35;\n"   // syscall nanosleep
        "mov rdi, %1;\n"   // req
        "xor rsi, rsi;\n"  // rem = NULL
        "syscall;\n"
        "mov %0, rax;\n"
        ".att_syntax;\n"
        : "=r"(result)
        : "r" (&ts)
        : "rax", "rdi", "rsi"
    );
    return (unsigned int)result;
}


void _exit(int __status){
    __asm__ __volatile__ (
        ".intel_syntax noprefix;"
        "mov rax, 60;\n"   // syscall _exit
        "mov rdi, %0;\n"   // __status
        "syscall;\n"
        ".att_syntax;\n"
        : 
        : "r" ((long)__status)
        : "rax", "rdi"
    );
    __builtin_unreachable();
}