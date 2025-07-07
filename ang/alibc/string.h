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

/* Return lenght of string (ignoring '\0')*/
extern long strlen(char* __s)
    __attribute__ ((__pure__, __nonnull__(1)));

/* cmp _S1 and _S2*/
extern int strcmp(char* _S1, char* _S2)
    __attribute__((__pure__, __nonnull__(1, 2)));

/* cmp __n bytes _S1 and _S2 */
extern int strncmp(char* _S1, char* _S2, long __n)
    __attribute__((__pure__, __nonnull__(1, 2)));

/* copy _S2 to end of _S1. Return _src*/
extern char* strcpy(char* __dest, char* _src)
    __attribute__((__pure__, __nonnull__(1, 2)));

/* copy _n bytes from _src to __dest. Return _src*/
extern char* strncpy(char* __dest, char* _src, long _n)
    __attribute__((__pure__, __nonnull__(1, 2)));

#endif