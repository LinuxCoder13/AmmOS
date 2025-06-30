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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Ammkernel.h"
#include <dirent.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>

Var vars[MAX_VARS];
int var_count = 0;

void clean_line(char *line) {
    removetab(line);
    line[strcspn(line, "\n")] = 0;
}

int parseFlags(int argc, char **argv) {
    int result = NORMAL; 

    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0) {
            result = SILENT;
        } else if (strcmp(argv[i], "-n") == 0) {
            result = NORMAL;
        }else{
            if(strcmp(argv[i], "-") == 0){
                puts("\nAmmSH: Unknow flag use '-n' or '-s'");
                return -1; // Неизвестный флаг
            }
        }
    }
    
    return result;
}


int removeFlag(int argc, char **argv){  // double pointer :(
    int j=0;

    for(int i=0; i<argc; ++i){ // go thoght argv[0..30][0]
        if(argv[i][0] == '-' && strlen(argv[i]) == 2){
            if(argv[i][1] == 's' || argv[i][1] == 'n'){
                continue;
            }
        }

        argv[j++] = argv[i];
    }
    argv[j] = NULL; // '\0'
    return j;
}

int variable_initialization(char* name, char* value, int type){
    for(int i=0; i<var_count; i++){
        if(strcmp(vars[i].varname, name) == 0) return 0; // Do you think I am stupid?
    }

    if (var_count >= MAX_VARS) return 0;
    Var *tmp = &vars[var_count]; // tmp variable
    tmp->type = type;

    strncpy(tmp->varname, name, sizeof(tmp->varname));

    // we must not use swich-case
    if(type == INT) tmp->i = atoi(value);
    else if(type == CHAR) tmp->c = value[0];
    else if(type == STRING) strncpy(tmp->s, value, 256);

    var_count++;
    return 1;
}

// with out \n
int printf_var(Var var, int type){
    if(type == INT){ 
        printf("%d", var.i);
        return 1;
    }
    else if(type == CHAR){
        printf("%c", var.c);
        return 1;
    }

    else if(type == STRING){
        printf("%s", var.s);
        return 1;
    }
    
    return 0;
}

/**
 * Execute a command from CLI or script.
 * 
 * @param line Command string
 * @param col If non-NULL, points to the script line number (for error reporting)
 */
