#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Ammkernel.h"
#include <dirent.h>



// functions that will help write kernel
void cut_after_substr(char *path, const char *substr);           // must -_-

char* username(){
	char fpath[100];	
	char fpath2[100];

	getcwd(fpath, 100);
	getcwd(fpath2, 100);

	cut_after_substr(fpath, "AmmOS/opens/user");
	chdir(fpath);

	char *username = malloc(15);

	FILE *fl = fopen("username.txt", "r");
	if (fgets(username, 15, fl)) {
                username[strcspn(username, "\n")] = '\0';
        }

	
	fclose(fl);
	if(username[0] == '\0'){
		printf("Hello Welcome to AmmOS!\nPlease write your username to continue: ");
		fgets(username, 15, stdin);
		username[strcspn(username, "\n")] = '\0';
	
		FILE *fl_w = fopen("username.txt", "w");
		fprintf(fl_w, "%s\n", username);
		fclose(fl_w);
	}

	chdir(fpath2); //  come back to FS
	return username;
}
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
    char obs_path2[100];
    char obs_path[100];

    getcwd(obs_path, 100);
    getcwd(obs_path2, 100);

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

    char buf[200];
    sprintf(buf, "Memory/memory.dat");

    FILE *fl = fopen(buf, "rb");
    int ch;
    while ((ch = fgetc(fl)) != EOF)
        printf("%c", ch);
    printf("\n");
    
    fclose(fl);
    chdir(obs_path2);   // come back
}
void removen(char *str, int n){
    int len = strlen(str);
    if (n >= len){
        str[0] = '\0';
        return;
    }
    memmove(str, str+n, len-n+1);
}


void cut_after_substr(char *path, const char *substr) {
    char *pos = strstr(path, substr);
    if (pos) {
        pos += strlen(substr);
        *pos = '\0';
    }
}



// now functions for os

    	 
void AmmIDE(){
    
    char obs_path2[256];
    char obs_path[256];

    getcwd(obs_path2, sizeof(obs_path2));
    getcwd(obs_path, sizeof(obs_path));

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

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
                perror("\n");
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
	   chdir(obs_path2); // I alweys come back;
           printf("\n"); 
           return;
        }

        else{
            printf("AmmIDE: no command found!\n");
        } 

                
    } 
}


void neofetch(){

printf("\033[1;33m        ==AmmOS==\033[0m\n\n");
printf("\033[1;37m       /\\\033[0m          \033[;31mKernel:\033[0m Ammkernel v0.6 \n");
printf("\033[1;37m      /  \\\033[0m         \033[;31mShell:\033[0m Ammshell v0.4 (not bash) \n");
printf("\033[1;37m     /    \\\033[0m        \033[;31mVersion:\033[0m 0.8 \n");
printf("\033[1;37m    /======\\\033[0m       \033[;31mFS:\033[0m AmmFS (calls Linux)\n");
printf("\033[1;37m   / Amm-OS \\\033[0m      \033[;31mMemory:\033[0m 10.2kb\n");
printf("\033[1;37m  /==========\\\033[0m     \033[;31mGPU:\033[0m Soon\n");
printf("\033[1;37m /    ____    \\\033[0m    \033[;31mRAM:\033[0m memory.dat\n");
printf("\033[1;37m/____/    \\____\\\033[0m    \n");	
}







