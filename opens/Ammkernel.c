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
#include <time.h>
#include <pthread.h>
#include <signal.h>

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
void *amm_malloc(int __size) {
    if (__size <= 0) {
        return NULL;
    }

    int blocks_needed = (__size + BLOCK_SIZE - 1) / BLOCK_SIZE; // +1
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

char* append(int *len, int *cap, char* oldarr, char value) {
    if (*cap <= 0) *cap = 4;

    if (oldarr == NULL) {
        oldarr = amm_malloc(sizeof(char) * (*cap));
    }

    if (*len >= *cap) {
        *cap *= 2;
        char *newarr = amm_malloc(sizeof(char) * (*cap));
        if (newarr == NULL) return oldarr;

        for (int i = 0; i < *len; i++) {
            newarr[i] = oldarr[i];
        }

        amm_free(oldarr, *len);  // исправлено
        oldarr = newarr;
    }

    oldarr[*len] = value;
    (*len)++;
    return oldarr;
}




// HELLO WORLD
void** TwoDappend(int *len, int *cap, void **arr, void* value) {
    // if void **arr = NULL;
    if (*cap == 0) {
        *cap = 4;
        arr = amm_malloc(*cap * sizeof(void*));
        *len = 0;
    }

    
    if (*len >= *cap) {
        int new_cap = *cap * 2;
        void **new_arr = amm_malloc(new_cap * sizeof(void*));

    
        for (int i = 0; i < *len; ++i) {
            new_arr[i] = arr[i];
        }

        amm_free(arr, *cap * sizeof(void*));

        arr = new_arr;
        *cap = new_cap;
    }

    arr[*len] = value;
    (*len)++;

    return arr;
}

void TwoDfree(char **arr, int len) {
    for (int i = 0; i < len; i++) {
        if (arr[i]) {
            amm_free(arr[i], strlen(arr[i]) + 1); // Освобождаем каждую строку
        }
    }
    amm_free(arr, len * sizeof(char*)); // Освобождаем массив указателей. вот и все!
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
    
    
    if(fl) fclose(fl);

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




char* get_username(AmmSHFlags mode) {
    static char username[20]; 
    char *fpath = amm_malloc(100);	
    char *fpath2 = amm_malloc(100);

    getcwd(fpath, 100);
    getcwd(fpath2, 100);

    cut_after_substr(fpath, "AmmOS/opens/user");
    chdir(fpath);

    FILE *fl = fopen("username.txt", "r");
    if(fl == NULL){
        if(mode == 2) puts("Geting username failed!");
        return NULL;
    }
    if (fl && fgets(username, 20, fl)) {
        username[strcspn(username, "\n")] = 0;
    }

    if (fl) fclose(fl);
    chdir(fpath2);
    amm_free(fpath, 100);
    amm_free(fpath2, 100);
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

int memload(AmmSHFlags mode){
    write(1, "load memory int or char or hex or value?(i, c, h, v): ", 55); // syscall lol. rdi, rdx, rsi. if I am not mistake. 
    unsigned char res;
    scanf("%c", &res);  // I am sorry
     
    while(getchar() != '\n');
     
    if(res == 'i'){
        for(int i=0; i<MEMSIZE-29; ++i){
            printf("%d, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'c'){
        for(int i=0; i<MEMSIZE-29; ++i){
            printf("%c, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'h'){
        for(int i=0; i<MEMSIZE-29; i++){
            printf("%#x, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }

    else if (res == 'v') {
        for (int i = 0; i < MEMSIZE - 3; i += 4) {
            int val = *(int*)(MEMORY + i);
            printf("[%04d] = %d\n", i, val);
        }
    }    

    else{
        if(mode == 2)
            puts("Abort");
        return 0;
    }
}
/* print disk.dat and return 0-false(somesing went wrong) 1-true(succses)*/
int diskread(AmmSHFlags mode){
    char* obs_path2 = amm_malloc(100);
    char* obs_path = amm_malloc(100);

    getcwd(obs_path, 100);
    getcwd(obs_path2, 100);

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

    char buf[200];
    sprintf(buf, "Memory/disk.dat");

    FILE *fl = fopen(buf, "rb");
    if (!fl){
      if (mode == 1) puts("Faild reading the disk");
      return 0;
    } 

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


void inline cut_after_substr(char *path, const char *substr) { // bro! inline is a good item use it!
    char *pos = strstr(path, substr);
    if (pos) {
        pos += strlen(substr);
        *pos = '\0';
    }
}



// now functions for os

    	 
int AmmIDE(AmmSHFlags mode){
    
    char* obs_path2 = (char*)amm_malloc(256);
    char* obs_path = (char*)amm_malloc(256);

    getcwd(obs_path2, 256);
    getcwd(obs_path, 256);

    cut_after_substr(obs_path, "/AmmOS");
    chdir(obs_path);

start:   
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
                if(mode == 2) puts("Open disk fail!");
                return 0;
            }
            removen(buff, 5);
            
            int num = atoi(buff);
            if(num > 256 || num < 0){
                if(mode == 2){
                    puts("Please enter value <0-256>");
                    goto start;
                }
            }
            unsigned char c = (char)num;
            fputc(c, fl);
            
            printf("AmmIDE: pushing '%c', (%d) to memory.\n", c, c);
            fclose(fl);
        }
        else if (strncmp(commads[1], buff, 4) == 0){
            char temp[200];
            sprintf(temp, "Memory/disk.dat");

            FILE *fl = fopen(temp, "rb");

            if(!fl){
                if(mode == 2) puts("Open disk fail!");
                return 0;
            }

            fseek(fl, 0, SEEK_END);
            long size = ftell(fl);
            fclose(fl);

            if (size > 0)
               truncate(temp, size-1); 
        }
        else if (strncmp(commads[2], buff, 4) == 0){
            diskread(mode);
        }

        else if (strcmp("exit", buff) == 0){
	       chdir(obs_path2); // I alweys come back;
           printf("\n");
           amm_free(obs_path, 256);
           amm_free(obs_path2, 256);
           return 1;
        }

        else{
            if(mode == 2) printf("AmmIDE: no command found!\n");
        } 

                
    }
    amm_free(obs_path, 256);
    amm_free(obs_path2, 256);
}


int neofetch(){

    printf("\033[1;33m        ==AmmOS==\033[0m\n\n");
    printf("\033[1;37m       /\\\033[0m          \033[;31mKernel:\033[0m Ammkernel \n");
    printf("\033[1;37m      /  \\\033[0m         \033[;31mShell:\033[0m AmmSH (not bash) \n");
    printf("\033[1;37m     /    \\\033[0m        \033[;31mVersion:\033[0m 0.8 \n");
    printf("\033[1;37m    /======\\\033[0m       \033[;31mFS:\033[0m AmmFS (calls Linux)\n");
    printf("\033[1;37m   / Amm-OS \\\033[0m      \033[;31mMemory:\033[0m 27.8kb\n");
    printf("\033[1;37m  /==========\\\033[0m     \033[;31mGPU:\033[0m Ammgpu (2d arr)\n");
    printf("\033[1;37m /    ____    \\\033[0m    \033[;31mRAM:\033[0m disk.dat\n");
    printf("\033[1;37m/____/    \\____\\\033[0m  \033[;32m Flags:\033[0m -n, -s, -b  \n");	

    return 1;
}


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


void replace(char *str, char target, char value){ // не ну а че
    for(int i=0; str[i] != '\0'; ++i){
        if(str[i] == target){
            str[i] = value;
        }
    }
    return; 
}

char *catstr(char *s1, char *s2) {
    int len1 = 0, len2 = 0;
    while (s1[len1]) len1++;
    while (s2[len2]) len2++;
    char *res = amm_malloc(len1 + len2 + 1);
    for (int i = 0; i < len1; i++) res[i] = s1[i];
    for (int j = 0; j < len2; j++) res[len1 + j] = s2[j];
    res[len1 + len2] = '\0';
    return res;
}

// I made this bro for .asm
void* funcs[] = {   // total 28 functions 
    amm_malloc, amm_free, username, str_ascii,  
        
    ascii_int, int_ascii, ascii_str, diskread, 
      
    AmmIDE, removen, mkfile, mkdir_cmd, 
        
    cd_cmd, up_path, ls_cmd, sizeinfo,
        
    cat_cmd, neofetch, AmmSH, get_username,
        
    echo_cmd ,memload, rm_cmd, rf_cmd,
    
    kprint, catstr, ret_int, KERNEL_PANIC
    
};


void TaskOn(const char *file_to_inter, AmmSHFlags mode){
    AmmSH(file_to_inter, mode);
}

typedef struct {
    void (*TaskOn)(const char *file_to_inter, AmmSHFlags mode); // pointer to function
} AmmTask;


int init_sys(void){
    amm_init(); // init memory
    signal(SIGSEGV, sigsegv_handler); // segfalt handler

    chdir(FS_ROOT);
    up_path(); 
}



void *cli(void* HelloWorld){
    // cli
    char *user = username();

    for(;;) {   // I am old Unix man 
     
        char command[256];
        printf("[%s]AmmOS %s$: ", user, path);

        fgets(command, sizeof(command), stdin);
        clean_line(command);

        AmmSH_execute(command, NULL); // CLI mode
        
     
    }
}


int main(void){     // finaly Ammkernel will be a program!
    unsigned long t1;
    init_sys();

    pthread_create(&t1, NULL, cli, NULL);
    



    pthread_join(t1, NULL);

    return 1;
}