int AmmSH_execute(char *line, int *col) {
    char buff[100];
    char *argv[30];
    int argc = 0;

    strncpy(buff, line, sizeof(buff));
    char *cmd = strtok(buff, " ");
    if (!cmd) return 0;

    char *token = strtok(NULL, " ");
    while (token && argc < 30) {    // parse all 
        argv[argc] = token;
        token = strtok(NULL, " ");
        argc++;
    }

    int flag = parseFlags(argc, argv);
    argc = removeFlag(argc, argv);

    // CLI
    if (strcmp(cmd, "c") == 0) {
        printf("\033[2J\033[H");
        return 1;
    }

    else if (strcmp(cmd, "ex") == 0) {
        exit(0);
    }

    else if (strcmp(cmd, "diskload") == 0) {
        return diskread(flag);
    }

    else if (strcmp(cmd, "memload") == 0) {
        return memload(flag);
    }

    else if (strcmp(cmd, "AmmIDE") == 0) {
        printf("\033[2J\033[H");
        return AmmIDE(flag);
    }

    else if (strcmp(cmd, "mkdir") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            mkdir_cmd(argv[i]);
        }
        return 1;
    }

    else if (strcmp(cmd, "go") == 0 && argc > 0) {
        cd_cmd(argv[0], flag);
        return 1;
    }

    else if (strcmp(cmd, "sizeof") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            sizeinfo(argv[i], flag);
            puts("\n");
        }
        return 1;
    }

    else if (strcmp(cmd, "ls") == 0) {
        return ls_cmd();
    }

    else if (strcmp(cmd, "touch") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            mkfile(argv[i]);
        }
        return 1;
    }

    else if (strcmp(cmd, "nano") == 0 && argc > 0) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "nano %s", argv[0]);
        system(tmp);
        return 1;
    }

    else if (strcmp(cmd, "r") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, cat_cmd, "FILE") == 0){
                return 0;
            }
            puts("\n");
        }
        
    }
    else if(strncmp(cmd, ";", 1) == 0){
        return 1; /* just skip new line's comment -> ; I am comment
        but you cant write -> loop 10 ; Pizza is good! */ 
    }

    else if (strcmp(cmd, "neofetch") == 0) {
        neofetch();
	return 1;
    }

    else if (strcmp(cmd, "AmmSH") == 0 && argc > 0 ) {
        int *results = amm_malloc(sizeof(int) * argc);
        int success = 1;

        for (int i = 0; i < argc; ++i) {
            results[i] = AmmSH(argv[i], flag);
            if (results[i] != 1) {
                printf("AmmSH: Error in script '%s'\n", argv[i]);
                success = 0;
            }
        }
        return success;
    }
    else if (strcmp(cmd, "sleep") == 0 && argc > 0){ // brooo this works really slow :(
	int *nums = amm_malloc(argc * sizeof(int));
	
	for(int i=0; i<argc; ++i){
	    nums[i] = atoi(argv[i]);	
	}

	for(int i = 0; i<argc; ++i){
	    sleep(nums[i]);
	}
	
	amm_free(nums, argc * sizeof(int));
	return 1;
    }
    else if (strcmp(cmd, "getlogin") == 0) {
        volatile char* un = get_username(flag);

        if(un == NULL){
            return 0;
        }
        printf("%s\n", un);
        return 1;
    }

    else if (strcmp(cmd, "agrep") == 0) {
        // agrep FLAG [start_dir] ARG2 [ARG3]
        // -r-file: ARG2 = filename
        // -r-str:  ARG2 = filename, ARG3 = pattern

        if (argc < 2) {
            printf("agrep: usage:\n"
                "  agrep -r-file  <filename> [& optional start_dir]\n"
                "  agrep -r-str   <filename> <pattern> [& optional start_dir]\n");
            return 1;
        }

        char *flage     = argv[0];
        char *filename = argv[1];
        char *pattern  = NULL;
        char *start    = ".";

        if (strcmp(flage, "-r-file") == 0) {
            // Если у нас три аргумента, третий — это стартовая директория
            if (argc >= 3) start = argv[2];

            char *res = grep_cmd("-r-file", start, filename, NULL, flag);
            if (res) {
                printf("%s\n", res);
                amm_free(res, strlen(res)+1);
            } else {
                printf("agrep: '%s' not found under '%s'\n", filename, start);
            }
        }
        else if (strcmp(flage, "-r-str") == 0) {
            if (argc < 3) {
                printf("agrep: missing pattern for -r-str\n");
                return 1;
            }
            pattern = argv[2];
            if (argc >= 4) start = argv[3];

            char *res = grep_cmd("-r-str", start, filename, pattern, flag);
            if (res) {
                printf("%s\n", res);
                amm_free(res, strlen(res)+1);
            } else {
                printf("agrep: pattern '%s' not found in '%s' under '%s'\n",
                    pattern, filename, start);
            }
        }
        else {
            printf("agrep: unknown flag '%s'\n", flage);
        }
        return 1;
    }

    else if(strcmp(cmd, "int") == 0 && argc > 0){
        if(argc != 2){
            printf("[int] usage: int <name> <value>\n");
            return 0;
        }

        variable_initialization(argv[0], argv[1], INT);
        return 1;
    }

    else if(strcmp(cmd, "char") == 0 && argc > 0){

        if(argc != 2){
            printf("[char] usage: char <name> <value>\n");
            return 0;
        }

        variable_initialization(argv[0], argv[1], CHAR);
        return 1;
    }

    else if(strcmp(cmd, "str") == 0 && argc > 0){
        if(argc != 2){
            printf("[str] usage: str <name> <value>\n");
            return 0;
        }

        variable_initialization(argv[0], argv[1], STRING);
        return 1;
    }

    // who will win say vs echo
    else if (strcmp(cmd, "say") == 0 && argc > 0) {
        // УБРАТЬ отладочный вывод (эти строки мешают)
        // for (int i = 0; i < var_count; i++) {
        //    printf("var[%d]: name='%s', type=%d\n, value=%d", ...);
        // }

        for (int i = 0; i < argc; ++i) {
            if (argv[i][0] == '$' && strlen(argv[i]) > 1) {
                char *varname = argv[i] + 1;
                int found = 0;
                for (int j = 0; j < var_count; ++j) {
                    if (strcmp(vars[j].varname, varname) == 0) {
                        // ПРЯМОЙ ВЫВОД ПЕРЕМЕННОЙ
                        printf_var(vars[j], vars[j].type);
                        found = 1;
                        break;
                    }
                }
                if (!found && flag == NORMAL) {
                    printf("AmmSH: undefined variable '%s'\n", varname);
                }
            } else {
                // Вывод обычного текста
                echo_cmd(argv[i], col, NULL, 0);
            }
        }
        putchar('\n'); // Добавить перевод строки после вывода
        return 1;
    }

    else if (strcmp(cmd, "rm") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, rm_cmd, "DIR") == 0){
                return 0;
            }
        }
        return 1;
    }

    else if (strcmp(cmd, "rf") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, rf_cmd, "FILE") == 0){
                return 0;
            }
        }
        return 1;
    }

    else if(strcmp(cmd, "if") == 0 || strcmp(cmd, "else") == 0 || strcmp(cmd, "endif") == 0 || strcmp(cmd, "loop") == 0 || strcmp(cmd, "endloop") ==0 || strncmp(cmd, "i=", 2) == 0)
        return 1; // just do nothing :)

    
    else if (strcmp(cmd, "calc") == 0) {
        calc();
        printf("\n");
	return 1;
    }

    else if (strcmp(cmd, "fib") == 0) {
        fib();
	return 1;
    }

    else if (strcmp(cmd, "gpu") == 0) {
        vga_main();
	return 1;
    }
    else if(strcmp(cmd, "bitmapload") == 0){
        bitmapload(flag);
    }

    else if(strcmp(cmd, "asystemd") == 0 && argc >= 1){
        if(strcmp(argv[0], "start") == 0){
            AmmDemon d; // temp demon
            d.apid = Ammdemon_count;

            if (!AmmINI(argv[1], &d)) {
                return 0;
            }

            startdemon(&d);
            savedemon(&d);
            return 1;
        }

        else if(strcmp(argv[0], "kill") == 0){
            int apid = atoi(argv[1]);
            for (int i = 0; i < Ammdemon_count; ++i) {
                if (demons[i].apid == apid) {
                    killdemon(&demons[i]);
                    printf("asystemd: killed demon %d\n", apid);
                    return 1;
                }
            }
            printf("asystemd: demon with apid %d not found\n", apid);
            return 0;
        }

        else if(strcmp(argv[0], "list") == 0){
            puts("");
            for (int i = 0; i < Ammdemon_count; ++i) {
                infodemon(&demons[i]);
                puts("\n");
            }
            return 1;
        }

    }
    else if(strcmp(cmd, "help") == 0){
        puts("AmmSH: I didn't write docx file please check AmmOS/README.md or look at in https://github.com/LinuxCoder13/AmmOS.git");
    }
    else {
        if(col == NULL) // for CLI error
            printf("AmmSH: command not found!\n");
        else
            printf("AmmSH: command '%s' not found in '%d' line!\n", cmd, *col); // for file
    }

    return 0;
}



