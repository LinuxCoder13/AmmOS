#include <stdio.h>
#include <stdlib.h>
#include <string.h> // нужно для strcmp()
#include "Ammkernel.h"
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>

 

int main(void) {
   
    
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
           
        else {
            printf("AmmSh: command not found!\n");
        }
    }

    return 0;
}

