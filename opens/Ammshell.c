#include <stdio.h>
#include <stdlib.h>
#include <string.h> // нужно для strcmp()
#include "Ammkernel.h"
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>

int main(void) {
    char path[256];
    int len = 0;
    char *mainpwd = malloc(sizeof(char) * 1024);   // кушаю твой память
    
    getcwd(mainpwd, 1024);
    char *ospwd = strstr(mainpwd, "/main");
    sprintf(path, "~%s", ospwd + strlen("/main")); // убираем наху "/main" с    принтуем в path "~" для даланса вселеной
    
    free(mainpwd); // чють не забыл  
    
    while (1) {
        char command[30];
        
        printf("[root]AmmOS ~$: ");
        fgets(command, sizeof(command), stdin);
            
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "c") == 0) {
            system("clear");
        }
        else if (strcmp(command, "") == 0){
            continue;
        }  
        else if (strcmp(command, "ex") == 0) {
            exit(60); // выход с кодом 60
        }
        else if (strcmp(command, "load") == 0){
            memload();
        }
        else if (strcmp(command, "AmmIDE") == 0){
            system("clear");
            AmmIDE();
        } 
        else if (strncmp(command, "cd ", 3) == 0){
            removen(command, 3);
            cd_cmd(command);   // функция в AmmFS.c
        }
        else {
            printf("AmmSh: command not found!\n");
        }
    }

    return 0;
}