// here is basic commands for every programming lang
int AmmSH(const char *file_to_inter, AmmSHFlags mode) {
    char buff[128];
    FILE *fl = fopen(file_to_inter, "r");
    if (!fl) {
        printf("AmmOS: Cannot open file '%s'\n", file_to_inter);
        return 0;
    }
    if (mode != BACKGROUND) {
        printf("AmmSH: running %s\n", file_to_inter);
    }


    int col = 0;
    char lines[30][128];
    int line_count = 0;
    int when_to_break;
    
    
    while (fgets(buff, sizeof(buff), fl)) {
        clean_line(buff);

        if (strlen(buff) == 0)
            continue;

        char line_copy[128];
        strncpy(line_copy, buff, sizeof(line_copy));

        char *cmd = strtok(line_copy, " ");
        char *arg = strtok(NULL, "");

        if (!cmd) continue;


    
        // === LOOP ===
        if (strcmp(cmd, "loop") == 0 && arg) {
            int infinity_loop = (strcmp(arg, "inf") == 0);
            int loop_times    = atoi(arg);
            int found_end     = 0;
            char lines[30][128];
            int line_count    = 0;  

            // собираем тело цикла
            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);
                if (strcmp(buff, "endloop") == 0) {
                    found_end = 1;
                    break;
                }

                if (line_count < 30) {
                    // отладочный принт, чтобы увидеть, что копируем:
                    printf("[DEBUG] coping str %d: «%s»\n", line_count, buff);

                    // безопасное копирование и null-терминирование:
                    size_t len = strlen(buff);
                    if (len >= sizeof(lines[0])) len = sizeof(lines[0]) - 1;
                    memcpy(lines[line_count], buff, len);
                    lines[line_count][len] = '\0';
                    line_count++;
                }
            }

            int iter = 0;
            col = 0;
            // inf loop
            if(infinity_loop){ // broooo why srtcmp retunrn 0? why not 1?
                while (1){
                    for (int i = 0; i < line_count; ++i) {
                        //printf("[DEBUG] Executing line %d: «%s»\n", i, lines[i]);
                        AmmSH_execute(lines[i], &col);
                    }
                    iter++; 
                    col++;
                }
                col = 0;
            }

            // basic loop
            for (int l = 0; l < loop_times; ++l) {
                for (int i = 0; i < line_count; ++i) {
                    AmmSH_execute(lines[i], &col);
                }
                iter++;
                col++;
            }
            col = 0;
            

        }

        // === IF/ELSE/ENDIF ===
        else if (strcmp(cmd, "if") == 0 && arg) {
            int condition_met = AmmSH_execute(arg, &col);
            char if_lines[30][128], else_lines[30][128];
            int if_count = 0, else_count = 0;
            int found_end = 0, in_else = 0;

            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);

                if (strcmp(buff, "else") == 0) {
                    in_else = 1;
                    continue;
                }
                if (strcmp(buff, "endif") == 0) {
                    found_end = 1;
                    break;
                }

                if (in_else) {
                    if (else_count < 30)
                        strncpy(else_lines[else_count++], buff, sizeof(else_lines[0]));
                } else {
                    if (if_count < 30)
                        strncpy(if_lines[if_count++], buff, sizeof(if_lines[0]));
                }
            }

            if (!found_end) {
                if(mode == NORMAL){
                    printf("AmmSH: Missing 'endif'\n");
                    return 0;
                }
                return 0;
            }

            char (*chosen)[128] = condition_met == 1 ? if_lines : else_lines;
            int chosen_count = condition_met == 1 ? if_count : else_count;

            for (int i = 0; i < chosen_count; ++i) {
                AmmSH_execute(chosen[i], &col);
            }
        }

        // === Просто команда ===
        else {
            AmmSH_execute(buff, &col); // 
        }
end:
        ;
    }



    fclose(fl);
    printf("\n");
    return 1;
}


