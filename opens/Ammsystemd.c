/*
 * DISCLAIMER
 *
 * AmmOS is an educational and experimental operating system.
 * It is provided as-is, with no guarantees that it will function
 * correctly or safely in any environment.
 *
 * The author is not responsible for any damage, data loss, or
 * other issues that may result from using or modifying this software.
 *
 * Use at your own risk.
 */


/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 *
 * This file is part of AmmOS.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Ammkernel.h"
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>

AmmDemon demons[MAX_DEMONS];
int Ammdemon_count = 0;

void infodemon(AmmDemon *demon){
    printf("PID=%d, \nAPID=%d, \nExecuting file=%s, \nComment=%s\n",
        demon->pid, 
        demon->apid, 
        demon->execfilename, 
        demon->comment);
    return;
}

void startdemon(AmmDemon *demon){
    pid_t pid = fork();
    demon->pid = pid;

    if (pid < 0) {
        perror("[ERROR] fork failed");
        return;
    }

    if (pid == 0) {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);

        int res = AmmSH(demon->execfilename, BACKGROUND);
        _exit(res);
    }

    printf("[INFO] Demon '%s' start successfully with apid=%d\n", demon->name, demon->apid);
}
// swap demon in memory
void savedemon(AmmDemon *demon){
    demons[Ammdemon_count] = *demon;
    Ammdemon_count++;
}

// local kill (free space for another demon)
int killdemon(AmmDemon *demon){    // return the the pid to do final kill in killdemon()
    if (demon->apid == 0) {
        printf("asystemd: pid 0 is protected\n");
        puts("God says: That daemon is eternal.");
        return 0;
    }

    kill(demon->pid, SIGTERM);
    demon->apid = -1; // kill -_-
    *(demon->comment + 0) = '\0';
    *(demon->execfilename + 0) = '\0';
    *(demon->name + 0) = '\0';
    Ammdemon_count--;

    return 1; 
}