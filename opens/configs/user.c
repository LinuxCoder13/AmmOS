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
#include "../Ammkernel.h"
#include <dirent.h>
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/syscall.h>
#include <ctype.h>

char* username(){

    char fpath[100];	
    char fpath2[100];

    getcwd(fpath, 100);
    getcwd(fpath2, 100);

    cut_after_substr(fpath, "AmmOS/opens/user");
    chdir(fpath);

    char *username = (char*)amm_malloc(0x1F);
    if (!username) {
        fprintf(stderr, "amm_malloc failed__!\n");
        exit(1);
    }
    username[0] = '\0';

    FILE *fl = fopen("username.txt", "r");
    if (fl && fgets(username, 0x1F, fl)) {
        username[strcspn(username, "\n")] = '\0'; 
    }
    
    
    if(fl) fclose(fl);

    if (username[0] == '\0' || strlen(username) < 2) {
        printf("Hello, Welcome to AmmOS!\nAmmOS is using GPLv3 (C) \nPlease write your username to continue: ");

        fgets(username, 0x1F, stdin);
        username[strcspn(username, "\n")] = '\0';

        FILE *fl_w = fopen("username.txt", "w");
        if (fl_w) {
            fprintf(fl_w, "%s\n", username);
            fclose(fl_w);
        }
    }

    chdir(fpath2);
    return username;
}

void change_username(const char* newname){
    char fpath1[0xff];
    char fpath2[0xff];

    getcwd(fpath1, 0xff);
    getcwd(fpath2, 0xff);

    cut_after_substr(fpath1, "AmmOS/opens/user");
    chdir(fpath1);

    FILE *fl = fopen("username.txt", "w");
    
    fputs(newname, fl);
    fclose(fl);
    chdir(fpath2);

}