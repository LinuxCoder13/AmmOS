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
#include <pthread.h>
#include <time.h>

void clean_line(char *line) {
    removetab(line);
    line[strcspn(line, "\n")] = 0;
}



int AmmSH_execute(char *line, int *col) {
    char buff[100];
    char argv[30][30];
    int argc = 0;

    strncpy(buff, line, sizeof(buff));
    char *cmd = strtok(buff, " ");
    if (!cmd) return 0;

    char *token = strtok(NULL, " ");
    while (token && argc < 30) {    // parse all 
        strncpy(argv[argc], token, 30);
        token = strtok(NULL, " ");
        argc++;
    }

    // CLI
    if (strcmp(cmd, "c") == 0) {
        printf("\033[2J\033[H");
        return 1;
    }

    else if (strcmp(cmd, "ex") == 0) {
        exit(0);
    }

    else if (strcmp(cmd, "diskload") == 0) {
        return diskread();
    }

    else if (strcmp(cmd, "memload") == 0) {
        return memload();
    }

    else if (strcmp(cmd, "AmmIDE") == 0) {
        printf("\033[2J\033[H");
        return AmmIDE();
    }

    else if (strcmp(cmd, "mkdir") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            mkdir_cmd(argv[i]);
        }
        return 1;
    }

    else if (strcmp(cmd, "go") == 0 && argc > 0) {
        cd_cmd(argv[0]);
        return 1;
    }

    else if (strcmp(cmd, "sizeof") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            sizeinfo(argv[i]);
            puts("\n");
        }
        return 1;
    }

    else if (strcmp(cmd, "ls") == 0) {
        ls_cmd();
        return 1;
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
            if(cat_cmd(argv[i]) == 0){
                return 0;
            }
            puts("");
        }
        
    }

    else if (strcmp(cmd, "neofetch") == 0) {
        neofetch();
    }

    else if (strcmp(cmd, "AmmSH") == 0 && argc > 0 ) {
        int *results = amm_malloc(sizeof(int) * argc);
        int success = 1;

        for (int i = 0; i < argc; ++i) {
            results[i] = AmmSH(argv[i]);
            if (results[i] != 1) {
                printf("AmmSH: Error in script '%s'\n", argv[i]);
                success = 0;
            }
        }

        return success;
    }
    else if (strcmp(cmd, "getlogin") == 0) {
        volatile char* un = get_username();

        if(un == NULL){
            return 0;
        }
        printf("%s\n", un);
        return 1;
    }

    else if (strcmp(cmd, "say") == 0 && argc > 0) {
        return echo_cmd(argv[0]);
    }

    else if (strcmp(cmd, "rm") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(rm_cmd(argv[i]) == 0){
                return 0;
            }
        }
        return 1;
    }

    else if (strcmp(cmd, "rf") == 0 && argc > 0) {
        for (int i = 0; i < argc; i++) {
            if(rf_cmd(argv[i]) == 0){
                return 0;
            }
        }
        return 1;
    }

    else if(strcmp(cmd, "if") == 0 || strcmp(cmd, "else") == 0 || strcmp(cmd, "endif") == 0 || strcmp(cmd, "loop") == 0 || strcmp(cmd, "endloop") == 0)
        return 1; // just do nothing :)

    else if (strcmp(cmd, "calc") == 0) {
        calc();
        printf("\n");
    }

    else if (strcmp(cmd, "fib") == 0) {
        fib();
    }

    else if (strcmp(cmd, "gpu") == 0) {
        vga_main();
    }

    else {
        if(col == NULL) // for CLI error
            printf("AmmSH: command not found!\n");
        else
            printf("AmmSH: command not found in '%d' line!\n", *col); // for file
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
            int loop_times = atoi(arg);
            line_count = 0;
            int found_end = 0;

            while (fgets(buff, sizeof(buff), fl)) {
                clean_line(buff);
                if (strcmp(buff, "endloop") == 0) {
                    found_end = 1;
                    break;
                }
                if (line_count < 30)
                    strncpy(lines[line_count++], buff, sizeof(lines[0]));
            }

            if (!found_end) {
                if(mode != 1){
                    printf("AmmSH: Missing 'endloop'\n");
                    return 0;
                }
            }

            for (int l = 0; l < loop_times; ++l) {
                for (int i = 0; i < line_count; ++i) {
                    AmmSH_execute(lines[i], &col);
                }
            }
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
                if(mode != 1){
                    printf("AmmSH: Missing 'endif'\n");
                    return 0;
                }
            }

            char (*chosen)[128] = condition_met ? if_lines : else_lines;
            int chosen_count = condition_met ? if_count : else_count;

            for (int i = 0; i < chosen_count; ++i) {
                AmmSH_execute(chosen[i], &col);
            }
        }

        // === Просто команда ===
        else {
            volatile int res = AmmSH_execute(buff, &col); // if -O2
        }
    }

    fclose(fl);
    printf("\n");
    return 1;
}
