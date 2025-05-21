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
#include <sys/mman.h>


char *MEMORY; // amm_malloc(), amm_free() 
short bit_map[MEMSIZE];  // 1, 0

// functions that will help write kernel

void amm_init(void) {
    MEMORY = mmap(NULL, MEMSIZE*2, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | 0x20, -1, 0);
    if (MEMORY == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    memset(MEMORY, 0, MEMSIZE*2);  // очищаем ВСе а не только MEMSIZE
    memset(bit_map, 0, sizeof(bit_map));  // битмап должен быть достаточно большим !.!.!.!
}

// I a sorry
void *amm_malloc(int size) {
    if (size <= 0) {
        return NULL;
    }

    int blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int consecutive = 0;
    int total_blocks = MEMSIZE / BLOCK_SIZE;

    for (int i = 0; i < total_blocks; ++i) {
        if (!bit_map[i]) {
            if (++consecutive == blocks_needed) {
                int start = i - blocks_needed + 1;
                for (int j = start; j <= i; ++j) {
                    bit_map[j] = 1;
                }
                return MEMORY + start * BLOCK_SIZE;
            }
        } else {
            consecutive = 0;
        }
    }

    return NULL;
}


void amm_free(void *ptr, int size) {
    if (size <= 0) return;
    if (!ptr) return;

    int start = ((char *)ptr - MEMORY) / BLOCK_SIZE;
    int blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int total_blocks = MEMSIZE / BLOCK_SIZE;

    for (int i = start; i < start + blocks && i < total_blocks; ++i) {
        bit_map[i] = 0; // освобождаем биты в бит мап а в памяти?! не надо ;) 
    }
}

void cut_after_substr(char *path, const char *substr);           

char* username(){
    char fpath[100];	
    char fpath2[100];

    getcwd(fpath, 100);
    getcwd(fpath2, 100);

    cut_after_substr(fpath, "AmmOS/opens/user");
    chdir(fpath);

    char *username = (char*)amm_malloc(15);
    username[0] = '\0';

    FILE *fl = fopen("username.txt", "r");
    if (fl && fgets(username, 15, fl)) {
        username[strcspn(username, "\n")] = '\0'; 
    }
    if (fl) fclose(fl);


    if (username[0] == '\0' || strlen(username) < 2) {
        printf("Hello, Welcome to AmmOS!\nPlease write your username to continue: ");
        fgets(username, 15, stdin);
        username[strcspn(username, "\n")] = '\0';

        FILE *fl_w = fopen("username.txt", "w");
        if (fl_w) {
            fprintf(fl_w, "%s\n", username);
            fclose(fl_w);
        }
    }

    chdir(fpath2);
    return username;
}


void removetab(char *str) {
    int i = 0;
    while (str[i] == ' ' || str[i] == '\t') i++; 
    if (i > 0) {
        int j = 0;
        while (str[i]) str[j++] = str[i++];
        str[j] = '\0';
    }
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

void int_ascii(long value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    char buff[20];
    int i = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    while (value > 0) {
        buff[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buff[i++] = '-';
    }

    int j = 0;
    while (i > 0) {
        str[j++] = buff[--i];
    }
    str[j] = '\0';
}


void ascii_str(int *arr, int sizearr, char *out){
    for (int i=0; i<sizearr; i++){
        out[i] = (char)arr[i];
    }
    out[sizearr] = '\0';
}

int memload(){
    write(1, "load memory int or char or hex?(i, c, h): ", 43); // syscall lol. rdi, rax, rdx, rsi. if I am not mistake. 
    int res = getchar();


    while(getchar() != '\n'); 
    if(res == 'i'){
        for(int i=0; i<MEMSIZE-29; ++i){
            printf("%#d, ", MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'c'){
        for(int i=0; i<MEMSIZE-29; ++i){
            printf("%#c, ", MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'h'){
        for(int i=0; i<MEMSIZE-29; i++){
            printf("%#x, ", MEMORY[i]);
        }
        return 1;
    }

    else{
        return 0;
    }
}
/* print disk.dat and return 0-false(somesing went wrong) 1-true(succses)*/
int diskread(){
    char* obs_path2 = amm_malloc(100);
    char* obs_path = amm_malloc(100);

    getcwd(obs_path, 100);
    getcwd(obs_path2, 100);

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

    char buf[200];
    sprintf(buf, "Memory/disk.dat");

    FILE *fl = fopen(buf, "rb");
    if (!fl) return 0;
    int ch;
    while ((ch = fgetc(fl)) != EOF)
        printf("%c", ch);
    printf("\n");
    
    fclose(fl);
    chdir(obs_path2);   // come back
    amm_free(obs_path2, 100);
    amm_free(obs_path, 100);
    return 1;
}
inline void removen(char *str, int n){
    int len = strlen(str);
    if (n >= len){
        str[0] = '\0';
        return;
    }
    memmove(str, str+n, len-n+1);
}


void cut_after_substr(char *path, const char *substr) { // bro! inline is a good item use it!
    char *pos = strstr(path, substr);
    if (pos) {
        pos += strlen(substr);
        *pos = '\0';
    }
}



// now functions for os

    	 
int AmmIDE(){
    
    char* obs_path2 = (char*)amm_malloc(256);
    char* obs_path = (char*)amm_malloc(256);

    getcwd(obs_path2, 256);
    getcwd(obs_path, 256);

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
            sprintf(buffer, "Memory/disk.dat");

            FILE *fl = fopen(buffer, "ab");
            if(!fl){
                return 0;
            }
            removen(buff, 5);
            
            int num = atoi(buff);
            char c = (char)num;
            fputc(c, fl);
            
            printf("AmmIDE: pushing '%c', (%d) to memory.\n", c, c);
            fclose(fl);
        }
        else if (strncmp(commads[1], buff, 4) == 0){
            char temp[200];
            sprintf(temp, "Memory/disk.dat");

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
            short res = diskread();
        }

        else if (strcmp("exit", buff) == 0){
	       chdir(obs_path2); // I alweys come back;
           printf("\n");
           amm_free(obs_path, 256);
           amm_free(obs_path2, 256);
           return 1;
        }

        else{
            printf("AmmIDE: no command found!\n");
        } 

                
    }
    amm_free(obs_path, 256);
    amm_free(obs_path2, 256);
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
    char *arg = strtok(NULL, " ");

    if (!cmd) return 0;

 
    else if (strcmp(cmd, "c") == 0 && arg) {
        write(1, "\033[2J\033[H", 8);
        return 1;
    }
    else if (strcmp(cmd, "print") == 0 && arg) {
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
    // else if (strcmp(cmd, "malloc") == 0) {
    //     int size = atoi(arg);
    //     void* amm_malloc(size)
    // }
    
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

        removetab(buff);
        col++;
    
        buff[strcspn(buff, "\n")] = 0;
    
        char line_copy[50];
        strncpy(line_copy, buff, sizeof(line_copy));
        // just a ...

        char *cmd = strtok(line_copy, " ");
        char *arg = strtok(NULL, "");
    
        if (!cmd) continue;
    
        if (strcmp(cmd, "loop") == 0 && arg) {
            int loop_times = atoi(arg);
            line_count = 0;
            
            while (fgets(buff, sizeof(buff), fl)) {
                removetab(buff);
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
                removetab(buff);
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
                    removetab(buff);
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
/*endAmmSH*/

/*We have memory and alloc memory so we need kernel panic if user will do segfalt in MEMORY >:)*/

void KERNEL_PANIC(){
    printf("\033[106m             Kernel panic!             \n");
    printf("                                       \n");
    printf("                /                      \n");
    printf("          X    /                       \n");    
    printf("              /                        \n");
    printf("              \\                       \n");
    printf("          X    \\                      \n");
    printf("                \\                     \n");            
    printf("                                       \n");
    printf("Please restart AmmOS                   \033[0m\n");
    
    fflush(stdout);
    exit(1);
}


void sigsegv_handler(int signum) {
    KERNEL_PANIC();
}


void kprint(char* text) {
    puts(text);
}

int ret_int(char* str){
    return atoi(str);
}


// amm_string functions --->

char *catstr(char* s1, char* s2){
    int len1 = 0, len2 = 0;

    while(s1[len1]) len1++;
    while(s1[len2]) len2++;

    char *resultstr = amm_malloc(len1 + len2 + 1); // +1 for '\0'
    int a = 0;

    for(; a < len1; a++) resultstr[a] = s1[a];
    a = 0; // xor a, a 
    
    for(; a < len2; a++) resultstr[a + a] = s2[a];
    
    return resultstr; // after use amm_free() btw
}

void replace(char *str, char target, char value){ // не ну а че
    for(int i=0; str[i] != '\0'; ++i){
        if(str[i] == target){
            str[i] = value;
        }
    }
    return; 
}


// I made this bro for .asm
void* funcs[] = {   // total 27 functions 
    amm_malloc, amm_free, username, str_ascii,  
        
    ascii_int, int_ascii, ascii_str, diskread, 
      
    AmmIDE, removen, mkfile, mkdir_cmd, 
        
    cd_cmd, up_path, ls_cmd, sizeinfo,
        
    cat_cmd, neofetch, AmmSH, get_username,
        
    echo_cmd ,memload, rm_cmd, rf_cmd,
    
    kprint, catstr, ret_int, KERNEL_PANIC

    
    
};

