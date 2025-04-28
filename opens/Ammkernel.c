/* Ammkernel - is main function-database it send tecnology to AmmFS, Ammshell, ect.
the prototype of functions, variables(extern), makros are in Ammkernel.h 
kernel life (2025 - ...)*/

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

int get_username(){
    char fpath[100];	
	char fpath2[100];

	getcwd(fpath, 100);
	getcwd(fpath2, 100);

	cut_after_substr(fpath, "AmmOS/opens/user");
	chdir(fpath);

	char username[20];

	FILE *fl = fopen("username.txt", "r");
	if (fgets(username, 20, fl)) {
        username[strcspn(username, "\n")] = 0;
    }

    printf("%s\n", username); 
 	
	fclose(fl);

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

int memload(){
    char obs_path2[100];
    char obs_path[100];

    getcwd(obs_path, 100);
    getcwd(obs_path2, 100);

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

    char buf[200];
    sprintf(buf, "Memory/memory.dat");

    FILE *fl = fopen(buf, "rb");
    if (!fl) return 0;
    int ch;
    while ((ch = fgetc(fl)) != EOF)
        printf("%c", ch);
    printf("\n");
    
    fclose(fl);
    chdir(obs_path2);   // come back
    return 1;
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

    	 
int AmmIDE(){
    
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
                return 0;
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

            if(!fl){
                return 0;
            }

            fseek(fl, 0, SEEK_END);
            long size = ftell(fl);
            fclose(fl);

            if (size > 0)
               truncate(temp, size-1); 
        }
        else if (strncmp(commads[2], buff, 4) == 0){
            short res = memload();
        }

        else if (strcmp("exit", buff) == 0){
	       chdir(obs_path2); // I alweys come back;
           printf("\n"); 
           return 1;
        }

        else{
            printf("AmmIDE: no command found!\n");
        } 

                
    } 
}


int neofetch(){

    printf("\033[1;33m        ==AmmOS==\033[0m\n\n");
    printf("\033[1;37m       /\\\033[0m          \033[;31mKernel:\033[0m Ammkernel v0.6 \n");
    printf("\033[1;37m      /  \\\033[0m         \033[;31mShell:\033[0m Ammshell v0.4 (not bash) \n");
    printf("\033[1;37m     /    \\\033[0m        \033[;31mVersion:\033[0m 0.8 \n");
    printf("\033[1;37m    /======\\\033[0m       \033[;31mFS:\033[0m AmmFS (calls Linux)\n");
    printf("\033[1;37m   / Amm-OS \\\033[0m      \033[;31mMemory:\033[0m 10.2kb\n");
    printf("\033[1;37m  /==========\\\033[0m     \033[;31mGPU:\033[0m Soon\n");
    printf("\033[1;37m /    ____    \\\033[0m    \033[;31mRAM:\033[0m memory.dat\n");
    printf("\033[1;37m/____/    \\____\\\033[0m    \n");	

    return 1;
}


// this functions must for write AmmSH-scripts letsss go!
// I didn't want write it in kernel but it's ok ...


// 1 line commands
int AmmSH_execute(const char *line, int col) {
    char buff[50];
    strncpy(buff, line, sizeof(buff));

    char *cmd = strtok(buff, " ");
    char *arg = strtok(NULL, "");

    if (!cmd) return 0;

    if (strcmp(cmd, "print") == 0 && arg) {
        printf("%s", arg);
        return 1;
    }
    else if(strcmp(cmd, "say") == 0 && arg){
        echo_cmd(arg);
    }
    else if (strcmp(cmd, "AmmIDE") == 0) {
        return AmmIDE();
    }
    else if (strcmp(cmd, "username") == 0) {
        printf("%s", get_username());
        return 1;
    }
    else if (strcmp(cmd, "mkdir") == 0 && arg) {
        return mkdir_cmd(arg);
    }
    else if(strcmp(cmd, "touch") == 0 && arg){
        return mkfile(arg);
    }
    else if (strcmp(cmd, "load") == 0) {
        return memload();
    }
    else if (strcmp(cmd, "go") == 0 && arg) {
        return cd_cmd(arg);
    }
    else if (strcmp(cmd, "sizeof") == 0 && arg) {
        return sizeinfo(arg);
    }
    else if (strcmp(cmd, "ls") == 0) {
        return ls_cmd();
    }
    else if (strcmp(cmd, "r") == 0 && arg) {
        return cat_cmd(arg);
    }
    else if (strcmp(cmd, "neofetch") == 0) {
        return neofetch();
    }
    else {
        printf("AmmOS: Bro what syntax did you write in '%d' line go fix or I will burn you PC\n", col);
        return 0;
    }
}

// here is basic commands for every programming lang
void AmmSH(const char *file_to_inter) {
    char buff[50];
    FILE *fl = fopen(file_to_inter, "r");
    if (!fl) {
        printf("AmmOS: Cannot open file '%s'\n", file_to_inter);
        return;
    }

    int col = 0;
    char lines[30][50];
    int line_count = 0;

    while (fgets(buff, sizeof(buff), fl)) {
        col++;
    
        buff[strcspn(buff, "\n")] = 0;
    
        char line_copy[50];
        strncpy(line_copy, buff, sizeof(line_copy));
    
        char *cmd = strtok(line_copy, " ");
        char *arg = strtok(NULL, "");
    
        if (!cmd) continue;
    
        if (strcmp(cmd, "loop") == 0 && arg) {
            int loop_times = atoi(arg);
            line_count = 0;
    
            while (fgets(buff, sizeof(buff), fl)) {
                buff[strcspn(buff, "\n")] = 0;
                if (strcmp(buff, "endloop") == 0) break;
                strncpy(lines[line_count++], buff, sizeof(buff));
            }
    
            for (int l = 0; l < loop_times; ++l) {
                for (int i = 0; i < line_count; ++i) {
                    AmmSH_execute(lines[i], col);

                }
            }
        }

        else if (strcmp(cmd, "if") == 0 && arg) {
            int lines_count = 0;
            int condition_met = AmmSH_execute(arg, col);
            
            char if_lines[30][50];
            char else_lines[30][50];
            int has_else = 0;  // есть ли else ваще?
        
            // Читаем if-блок
            while (fgets(buff, sizeof(buff), fl)){
                buff[strcspn(buff, "\n")] = 0;
                if (strcmp(buff, "else") == 0) {
                    has_else = 1;
                    break;
                }
                if (strcmp(buff, "endif") == 0) break;
                strncpy(if_lines[lines_count++], buff, sizeof(buff));
            }
        
            int else_count = 0;
        
            if (has_else) {

                while (fgets(buff, sizeof(buff), fl)){
                    buff[strcspn(buff, "\n")] = 0;
                    if (strcmp(buff, "endif") == 0) break;
                    strncpy(else_lines[else_count++], buff, sizeof(buff));
                }
            }
        
            if (condition_met) {
                for (int i = 0; i < lines_count; ++i) {
                    AmmSH_execute(if_lines[i], col);
                }
            } else if (has_else) {
                for (int i = 0; i < else_count; ++i) {
                    AmmSH_execute(else_lines[i], col);
                }
            }
        }
        

        else{    
            AmmSH_execute(buff, col);
        }
            
    }

    fclose(fl);
    printf("\n");
}





