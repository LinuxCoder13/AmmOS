#include <stdio.h>
#include <stdlib.h>
#include <string.h> // нужно для strcmp()
#include "Ammkernel.h"
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>


int main(void) {
    amm_init(); // init memory
    signal(SIGSEGV, sigsegv_handler); // segfalt handler

    chdir(FS_ROOT);
    up_path();

    char *user = username();

    for(;;) {   // I am old Unix man 
        char command[30];

        printf("[%s]AmmOS %s$: ", user, path);
        fgets(command, sizeof(command), stdin);

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "c") == 0) {
            printf("\033[2J\033[H");
        }
        else if (strcmp(command, "") == 0){
            continue;
        }
        else if (strcmp(command, "ex") == 0) {
	        amm_free(user, 15);
            exit(0); 
        }
        else if (strcmp(command, "diskload") == 0){
            diskread();
        }
        else if(strcmp(command, "memload") == 0){
            memload();
        }
        else if (strcmp(command, "AmmIDE") == 0){
            printf("\033[2J\033[H");
            AmmIDE();
        } 
        else if(strncmp(command, "mkdir ", 6) == 0){
            removen(command, 6);
            mkdir(command, 0755);
        }
        else if (strncmp(command, "go ", 3) == 0){
            removen(command, 3);
            cd_cmd(command);   // функция в AmmFS.c
        }
        else if(strncmp(command, "sizeof ", 7) == 0){
            removen(command, 7);
            sizeinfo(command);
        }
        else if(strcmp(command, "ls") == 0){
            ls_cmd();
        }
        else if(strncmp(command, "touch ", 6) == 0){
            removen(command, 6);
            mkfile(command);
        }
        else if(strncmp(command, "nano ", 5) == 0){
            removen(command, 5);
            char tmp[20];
            sprintf(tmp, "nano %s", command);
            system(tmp);
        }
        else if(strncmp(command, "r ", 2) == 0){
            removen(command, 2);
            cat_cmd(command);
        }
	    else if(strcmp(command, "neofetch") == 0){
	        neofetch();
	    }
        else if(strncmp(command, "AmmSH ", 6) == 0){
            removen(command, 6);
            AmmSH(command);
        }
        else if(strncmp(command, "getlogin", 8) == 0){
            get_username();
        }
        else if(strncmp(command, "say ", 4) == 0){
            removen(command, 4);
            echo_cmd(command);
        }
        else if(strncmp(command, "rm ", 3) == 0){
            removen(command, 3);
            rm_cmd(command);
        } 
        else if(strncmp(command, "rf ", 3) == 0){
            removen(command, 3);
            rf_cmd(command);
        }
        else if(strcmp(command, "calc") == 0){
            calc();
            printf("\n"); 
        }
        else if(strcmp(command, "fib") == 0){
            fib();
        }
        else if(strcmp(command, "gpu") == 0){
            vga_main();
        }
        else {
            printf("AmmSh: command not found!\n");
        }
    }
        
    return 0;
}
