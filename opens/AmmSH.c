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
	int *nums = amm_malloc(argc);
	
	for(int i=0; i<argc; ++i){
	    nums[i] = atoi(argv[i]);	
	}

	for(int i = 0; i<argc; ++i){
	    sleep(nums[i]);
	}
	
	amm_free(nums, argc);
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

    else if (strcmp(cmd, "say") == 0 && argc > 0) {
        for(int i=0; i<argc; ++i){    
            echo_cmd(argv[i], col); // see this function in AmmFS.c 151 line 
        }
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
    // +thr block
    else if (strcmp(cmd, "+thr") == 0){
	// so we need to start call function AmmSH(argv[i], mode); // but we need to work it in fone mode, so we will use pthread();

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


        if (strncmp(buff, "i=", 2) == 0){
            removen(buff, 2);
            when_to_break = atoi(buff);
            buff[0] = '\0'; // I did somesing
        } 


        
        line_count = 0; // index of loop. To print index use '^' char
        // === LOOP ===
        if (strcmp(cmd, "loop") == 0 && arg) {
            int infinity_loop = strcmp(arg, "inf") == 0; // hello C99
            int loop_times = atoi(arg);
            int found_end = 0; 

            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);
                if (strcmp(buff, "endloop") == 0) {
                    found_end = 1;
                    break;
                }

                // sorry for ctrl + c, ctrl + v
                if (strncmp(buff, "i=", 2) == 0){
                    removen(buff, 2);
                    when_to_break = atoi(buff);
                    buff[0] = '\0'; // I did somesing
                } 
                

                if (line_count < 30)
                    strncpy(lines[line_count++], buff, sizeof(lines[0]));
            }

            if (!found_end) {
                if(mode == NORMAL){
                    printf("AmmSH: Missing 'endloop'\n");
                    return 0;
                }
                return 0;
            }

            int iter = 0;
            col = 0;
            // inf loop
            if(infinity_loop){ // broooo why srtcmp retunrn 0? why not 1?
                while (1){
                    for (int i = 0; i < line_count; ++i) {
                        if(col == when_to_break){
                            goto end;
                        } 
                        AmmSH_execute(lines[i], &col);
                    }
                    iter++; // becouse inf loop :)
                    col++;
                }
                col = 0;
            }

            // basic loop
            for (int l = 0; l < loop_times; ++l) {
                for (int i = 0; i < line_count; ++i) {
                    if(loop_times == when_to_break){
                        goto end;
                    } 
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

            char (*chosen)[128] = condition_met ? if_lines : else_lines;
            int chosen_count = condition_met ? if_count : else_count;

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


