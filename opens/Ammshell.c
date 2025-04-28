#include <stdio.h>
#include <stdlib.h>
#include <string.h> // нужно для strcmp()
#include "Ammkernel.h"
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void) {

    chdir(FS_ROOT);
    up_path();
    char *user = username();

    while (1) {
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
	        free(user);
            exit(60); // выход с кодом 60
        }
        else if (strcmp(command, "load") == 0){
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
        else {
            printf("AmmSh: command not found!\n");
        }
    }
        
    return 0;
}
