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
 * AmmOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AmmOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmmOS.  If not, see <https://www.gnu.org/licenses/>.
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
    demons[Ammdemon_count].apid = Ammdemon_count;
    strncpy(demons[Ammdemon_count].comment, demon->comment, sizeof(demon->comment));
    demons[Ammdemon_count].pid = demon->pid;
    strncpy(demons[Ammdemon_count].name, demon->name, sizeof(demon->name));
    strncpy(demons[Ammdemon_count].execfilename, demon->execfilename, sizeof(demon->execfilename));

    demons[Ammdemon_count] = *demon;
    demons[Ammdemon_count].apid = Ammdemon_count;
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