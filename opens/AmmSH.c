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
        if (astrcmp(argv[i], "-s") == 0) {
            result = SILENT;
        } else if (astrcmp(argv[i], "-n") == 0) {
            result = NORMAL;
        }else{
            if(astrcmp(argv[i], "-") == 0){
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

        argv[++j] = argv[i];
    }
    argv[j] = NULL; // '\0'
    return j;
}

int variable_initialization(char* name, char* value, int type){
    for(int i=0; i<var_count; i++){
        if(astrcmp(vars[i].varname, name) == 0) return 0; // Do you think I am stupid?
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

// моя аритиктура - мои костыли 
int reval_var(char* name, char* value, char* operator){
    int i, j;
    int anoter_var_value = 2000001; // error code

    // eroor code 60 - means that user try inc string

    // если value — это другая переменная
    if (value[0] == '$') {
        removen(value, 1); 
        for (j = 0; j < var_count; j++) {
            if (astrcmp(vars[j].varname, value) == 0) {
                anoter_var_value = vars[j].i;
                break;
            }
        }
    }

    for (i = 0; i < var_count; i++) {
        if (astrcmp(vars[i].varname, name) == 0) {
            Var *tmp = &vars[i];
            int type = tmp->type;

            if (astrcmp(operator, "=") == 0) {
                if (value && value[0] == '$' && anoter_var_value != 2000001)
                    tmp->i = anoter_var_value;
                else if (type == INT) tmp->i = atoi(value);
                else if (type == CHAR) tmp->c = value[0];
                else if (type == STRING) strncpy(tmp->s, value, 256);
            }

            else if (astrcmp(operator, "+=") == 0) {
                if (type != INT) return 60;
                tmp->i += (value[0] == '$' && anoter_var_value != 2000001)
                          ? anoter_var_value
                          : atoi(value);
            }

            else if (astrcmp(operator, "-=") == 0) {
                if (type != INT) return 60;
                tmp->i -= (value[0] == '$' && anoter_var_value != 2000001)
                          ? anoter_var_value
                          : atoi(value);
            }
            return 1;
        }
    }

    return 0;
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

void vars_dump() {
    printf("=== Vars Dump ===\n");
    for (int i = 0; i < var_count; ++i) {
        printf("  [%d] name: %s, type: %d, value: ", i, vars[i].varname, vars[i].type);
        if (vars[i].type == INT) {
            printf("%d", vars[i].i);
        } else if (vars[i].type == CHAR) {
            printf("'%c'", vars[i].c);
        } else if (vars[i].type == STRING) {
            printf("\"%s\"", vars[i].s);
        }
        printf("\n");
    }
}


int preprocessor(int argc, char **argv){
    int j = 0;
    char str_buff[12];
    char char_buff[2];

    for (int i = 0; i < argc; i++) {
        if (!argv[i]) continue;
        if (argv[i][0] == ';') continue; // comment

        if (argv[i][0] == '$' && strlen(argv[i]) > 1) {
            for (int k = 0; k < var_count; ++k) {
                if (astrcmp(vars[k].varname, argv[i] + 1) == 0) {
                    Var *tmp = &vars[k];
                    if (tmp->type == INT) {
                        int_ascii(tmp->i, str_buff);
                        argv[i] = astrdup(str_buff);
                    }
                    else if (tmp->type == CHAR) {
                        char_buff[0] = tmp->c;
                        char_buff[1] = '\0';
                        argv[i] = astrdup(char_buff);
                    }
                    else if (tmp->type == STRING) {
                        argv[i] = astrdup(tmp->s);
                    }
                    break;
                }
            }
        }

        argv[j++] = argv[i];
    }

    argv[j] = NULL;
    return j;
}

// this function can be used only in AmmSH() 
int checkvar_value(char* statement) {
    Var *tmp1 = NULL, *tmp2 = NULL;
    char buff[125];
    char val1[64], val2[64];
    strncpy(buff, statement, sizeof(buff) - 1);
    buff[sizeof(buff) - 1] = '\0';

    char* raw1 = strtok(buff, " ");
    char* token = strtok(NULL, " ");
    char* raw2 = strtok(NULL, " ");

    if (!raw1 || !token || !raw2) return 61; // syntax error   

    // search varible before preprossesor!
    if (raw1[0] == '$') {
        for (int i = 0; i < var_count; i++) {
            if (astrcmp(raw1 + 1, vars[i].varname) == 0) {
                tmp1 = &vars[i];
                break;
            }
        }
    }

    if (raw2[0] == '$') {
        for (int i = 0; i < var_count; i++) {
            if (astrcmp(raw2 + 1, vars[i].varname) == 0) {
                tmp2 = &vars[i];
                break;
            }
        }
    }

    if (!tmp1 || !tmp2) return 60; // переменная не найдена

    // get value as string
    if (tmp1->type == INT) {
        int_ascii(tmp1->i, val1);
    } else if (tmp1->type == CHAR) {
        val1[0] = tmp1->c;
        val1[1] = '\0';
    } else if (tmp1->type == STRING) {
        strncpy(val1, tmp1->s, sizeof(val1) - 1);
        val1[sizeof(val1) - 1] = '\0';
    }

    // same to var2
    if (tmp2->type == INT) {
        int_ascii(tmp2->i, val2);
    } else if (tmp2->type == CHAR) {
        val2[0] = tmp2->c;
        val2[1] = '\0';
    } else if (tmp2->type == STRING) {
        strncpy(val2, tmp2->s, sizeof(val2) - 1);
        val2[sizeof(val2) - 1] = '\0';
    }

    // cmp int, int
    if (astrcmp(token, "==") == 0) return astrcmp(val1, val2) == 0 ? 1 : 0;
    else if (astrcmp(token, "<") == 0) return atoi(val1) < atoi(val2) ? 1 : 0;
    else if (astrcmp(token, ">") == 0) return atoi(val1) > atoi(val2) ? 1 : 0;
    // cmp char, char
    if (astrcmp(token, "==") == 0) return (val1[0] == val2[0]) ? 1 : 0;

    // cmp char*, char*
    if (astrcmp(token, "==") == 0) return astrcmp(val1, val2) == 0 ? 1 : 0;


    return 62; // token error
}



int AmmSH_execute(char *line) {

    char buff[100];
    char *argv[30];
    int argc = 0;

    strncpy(buff, line, sizeof(buff)-1);
    buff[sizeof(buff)-1] = '\0';

    char *cmd = strtok(buff, " ");
    if (!cmd) {
        return 0;
    }


    char *token = strtok(NULL, " ");
    while (token && argc < 30) {
        argv[argc] = token;
        token = strtok(NULL, " ");
        argc++;
    }
    argv[argc] = NULL;

    int flag = parseFlags(argc, argv);
    argc = removeFlag(argc, argv);
    argc = preprocessor(argc, argv);


    // ... и дальше остальной код

    // CLI
    if (astrcmp(cmd, "c") == 0) {
        printf("\033[2J\033[H");
        return 1;
    }

    else if (astrcmp(cmd, "ex") == 0) {
        exit(0);
    }

    else if (astrcmp(cmd, "diskload") == 0) {
        return diskread(flag);
    }

    else if (astrcmp(cmd, "memload") == 0) {
        return memload(flag);
    }

    else if (astrcmp(cmd, "AmmIDE") == 0) {
        printf("\033[2J\033[H");
        return AmmIDE(flag);
    }

    else if (astrcmp(cmd, "mkdir") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            mkdir_cmd(argv[i]);
        }
        return 1;
    }

    else if (astrcmp(cmd, "go") == 0 && argc > 0) {
        cd_cmd(argv[0], flag);
        return 1;
    }

    else if (astrcmp(cmd, "sizeof") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            sizeinfo(argv[i], flag);
            puts("\n");
        }
        return 1;
    }

    else if (astrcmp(cmd, "ls") == 0) {
        return ls_cmd();
    }

    else if (astrcmp(cmd, "touch") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            mkfile(argv[i]);
        }
        return 1;
    }

    else if (astrcmp(cmd, "nano") == 0 && argc > 0) {
        char tmp[64];
        snprintf(tmp, sizeof(tmp), "nano %s", argv[0]);
        system(tmp);
        return 1;
    }

    else if (astrcmp(cmd, "r") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, cat_cmd, "FILE") == 0){
                return 0;
            }
            puts("\n");
        }
        
    }

    else if (astrcmp(cmd, "neofetch") == 0) {
        neofetch();
	return 1;
    }

    else if (astrcmp(cmd, "AmmSH") == 0 && argc > 0 ) {
        char *results = amm_malloc(sizeof(int) * argc); // useing as int!
        int success = 1;

        for (int i = 0; i < argc; ++i) {
            results[i] = AmmSH(argv[i], flag);
            if (results[i] != 1) {
                printf("AmmSH: Error in script '%s'\n", argv[i]);
                success = 0;
            }
        }
        amm_free(results, sizeof(int) * argc);
        return success;
    }

    else if (astrcmp(cmd, "sleep") == 0 && argc > 0){ // brooo this works really slow :(
        char *nums = amm_malloc(argc * sizeof(int)); // useing as int!
        
        for(int i=0; i<argc; ++i){
            nums[i] = atoi(argv[i]);	
        }

        for(int i = 0; i<argc; ++i){
            sleep(nums[i]);
        }
        
        amm_free(nums, argc * sizeof(int));
        return 1;
    }

    else if (astrcmp(cmd, "getlogin") == 0) {
        volatile char* un = get_username(flag);

        if(un == NULL){
            return 0;
        }
        printf("%s\n", un);
        return 1;
    }

    else if (astrcmp(cmd, "agrep") == 0) {
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

        if (astrcmp(flage, "-r-file") == 0) {
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
        else if (astrcmp(flage, "-r-str") == 0) {
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

    else if(astrcmp(cmd, "int") == 0 && argc > 0){
        if(argc != 2){
            printf("[int] usage: int <name> <value>\n");
            return 0;
        }

        if(!variable_initialization(argv[0], argv[1], INT) && flag == NORMAL) 
            printf("AmmSH: Varible '%s' alredy exists\n", argv[0]);
        return 1;
    }

    else if(astrcmp(cmd, "char") == 0 && argc > 0){

        if(argc != 2){
            printf("[char] usage: char <name> <value>\n");
            return 0;
        }

        if(!variable_initialization(argv[0], argv[1], CHAR) && flag == NORMAL) 
            printf("AmmSH: Varible '%s' alredy exists", argv[0]);
        return 1;
    }

    else if(astrcmp(cmd, "str") == 0 && argc > 0){
        if(argc != 2){
            printf("[str] usage: str <name> <value>\n");
            return 0;
        }

        if(!variable_initialization(argv[0], argv[1], STRING) && flag == NORMAL) 
            printf("AmmSH: Varible '%s' alredy exists\n", argv[0]);
        return 1;
    }

    else if(astrcmp(cmd, "reval") == 0 && argc > 0){
        if(argc < 2){
            puts("[INFO] usage: reval <varname> (+=, -=, =) <value>");
            return 0;
        }

        if (astrcmp(argv[1], "=") != 0 &&
            astrcmp(argv[1], "+=") != 0 &&
            astrcmp(argv[1], "-=") != 0 ) {
                printf("AmmSH: Unknown operator '%s'\n", argv[1]);
                return 0;
        }
         

        int result = reval_var(argv[0], argv[2], argv[1]);
        switch (result){
        case 0: printf("AmmSH: varible '%s' does not exist in stack\n", vars[0].varname); break;
        case 60: puts("AmmSH: you can't add the string varible!"); break;
        case 1: break;
        }
        
        
    }   
    else if(astrcmp(cmd, "varsdump") == 0){
        vars_dump();
        return 1;
    }

    else if (astrcmp(cmd, "say") == 0 && argc > 0) {
        int ignoredolar = 0;

        char *filtered_argv[30];
        int filtered_argc = 0;

        
        for (int i = 0; i < argc; ++i) {
            if (astrcmp(argv[i], "-v") == 0) {
                ignoredolar = 1;
                continue;
            }
            filtered_argv[filtered_argc++] = argv[i];
        }

        
        if (!ignoredolar) {
            
            filtered_argc = preprocessor(filtered_argc, filtered_argv);
        }

        for (int i = 0; i < filtered_argc; ++i) {   
            echo_cmd(filtered_argv[i]); 
            if (i != filtered_argc - 1) {
                putchar(' '); 
            } else {
                putchar('\n'); 
            }
        }


        return 1;
    }



    else if (astrcmp(cmd, "rm") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, rm_cmd, "DIR") == 0){
                return 0;
            }
        }
        return 1;
    }

    else if (astrcmp(cmd, "rf") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(cat_expand(argv[i], flag, rf_cmd, "FILE") == 0){
                return 0;
            }
        }
        return 1;
    }

    else if(astrcmp(cmd, "if") == 0 || astrcmp(cmd, "else") == 0 || astrcmp(cmd, "endif") == 0 || astrcmp(cmd, "loop") == 0 || astrcmp(cmd, "endloop") == 0)
        return 1; // just do nothing :)

    
    else if (astrcmp(cmd, "calc") == 0) {
        calc();
        printf("\n");
	return 1;
    }

    else if (astrcmp(cmd, "fib") == 0) {
        fib();
	    return 1;
    }
    else if(astrcmp(cmd, "fac") == 0){
        factoral();
        return 1;
    }

    else if (astrcmp(cmd, "gpu") == 0) {
        vga_main();
	    return 1;
    }
    else if (astrcmp(cmd, "hlt") == 0){
        KERNEL_PANIC();
    }
    else if(astrcmp(cmd, "bitmapload") == 0){
        bitmapload(flag);
    }

    else if(astrcmp(cmd, "asystemd") == 0 && argc >= 1){
        if(astrcmp(argv[0], "start") == 0){
            AmmDemon d; // temp demon
            d.apid = Ammdemon_count;

            if (!AmmINI(argv[1], &d)) {
                return 0;
            }

            startdemon(&d);
            savedemon(&d);
            return 1;
        }

        else if(astrcmp(argv[0], "kill") == 0){
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

        else if(astrcmp(argv[0], "list") == 0){
            puts("");
            for (int i = 0; i < Ammdemon_count; ++i) {
                infodemon(&demons[i]);
                puts("\n");
            }
            return 1;
        }

    }
    else if(astrcmp(cmd, "help") == 0){
        puts("AmmSH: I didn't write docx file please check AmmOS/README.md or look at in https://github.com/LinuxCoder13/AmmOS.git");
    }
    else {
        printf("AmmSH: command '%s' not found!\n", cmd);
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


    char lines[30][128];
    int line_count = 0;
    
    
    while (fgets(buff, sizeof(buff), fl)) {
        clean_line(buff);

        if (strlen(buff) == 0)
            continue;

        char line_copy[128];
        strncpy(line_copy, buff, sizeof(line_copy));

        char *cmd = strtok(line_copy, " ");
        char *arg = strtok(NULL, "");
        // Убираем ведущие пробелы и табуляции
        while (arg && (*arg == ' ' || *arg == '\t'))
            arg++;


        if (!cmd) continue;


    
        // === LOOP ===
        if (astrcmp(cmd, "loop") == 0 && arg) {
            int infinity_loop = (astrcmp(arg, "inf") == 0);
            int loop_times    = atoi(arg);
            int found_end     = 0;
            char lines[30][128];
            int line_count    = 0;  

            // собираем тело цикла
            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);
                if (astrcmp(buff, "endloop") == 0) {
                    found_end = 1;
                    break;
                }

                if (line_count < 30) {
                    //printf("[DEBUG] coping str %d: «%s»\n", line_count, buff);

                    size_t len = strlen(buff);
                    if (len >= sizeof(lines[0])) len = sizeof(lines[0]) - 1;
                    memcpy(lines[line_count], buff, len);
                    lines[line_count][len] = '\0';
                    line_count++;
                }
            }

            // inf loop
            if(infinity_loop){ // broooo why srtcmp retunrn 0? why not 1?
                while (1){
                    for (int i = 0; i < line_count; ++i) {
                        //printf("[DEBUG] Executing line %d: «%s»\n", i, lines[i]);
                        AmmSH_execute(lines[i]);
                    }
                }
            }

            // basic loop
            for (int l = 0; l < loop_times; ++l) {
                for (int i = 0; i < line_count; ++i) {
                    AmmSH_execute(lines[i]);
                }
            }
        }

        // === IF/ELSE/ENDIF ===
        else if (astrcmp(cmd, "if") == 0 && arg) {
            // Убираем ведущие пробелы в arg
            int condition_met = 0;

            if (arg[0] == '$') {
                condition_met = checkvar_value(arg);
            } else {
                condition_met = atoi(arg) != 0 ? 1 : 0;
            }
            while (*arg == ' ' || *arg == '\t') arg++;
            
            char if_lines[30][128], else_lines[30][128];
            int if_count = 0, else_count = 0;
            int found_end = 0, in_else = 0;

            // Обработка ошибок
            if (condition_met == 60 || condition_met == 61 || condition_met == 62) {
                if (mode == NORMAL) {
                    if (condition_met == 60) puts("AmmSH: Variable not found in stack!");
                    else if (condition_met == 61) puts("AmmSH: Syntax error in if statement. Usage: if $<var1> == $<var2>");
                    else if (condition_met == 62) puts("AmmSH: Invalid token in if statement. Valid: '==', '<', '>'");
                }
                return 0;
            }

            // Считываем тело if / else
            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);

                if (astrcmp(buff, "else") == 0) {
                    in_else = 1;
                    continue;
                }

                if (astrcmp(buff, "endif") == 0) {
                    found_end = 1;
                    break;
                }

                if (in_else) {
                    if (else_count < 30)
                        strncpy(else_lines[else_count++], buff, sizeof(else_lines[0]) - 1);
                } else {
                    if (if_count < 30)
                        strncpy(if_lines[if_count++], buff, sizeof(if_lines[0]) - 1);
                }
            }

            if (!found_end) {
                if (mode == NORMAL) puts("AmmSH: Missing 'endif'");
                return 0;
            }

            char (*chosen)[128] = (condition_met == 1) ? if_lines : else_lines;
            int chosen_count = (condition_met == 1) ? if_count : else_count;

            for (int i = 0; i < chosen_count; ++i) {
                AmmSH_execute(chosen[i]);
            }

            return 1;
        }


        // === Просто команда ===
        else {
            AmmSH_execute(buff); // 
        }

    }



    fclose(fl);
    printf("\n");
    return 1;
}


