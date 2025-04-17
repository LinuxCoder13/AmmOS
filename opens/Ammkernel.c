#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Ammkernel.h"

#define VERSION "0.5"
#define KERNEL "AmmKernel"

// functions that will help write kernel

void str_ascii(char *str, int *arr){
    int i=0;
    while (str[i] != '\0'){
        arr[i] = (int)str[i];
        i++;
    }
    arr[i] = '\0';
    return;
}
int ascii_int(char c){
    return (int)c;
}
int int_ascii(int integer){
    return (char)integer;
}
void ascii_str(int *arr, int sizearr, char *out){
    for (int i=0; i<sizearr; i++){
        out[i] = (char)arr[i];
    }
    out[sizearr] = '\0';
}
void memload(){
    chdir(FS_ROOT);
    chdir("../../..");
   
    char buf[200];
    sprintf(buf, "Memory/memory.dat");

    FILE *fl = fopen(buf, "rb");
    int ch;
    while ((ch = fgetc(fl)) != EOF)
        printf("%c", ch);
    printf("\n");

    fclose(fl);
}
void removen(char *str, int n){
    int len = strlen(str);
    if (n >= len){
        str[0] = '\0';
        return;
    }
    memmove(str, str+n, len-n+1);
}

// now functions for os

void AmmIDE(){
    chdir(FS_ROOT);
    chdir("../../..");
    

    while (1){
        char buff[30];
        const char *commads[] = {"push ", "free", "read"};

        printf("->>> ");
        fgets(buff, sizeof(buff), stdin);
        
        buff[strcspn(buff, "\n")] = '\0';

        if (strncmp(commads[0], buff, 5) == 0){
            char buffer[200];
            sprintf(buffer, "Memory/memory.dat");

            FILE *fl = fopen(buffer, "ab");
            if(!fl){
                perror("NONE\n");
                continue;
            }
            removen(buff, 5);
            
            int num = atoi(buff);
            char c = int_ascii(num);
            fputc(c, fl);
            
            printf("AmmIDE: pushing '%c', (%d) to memory.\n", c, c);
            fclose(fl);
        }
        else if (strncmp(commads[1], buff, 4) == 0){
            char temp[200];
            sprintf(temp, "Memory/memory.dat");

            FILE *fl = fopen(temp, "rb");
     
            fseek(fl, 0, SEEK_END);
            long size = ftell(fl);
            fclose(fl);

            if (size > 0)
               truncate(temp, size-1); 
        }
        else if (strncmp(commads[2], buff, 4) == 0){
            memload();
        }

        else if (strcmp("exit", buff) == 0){
           printf("\n"); 
           return;
        }

        else{
            printf("AmmIDE: no command found!\n");
        } 

                
    } 
}

void total_help(){
	chdir(FS_ROOT);
	system("nano _help_.txt");
}


void neofetch(){
	printf("")



}







