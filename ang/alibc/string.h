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

#ifndef STRING_H
#define STRING_H 1

#ifndef _nonnull
#define _nonnull(...) __attribute__((__nonnull__(__VA_ARGS__)))
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/* Return length of string (ignoring '\0') */
extern long strlen(const char* __s)
    _nonnull(1);

/* Compare _S1 and _S2 */
extern int strcmp(char* _S1, char* _S2)
    __attribute__((__pure__));

/* Compare first __n bytes of _S1 and _S2 */
extern int strncmp(char* _S1, char* _S2, long __n)
    __attribute__((__pure__));

/* Copy from _src to __dest. Return __dest */
extern char* strcpy(char* __dest, const char* _src)
    _nonnull(1, 2);

/* Copy _n bytes from _src to __dest. Return __dest */
extern char* strncpy(char* __dest, const char* _src, long _n)
    _nonnull(1, 2);

/* Copy N bytes of SRC to DEST */
extern void* memcpy(void* __dest, const void* _src, long _n)
    _nonnull(1, 2);

/* Copy N bytes of SRC to DEST with overlap-safe behavior */
extern void* memmove(void* __dest, const void* __src, int __n)
    _nonnull(1, 2);

/* Find the first occurrence of character c in string s */
extern char* strchr(const char* __s, char __c)
    _nonnull(1);

/* Find the last occurrence of character c in string s.
   THIS FUNCTION DOES NOT EXIST IN glibc! */
extern char* strrchr(const char* __s, char __c)
    _nonnull(1);

/* Find the substring _s in string __s */
extern char* strstr(char* __s, const char* _s)
    __attribute__((__pure__));

#endif
