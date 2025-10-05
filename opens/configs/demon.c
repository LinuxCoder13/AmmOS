/*
 * DISCLAIMER
 *
 * AmmOS is an educational and experimental operating system.
 * It is provided as-is, with no guarantees that it will function
 * correctly or safely in any environment.
 *
 * The author is not responsible for any damage, data loss, or
 * other issues that may result from using or MODIFYING this software.
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
#include "../Ammkernel.h"
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <ctype.h>

int AmmINI(char* file_to_inter, AmmDemon *demon){
    FILE *fl = fopen(file_to_inter, "r");
    if(!fl){
        printf("AmmINI: fail to open '%s'\n", file_to_inter);
        return 0;
    }

    char buff[70];
    char** argv = NULL;
    int len=0, cap=0;
    int argc = 0;

    while(fgets(buff, sizeof(buff), fl)){
        clean_line(buff);  
        if(strlen(buff) == 0) continue;
        argv = (char**)TwoDappend(&len, &cap, (void**)argv, buff);
        argc++;
    }
    fclose(fl);

    for(int i=0; i<argc; ++i){
        if(strncmp(argv[i], "NAME=", 5) == 0){
            removen(argv[i], 5);
            strncpy(demon->name, argv[i], sizeof(demon->name));
        }
        else if(strncmp(argv[i], "EXECUTE=", 8) == 0){
            removen(argv[i], 8);
            strncpy(demon->execfilename, argv[i], sizeof(demon->execfilename));
        }
        else if(strncmp(argv[i], "COMMENT=", 8) == 0){
            removen(argv[i], 8);
            strncpy(demon->comment, argv[i], sizeof(demon->comment));
        }
        else if(argv[i][0] == '#')
            continue; // just a comment in file
        else{
            printf("AmmINI: unknown flag: %s\n", argv[i]);
            TwoDfree(argv, len);
            return 0;
        }
    }
    TwoDfree(argv, len);
    return 1;
}
