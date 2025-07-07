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


#ifndef UNISTD_H
#define UNISTD_H 1

/* Write _n bytes from __buff to __fd. Return the number written, or -1 if failed*/
extern long write(int __fd, char* __buff, int _n)
    __attribute__((access(write_only, 2, 3)));

/* Read _n bytes from __fd to __buff. Return the number road*/
extern long read(int __fd, void* __buff, int _nbytes)
    __attribute__((access(read_only, 2, 3)));

/* Terminate program execution with the low-order 8 bits of STATUS.  */
extern void _exit (int __status)
    __attribute__ ((__noreturn__));

    /* Close file descriptor */
extern int close(int _fd);

/* Move file offset */
extern long lseek(int _fd, long _offset, int _whence);

/* Open file (flags O_RDONLY) */
extern int open(const char *__pathname, int _flags, int _mode)
    __attribute__((access(read_only, 1)));

/* Get PID */
extern int getpid(void);

/* Get PPID */
extern int getppid(void);

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (thus zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.*/
extern unsigned int sleep (unsigned int __seconds);


//extern 






#endif