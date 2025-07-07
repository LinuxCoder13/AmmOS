/*
 * DISCLAIMER
 *
 * AmmOS is an educational and experimental operating system.
 * It is provided as-is, with no guarantees that it will function
 * correctly or safely in any environment.
 *
 * The author is not responsible for any damage, data loss, or
 * other issues that may result from using or modifying this software.
 *
 * Use at your own risk.
 */



/* Ammkernel - is main function-database it send tecnology to AmmFS, Ammshell, ect.
the prototype of functions, variables(extern), makros are in Ammkernel.h 
microkernel life (2025 - ...)*/

/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 *
 * This file is part of AmmOS.
 *
 * AmmOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AmmOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmmOS.  If not, see <https://www.gnu.org/licenses/>.
 */


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
    

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif


static uint8_t bit_map[BLOCK_COUNT]; // amm_malloc(), amm_free()
static uint8_t *MEMORY;  // 1, 0

// go to .bss
DictEntry aunicode[MAX_ENTRIES];
int aunicode_count = 0;


// functions that will help write kernel

void amm_init(void) {
    MEMORY = mmap(NULL, MEMSIZE, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (MEMORY == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    memset(MEMORY, 0, MEMSIZE);
    memset(bit_map, 0, sizeof(bit_map));
}

// I a sorry for this shit
void *amm_malloc(int __size) {
    if (__size <= 0) {
        return NULL;
    }

    int blocks_needed = (__size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int consec = 0;

    for (int i = 0; i < BLOCK_COUNT; ++i) {
        if (!bit_map[i]) {
            consec++;
            if (consec == blocks_needed) {
                int start = i - blocks_needed + 1;
                for (int j = start; j <= i; ++j) {
                    bit_map[j] = 1;
                }
                return MEMORY + (start * BLOCK_SIZE);
            }
        } else {
            consec = 0;  
        }
    }

    return NULL;
}


void amm_free(void *ptr, int __size) {
    if (!ptr || __size <= 0) return;
    intptr_t offset = (uint8_t*)ptr - MEMORY;
    if (offset < 0 || offset >= MEMSIZE) return;

    int start = offset / BLOCK_SIZE;
    int blocks = (__size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (int i = start; i < start + blocks && i < BLOCK_COUNT; ++i) {
        bit_map[i] = 0;
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
    if (*cap == 0) {                 
        *cap = 4;
        arr = amm_malloc(*cap * sizeof(void*));
        *len = 0;                 
    }

    if (*len >= *cap) {
        int new_cap = *cap * 2;
        void **new_arr = amm_malloc(new_cap * sizeof(void*));
        if (new_arr == NULL) printf("IDI NAHUY\n");
        for (int i = 0; i < *len; ++i) {
            new_arr[i] = arr[i];
        }
        amm_free(arr, *cap * sizeof(void*));
        arr = new_arr;
        *cap = new_cap;
    }

    int sz = strlen((char*)value) + 1;

    char* strcopy = amm_malloc(sz);
    if(strcopy == NULL){
        printf("FATAL: MEMORY FULL\n");
        return NULL;
    }
    memcpy(strcopy, value, sz);

    arr[*len] = strcopy;
    (*len)++;

    return arr;
}


void TwoDfree(char **arr, int count) {
    if (!arr) return;
    // сначала освобождаем каждую строку
    for (int i = 0; i < count; ++i) {
        if (arr[i]) {
            amm_free(arr[i], strlen(arr[i]) + 1);
        }
    }
    // затем сам указатель на массив
    amm_free(arr, count * sizeof(char *));
}

void dict_set(char key, char* value){
    // sorry for O(n)
    for(int i=0; i<aunicode_count; i++){
        if(aunicode[i].key == key){
            strncpy(aunicode[i].value, value, MAX_VALUE_LEN);
            return;
        }
    }
    // but it's O(1) btw
    if(aunicode_count < MAX_ENTRIES){
        aunicode[aunicode_count].key = key;
        strncpy(aunicode[aunicode_count].value, value, MAX_VALUE_LEN);
        aunicode_count++;
        return;
    }
}


char* dict_get(char key) {
    for (int i = 0; i < aunicode_count; i++) {
        if (aunicode[i].key == key) {
            return aunicode[i].value;
        }
    }
    return NULL;
}
    
void aencrypt(char* inputfile, char* outfile){
    // I/O
    FILE *fl = fopen(inputfile, "r");
    FILE *of = fopen(outfile, "w");
    if (!fl || !of) {
        if (fl) fclose(fl);
        if (of) fclose(of);
        return;
    }


    char c;
    char* str;
    while((c = fgetc(fl)) != EOF){
        str = dict_get(c);
        if(!str) fputs("_______", of); // unknow char
        else fputs(str, of);
    }


    fclose(fl);
    fclose(of);
}

void adecrypt(char* targetfile, char* outfile){
    FILE *fl = fopen(targetfile, "r");
    FILE *of = fopen(outfile, "w");
    if (!fl || !of) {
        if (fl) fclose(fl);
        if (of) fclose(of);
        return;
    }



    char buff[MAX_VALUE_LEN + 1];
    while (fread(buff, 1, MAX_VALUE_LEN, fl) == MAX_VALUE_LEN) {
        buff[MAX_VALUE_LEN] = '\0'; 

        // Ищем, кто такой
        char found = '?';
        for (int i = 0; i < aunicode_count; i++) {
            if (strncmp(aunicode[i].value, buff, MAX_VALUE_LEN) == 0) {
                found = aunicode[i].key; 
                break;
            }
        }
        putc(found, of); 
    }

    fclose(fl);
    fclose(of);
}


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
        printf("Hello, Welcome to AmmOS!\nAmmOS is using GPLv3 (C) \nPlease write your username to continue: ");
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



int memload(AmmSHFlags mode){
    write(1, "load memory int or char or hex or value?(i, c, h, v): ", 55); // syscall lol. rdi, rdx, rsi. if I am not mistake. 
    unsigned char res;
    scanf("%c", &res);  // I am sorry
     
    while(getchar() != '\n');
     
    if(res == 'i'){
        for(int i=0; i<BLOCK_COUNT-29; ++i){
            printf("%d, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'c'){
        for(int i=0; i<BLOCK_COUNT-29; ++i){
            printf("%c, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }
    else if(res == 'h'){
        for(int i=0; i<BLOCK_COUNT-29; i++){
            printf("%#x, ", (unsigned char)MEMORY[i]);
        }
        return 1;
    }

    else if (res == 'v') {
        for (int i = 0; i < BLOCK_COUNT - 3; i += 4) {
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

int bitmapload(AmmSHFlags mode){
    for(int i=0; i<BLOCK_COUNT; ++i){
        printf("%hd, ", bit_map[i]);
    }
    printf("\n");
    return 1;
}

/* print disk.dat and return 0-false(somesing went wrong) 1-true(succses)*/
int diskread(AmmSHFlags mode){
    char obs_path2[100];
    char obs_path[100];

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
           return 1;
        }

        else{
            if(mode == 2) printf("AmmIDE: no command found!\n");
        } 

                
    }
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
    printf("              \\                        \n");
    printf("          X    \\                       \n");
    printf("                \\                      \n");            
    printf("                                       \n");
    printf("Please restart AmmOS                   \033[0m\n");
    
    fflush(stdout);
    exit(7);
}


void sigsegv_handler(int signum) {
    KERNEL_PANIC();
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


char* astrdup(char* p){
    int len = strlen(p);
    char* new = (char*)amm_malloc(len+1); // + 1 for '\0'

    int i=0;
    while(i<len){
        new[i] = p[i];
        i++;
    }
    new[len] = '\0';
    return new;
}


// I made this bro for .asm
void* funcs[] = {   // total 28 functions 
    amm_malloc, amm_free, username, str_ascii,  
        
    ascii_int, int_ascii, NULL, diskread,  // I will fix this I promise! 
      
    AmmIDE, removen, mkfile, mkdir_cmd, 
        
    cd_cmd, up_path, ls_cmd, sizeinfo,
        
    cat_cmd, neofetch, AmmSH, get_username,
        
    echo_cmd ,memload, rm_cmd, rf_cmd,
    
    puts, catstr, atoi, KERNEL_PANIC
    
};





int init_sys(void){
    amm_init(); // init memory
    signal(SIGSEGV, sigsegv_handler); // segfalt handler
    
    // init main pid
    demons[0].pid=getpid();
    demons[0].apid=Ammdemon_count;
    strncpy(demons[0].name, "MAIN", 5); // +1 for '\0'
    strcpy(demons[0].execfilename, __FILE__);
    strcpy(demons[0].comment, "It's the main proses in AmmOS");
    Ammdemon_count++;

    // AmmDemons
    for(int i=1; i<MAX_DEMONS; i++){
        demons[i].apid = i;
        demons[i].name[0] = '\0';
        demons[i].execfilename[0] = '\0';
        demons[i].comment[0] = '\0';
    }
    signal(SIGCHLD, SIG_IGN);

    // lower-case a-z
    dict_set('a', "*lk3@f!"); dict_set('b', "^vZ@01x"); dict_set('c', "qq@!!s0"); dict_set('d', "x1p@z#9"); dict_set('e', "&L2@jkz"); dict_set('f', "mkL!@02");
    dict_set('i', "sS@#12z"); dict_set('j', "z@vP!!0"); dict_set('k', "99x@#Qm"); dict_set('l', "Kk@lq2!"); dict_set('m', "z!@sL00"); dict_set('n', "88@vMzz");
    dict_set('o', "@@1x2!q"); dict_set('p', "lLz@x33"); dict_set('q', "#@pQvv0"); dict_set('r', "0sLz@!9"); dict_set('s', "!@s9xLk"); dict_set('t', "pq@!23z");
    dict_set('u', "xx@Lp01"); dict_set('v', "@@z1!lk"); dict_set('w', "mMz@q82"); dict_set('x', "!q@xKk9"); dict_set('y', "ZZ@pl01"); dict_set('z', "vV!@29z");
    dict_set('g', "!!3z@Lp"); dict_set('h', "pp@*8ls");
    
    // uper-case A-Z
    dict_set('A', "$euf*vb"); dict_set("B", "I*Vu*vei");
    dict_set('C', "@29dj#kL"); dict_set('D', "m&*sdf83"); dict_set('E', "7^bV@e!!"); dict_set('F', "^^9ds@lm"); dict_set('G', "*sk#mv03"); dict_set('H', "lp@#9xZ&");
    dict_set('I', "z!83@klq"); dict_set('J', "Uw#1!mkd"); dict_set('K', "00&xve@q"); dict_set('L', "rr*8@Xjw"); dict_set('M', "k!!ss&33"); dict_set('N', "^z*0Ls1@");
    dict_set('O', "pP@x3*vl"); dict_set('P', "d^@z8&Ui"); dict_set('Q', "++9ksj@1"); dict_set('R', "iZz@!qkL"); dict_set('S', "!x#p02LM"); dict_set('T', "vbL@zZ&&");
    dict_set('U', "m#@23!pQ"); dict_set('V', "u*ssP09&"); dict_set('W', "qp09@kl#"); dict_set('X', "0@1!Lsdf"); dict_set('Y', "^&lkj908"); dict_set('Z', "zZ@kLx1^");

    // digits 0-9
    dict_set('0', "0@v!Zl9");
    dict_set('1', "1!@LxZz"); dict_set('2', "2@xLp00"); dict_set('3', "3@Pp!!q");
    dict_set('4', "4!z@kLx"); dict_set('5', "5@L#zz2"); dict_set('6', "6@xLmQ1");
    dict_set('7', "7@v!PQz"); dict_set('8', "8@pQlz!"); dict_set('9', "9!@zLm1");

    // basic symbols
    dict_set(' ', "_@xLp__");   
    dict_set('.', ".@!Kz90");
    dict_set(',', ",@L#29q");
    dict_set('!', "!@9xLpq");
    dict_set('?', "?@Lp00z");
    dict_set('\'',"[@zL!88");
    dict_set('\\',"uT@QpKz");

    // fs
    chdir(FS_ROOT);
    up_path(); 
    return 1;
}



void *cli(void* HelloWorld){
    // cli
    char *user = username();


    for(;;) {   // I am old Unix man 
     
        char command[256];
        printf("[%s]AmmOS %s$: ", user, path);

        fgets(command, sizeof(command), stdin);
        clean_line(command);

        AmmSH_execute(command);
        
     
    }
}


int main() {
    init_sys();
    pthread_t cli_thread;
    pthread_create(&cli_thread, NULL, cli, NULL);
    

    while (1) sleep(1);

}
