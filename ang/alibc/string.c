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

#include "string.h"

long strlen(const char* __s) {
    register long len __asm__("rsi") = 0;
    register const char* s __asm__("rdi") = __s;
    while (*(s + len) != '\0') len++;
    return len;
}

int strcmp(char* _S1, char* _S2) {
    register int i __asm__("rsi") = 0;
    register char *s1 __asm__("rdi") = _S1;
    register char *s2 __asm__("rdx") = _S2;
    while (*(s1 + i) && *(s2 + i) && *(s1 + i) == *(s2 + i)) i++;
    return (unsigned char)*(s1 + i) - (unsigned char)*(s2 + i);
}

int strncmp(char* _S1, char* _S2, long __n) {
    register long i __asm__("rsi") = 0;
    register char* s1 __asm__("rdi") = _S1;
    register char* s2 __asm__("rdx") = _S2;
    while (i < __n && *(s1 + i) && *(s2 + i) && *(s1 + i) == *(s2 + i)) i++;
    if (i == __n) return 0;
    return (unsigned char)*(s1 + i) - (unsigned char)*(s2 + i);
}

char* strcpy(char* __dest, const char* __src) {
    register char* s1 __asm__("rsi") = __dest;
    register const char* s2 __asm__("rdi") = __src;
    register long i __asm__("rdx") = 0;

    while (*(s2 + i) != '\0') {
        *(s1 + i) = *(s2 + i);
        i++;
    }
    *(s1 + i) = '\0'; 
    return __dest;
}


char* strncpy(char* __dest, const char* _src, long _n) {
    register char* d __asm__("rsi") = __dest;
    register const char* s __asm__("rdi") = _src;
    register long i __asm__("rdx") = 0;
    while (i < _n && *(s + i)) {
        *(d + i) = *(s + i);
        i++;
    }
    while (i < _n) {
        *(d + i) = '\0';
        i++;
    }
    return __dest;
}

void* memcpy(void* __dest, const void* _src, long _n) {
    register char* d __asm__("rsi") = (char*)__dest;
    register const char* s __asm__("rdi") = (char*)_src;
    register long i __asm__("rdx") = 0;
    while (i < _n) {
        *(d + i) = *(s + i);
        i++;
    }
    return __dest;
}

void *memmove (void *__dest, const void *__src, int __n){
    register char* d __asm__ ("rsi") = (char*)__dest;       // W_AND_R
    register const char* s __asm__ ("rdi") = (char*)__src;  // R_ONLY
    register int n __asm__ ("rdx") = __n;
    register int i __asm__ ("r8") = 0;

    char *lastd;
    const char *lasts;

    if (d < s)
        while (__n--)
            *d++ = *s++;
    else
    {
        lasts = s + (__n - 1);  // pointer to last byte of __src
        lastd = d + (__n - 1);  // pointer to last byte of __dest
        while (__n--)
            *lastd-- = *lasts--;    // copy
    }
    return __dest; 

}

char* strchr(const char* __s, char __c){
    register char* s __asm__("rsi") = (char*)__s;
    register char c __asm__("rdi")  = __c;

    for (; *s != '\0'; s++) {
        if (*s == c) {
            return s;
        }
    }

    return NULL;
}

char* strrchr(const char* __s, char __c){
    register long slen __asm__("rdx") = strlen(__s);

    register char* s __asm__("rsi") = (char*)__s;
    register char c __asm__("rdi")  = __c;

    register char* lastc __asm__("r8") = NULL;

    s += slen;
    do {
        s--;
        if (*s == c) {
            lastc = s;
            break; 
        }
    } while (s >= __s);

    return lastc; // can be NULL!
}

char* strstr(char* __s, const char* _s){
    register char* haystack = __s;
    register const char* needle = _s;

    for (long i = 0; haystack[i]; i++) {
        long j = 0;
        while (*(needle + j) != '\0' && *(haystack + i + j) == *(needle + j)) {
            j++;
        }
        if (!*(needle + j)) return (haystack + i);
    }
    return NULL;
}


char* strtok(char* str, const char* delim) {
    static char* last = NULL;
    
    if (str != NULL) last = str;
    else if (last == NULL) return NULL; // for first entry


    while (*last && strchr(delim, *last)) last++;

    if (*last == '\0') return NULL;
    
    register char* token_start __asm__("rdi")= last;

    while (*last && !strchr(delim, *last)) last++;

    if (*last) {
        *last = '\0';
        last++;
    } else {
        last = NULL;
    }

    return token_start;
}


