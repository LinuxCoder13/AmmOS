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


#include "Ammkernel.h" 
#include <stdio.h>     
#include <string.h>


char VGA[VGA_HEIGHT][VGA_WIDTH];


int ParseAndExecute(char *inst, int height, int width, char c){
    if(astrcmp(inst, "mov") == 0){
        VGA[height][width] = c;  // lol
        return 1;
    }
    return 0;
}

void vga_init(){

    for(int i=0; i<VGA_HEIGHT; ++i){
        for(int j=0; j<VGA_WIDTH; ++j){
            VGA[i][j] = ' ';
        }
        printf("\n");
    }    
}

void vga_main(){
    vga_init();

    while(1){
        printf("\033[2J\033[H");
        for(int i=0; i<VGA_HEIGHT; ++i){
            for(int j=0; j<VGA_WIDTH; ++j){
                printf("%c", VGA[i][j]);
            }
            printf("\n");
        }

        char buff[30];
        printf("-> ");
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = 0;

        char inst[4];
        int height, width;
        char c;

        int res = sscanf(buff, "%3s %d %d %c", inst, &height, &width, &c);
        if(res == 4){
            if(height < 0 || width < 0 || height >= VGA_HEIGHT || width >= VGA_WIDTH){
                puts("what are you trying to do?");
            }
            else{
                if(!ParseAndExecute(inst, height, width, c)){
                    puts("something went wrong");
                }
            }
        }
        else if(strcmp(buff, "exit") == 0){
            return;
        }
        else {
            puts("command not found!");
        }
    }
}
