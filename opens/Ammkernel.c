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
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
#include <stdatomic.h>
#include <sys/syscall.h>


    
#include <ctype.h>

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

// go to .bss
uint8_t bit_map_alloc[BLOCK_COUNT]; 
uint8_t *MEMORY;
unsigned long amm_free_count = 0;
unsigned long amm_malloc_count = 0;

uint8_t *sp;
uint8_t *heap;


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
    memset(bit_map_alloc, 0, sizeof(bit_map_alloc));
    sp = (uint8_t*)MEMORY + STAK_END;
    heap = (uint8_t*)MEMORY + HEAP_START;
}

void* amm_malloc(int __size) __attribute__((malloc));
// I am sorry for this O(n) shit
void *amm_malloc(int __size) {
    if (__size <= 0) {
        return (void*)0;
    }

    register long blocks_needed __asm__("rax") = (__size + sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    register long consec __asm__("rbx") = 0;

    for (register int i __asm__("rcx") = 0; i < BLOCK_COUNT; ++i) { 
        if (!bit_map_alloc[i]) {
            consec++;
            if (consec == blocks_needed) {
                int start = i - blocks_needed + 1;
                for (register int j __asm__("rdx") = start; j <= i; ++j) bit_map_alloc[j] = 1;

                amm_malloc_count ++;
                int* meta = (int*)((uint8_t*)heap + start * BLOCK_SIZE);
                *meta = __size;
                return (void*)((char*)meta + sizeof(int));
            }
        } 
        else {
            consec = 0;  
        }
    }
    
    return NULL;
}

void amm_free(void *ptr) {
    if (!ptr) return;

    int* meta = (int*)((char*)ptr - sizeof(int));
    int size = *meta; // mov size, [meta] ; lol

    int start = ((char*)meta - (char*)heap) / BLOCK_SIZE;
    int blocks = (size + sizeof(int) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    for (int i = start; i < start + blocks && i < BLOCK_COUNT; ++i) {
        bit_map_alloc[i] = 0;
    }
    amm_free_count++;
}


void amm_push(void* arr, long n){
    register char *_arr __asm__("rdi") = (char*)arr;
    register long _n __asm__("rsi") = n;

    if (sp - (long)_n < (uint8_t*)MEMORY + STAK_END) {
        puts("Stack overflow!"); return;
    }
    
    for(int i=0; i < _n; ++i) *sp-- = *_arr++;
}


void* amm_pop(void* dest, long n) {
    register char* d __asm__("rdi") = (char*)dest;
    register long _n __asm__("rsi") = n;

    if (sp + (long)n > (uint8_t*)MEMORY + STAK_END) {
        puts("Stack underflow!"); 
        return NULL;
    }
    for(long i = 0; i < n; ++i) d[i] = *++sp;
    return dest;
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

        amm_free(oldarr); 
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
        // Allocate space for new_arr[0 .. new_cap - 1], each of type (void *)
        void **new_arr = amm_malloc(new_cap * sizeof(void*));

        for (int i = 0; i < *len; ++i) {
            new_arr[i] = arr[i];
        }
        amm_free(arr);
        arr = new_arr;
        *cap = new_cap;
    }

    int sz = strlen((char*)value) + 1;

    char* strcopy = astrdup(value);
    if(strcopy == NULL){
        printf("FATAL: MEMORY FULL\n");
        return NULL;
    }

    arr[*len] = strcopy;
    (*len)++;

    return arr;
}


void TwoDfree(char **arr, int count) {
    if (!arr) return;
    for (int i = 0; i < count; ++i) {
        if (arr[i]) {
            amm_free(arr[i]);
        }
    }
    amm_free(arr);
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

int isin(char *str, char c){ for(int i=0; *str != '\0'; i++) if((*str + i) == c) return 1; return 0;}
int is2arrin(char **str, char *str2){ for(int i=0; *str != ((void*)0); ++i) if(astrcmp(str[i], str2) == 0) return 1; return 0;}



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
    amm_free(fpath);
    amm_free(fpath2);
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
        printf("%hd, ", bit_map_alloc[i]);
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
inline void removen(char *str, int n){  // bro! inline is a good item use it!
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

void GodSays() {
    srand(time(NULL));  
    int n = rand() % 100;

    switch (n) {
        case 0: puts("God says: Don't touch that."); break;
        case 1: puts("God says: Proceed."); break;
        case 2: puts("God says: Turn off the computer."); break;
        case 3: puts("God says: Reboot and try again."); break;
        case 4: puts("God says: TempleOS is holy."); break;
        case 5: puts("God says: Type carefully."); break;
        case 6: puts("God says: Trust the kernel."); break;
        case 7: puts("God says: Pray before compiling."); break;
        case 8: puts("God says: Thou shalt not segfault."); break;
        case 9: puts("God says: Seek the logs."); break;
        case 10: puts("God says: You are not root."); break;
        case 11: puts("God says: Run as root, but beware."); break;
        case 12: puts("God says: Kill -9 wisely."); break;
        case 13: puts("God says: Read the docs."); break;
        case 14: puts("God says: No printf debugging."); break;
        case 15: puts("God says: Use `make clean`."); break;
        case 16: puts("God says: Memory is sacred."); break;
        case 17: puts("God says: Silence is golden."); break;
        case 18: puts("God says: Check the stack."); break;
        case 19: puts("God says: Avoid undefined behavior."); break;
        case 20: puts("God says: Respect segmentation."); break;
        case 21: puts("God says: Watch your pointers."); break;
        case 22: puts("God says: Don't leak memory."); break;
        case 23: puts("God says: Use amm_malloc."); break;
        case 24: puts("God says: Fork not in vain."); break;
        case 25: puts("God says: There are no coincidences."); break;
        case 26: puts("God says: AmmSH is watching."); break;
        case 27: puts("God says: Avoid infinite loops."); break;
        case 28: puts("God says: Give `resb` what it deserves."); break;
        case 29: puts("God says: Align your soul."); break;
        case 30: puts("God says: The compiler sees all."); break;
        case 31: puts("God says: Delete wisely."); break;
        case 32: puts("God says: Debug is sacred ritual."); break;
        case 33: puts("God says: Commit often."); break;
        case 34: puts("God says: Comments are commandments."); break;
        case 35: puts("God says: Don't trust the scheduler."); break;
        case 36: puts("God says: Return zero."); break;
        case 37: puts("God says: 0xDEADBEEF is real."); break;
        case 38: puts("God says: Shift with purpose."); break;
        case 39: puts("God says: Respect the bootloader."); break;
        case 40: puts("God says: Do not overflow."); break;
        case 41: puts("God says: You shall not core dump."); break;
        case 42: puts("God says: This is the answer."); break;
        case 43: puts("God says: If it compiles, it's divine."); break;
        case 44: puts("God says: RTFM."); break;
        case 45: puts("God says: Assembly is a prayer."); break;
        case 46: puts("God says: Execute, don’t question."); break;
        case 47: puts("God says: Trust in NULL."); break;
        case 48: puts("God says: Break before madness."); break;
        case 49: puts("God says: Watch RIP carefully."); break;
        case 50: puts("God says: Reload AmmOS."); break;
        case 51: puts("God says: Debug with honor."); break;
        case 52: puts("God says: /main is sacred."); break;
        case 53: puts("God says: Use hex, not decimal."); break;
        case 54: puts("God says: Obey the syscall."); break;
        case 55: puts("God says: Never trust user input."); break;
        case 56: puts("God says: Signals are divine messages."); break;
        case 57: puts("God says: Use SIGINT to awaken."); break;
        case 58: puts("God says: Exit gracefully."); break;
        case 59: puts("God says: Obey the kernel log."); break;
        case 60: puts("God says: Write clean code."); break;
        case 61: puts("God says: Worship the boot sector."); break;
        case 62: puts("God says: syscalls are sacred."); break;
        case 63: puts("God says: Be kind to threads."); break;
        case 64: puts("God says: Beware of race conditions."); break;
        case 65: puts("God says: Check return values."); break;
        case 66: puts("God says: One tab, not spaces."); break;
        case 67: puts("God says: Backup before chaos."); break;
        case 68: puts("God says: Rename with purpose."); break;
        case 69: puts("God says: 69 is a lucky syscall."); break;
        case 70: puts("God says: Don't fear segmentation."); break;
        case 71: puts("God says: Respect the binary."); break;
        case 72: puts("God says: Don't touch /dev/null."); break;
        case 73: puts("God says: Worship only AmmOS."); break;
        case 74: puts("God says: Invoke Amm_systemd."); break;
        case 75: puts("God says: Seek AmmDemon wisdom."); break;
        case 76: puts("God says: Protect the .bss."); break;
        case 77: puts("God says: Bytecode is prophecy."); break;
        case 78: puts("God says: Signals are warnings."); break;
        case 79: puts("God says: Use strace to see truth."); break;
        case 80: puts("God says: printf is a miracle."); break;
        case 81: puts("God says: Use asserts to test faith."); break;
        case 82: puts("God says: Use amm_free wisely."); break;
        case 83: puts("God says: Delete with precision."); break;
        case 84: puts("God says: Obey the memory map."); break;
        case 85: puts("God says: mmap is sacred space."); break;
        case 86: puts("God says: Load the truth."); break;
        case 87: puts("God says: Worship the ELF."); break;
        case 88: puts("God says: Don't touch EFLAGS."); break;
        case 89: puts("God says: All bugs are demons."); break;
        case 90: puts("God says: AmmInit is the genesis."); break;
        case 91: puts("God says: Align to 4KB."); break;
        case 92: puts("God says: Interpret with caution."); break;
        case 93: puts("God says: Don't forget NULL."); break;
        case 94: puts("God says: The stack never lies."); break;
        case 95: puts("God says: All loops end eventually."); break;
        case 96: puts("God says: Don't optimize yet."); break;
        case 97: puts("God says: Reboot in doubt."); break;
        case 98: puts("God says: Create, not copy."); break;
        case 99: puts("God says: You are chosen."); break;
    }
}

long long parse_proc_status_kb(char *key) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) return -1;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, strlen(key)) == 0) {
            char *p = strchr(line, ':');
            if (!p) continue;  
            p++; 

            while (isspace(*p)) p++;

            long long value_kb = atoll(p);
            fclose(f);
            return value_kb;
        }
    }

    fclose(f);
    return -1;
}

long parse_ammMemory_size(){
    long result = 0;
    for(int i=0; i<BLOCK_COUNT; i++){
        if(bit_map_alloc[i]) result++;
    }
    return result * BLOCK_SIZE;
}

long get_memdat_size() {
    struct stat st;
    char obspath[256];
    char obspath2[256];

    getcwd(obspath, 256);
    getcwd(obspath2, 256);

    cut_after_substr(obspath, "/AmmOS");
    chdir(obspath);

    if (stat("Memory/disk.dat", &st) == 0){
        long result = (long)st.st_size;
        chdir(obspath2);
        return result;
    }
    return 0;
    chdir(obspath2);
}

// now functions for os

    	 
int AmmIDE(AmmSHFlags mode){
    
    char obs_path2[256];
    char obs_path[256]; 


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
                    puts("Please enter value <0-255>");
                    goto start;
                }
                goto start;
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
            chdir(obs_path2);
            printf("\n");
            return 1;
        }

        else{
            if(mode == 2) printf("AmmIDE: no command found!\n");
        } 
    }
    return 1;
}


int neofetch(){
    struct stat st;
    char obspath[256];
    char obspath2[256];

    getcwd(obspath, 256);
    getcwd(obspath2, 256);

    cut_after_substr(obspath, "/AmmOS");
    chdir(obspath);
    FILE *f = fopen("opens/AmmOS", "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long binary_size = ftell(f);
    short HDD_size = get_memdat_size();

    

    printf("\033[1;33m        ==AmmOS==\033[0m\n\n");
    printf("\033[1;37m       /\\\033[0m          \033[;31mKernel:\033[0m Ammkernel \n");
    printf("\033[1;37m      /  \\\033[0m         \033[;31mShell:\033[0m AmmSH (not bash) \n");
    printf("\033[1;37m     /    \\\033[0m        \033[;31mVersion:\033[0m 0.8 \n");
    printf("\033[1;37m    /======\\\033[0m       \033[;31mFS:\033[0m AmmFS (calls Linux)\n");
    printf("\033[1;37m   / Amm-OS \\\033[0m      \033[;31mSizeof(AmmOS.elf):\033[0m %ld KB\n", binary_size / 1024);
    printf("\033[1;37m  /==========\\\033[0m     \033[;31mGPU:\033[0m Ammgpu (2d arr)\n");
    printf("\033[1;37m /    ____    \\\033[0m    \033[;31mRAM:\033[0m %lld/%lld\n", parse_ammMemory_size(), MEMSIZE);
    printf("\033[1;37m/____/    \\____\\\033[0m  \033[;32m Flags:\033[0m -n, -s, -b  \n");
    printf("                   \033[;31mHDD:\033[0m %lld/4069 %s\n", HDD_size, 
    (HDD_size > 4069) ? "HDD is full!" : "");

    fclose(f);
    chdir(obspath2);
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
void sigint_handler(int signum){
    (void)signum;
    puts("\nAmmSH: please use command \"ex\" to exit");
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


// my ABI
void* funcs[] = {   // total 28 functions 
    amm_malloc, amm_free, username, str_ascii,  
        
    ascii_int, int_ascii, sigint_handler, diskread,   
      
    AmmIDE, removen, mkfile, mkdir_cmd, 
        
    cd_cmd, up_path, ls_cmd, sizeinfo,
        
    cat_cmd, neofetch, AmmSH, get_username,
        
    echo_cmd ,memload, rm_cmd, rf_cmd,
    
    puts, catstr, atoi, KERNEL_PANIC
    
};

int init_sys(void){
    amm_init(); // init memory
    signal(SIGSEGV, sigsegv_handler); // segfalt handler
    signal(SIGINT, sigint_handler);
    
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

    // AmmTimeDemon
    // AmmDemon d;
    // d.apid=Ammdemon_count;
    // if(!AmmINI("TIMEdemon/Time.ammservice", &d)) exit(228); // to check write `echo $?`


    // lower-case a-z
    dict_set('a', "*lk3@f!"); dict_set('b', "^vZ@01x"); dict_set('c', "qq@!!s0"); dict_set('d', "x1p@z#9"); dict_set('e', "&L2@jkz"); dict_set('f', "mkL!@02");
    dict_set('i', "sS@#12z"); dict_set('j', "z@vP!!0"); dict_set('k', "99x@#Qm"); dict_set('l', "Kk@lq2!"); dict_set('m', "z!@sL00"); dict_set('n', "88@vMzz");
    dict_set('o', "@@1x2!q"); dict_set('p', "lLz@x33"); dict_set('q', "#@pQvv0"); dict_set('r', "0sLz@!9"); dict_set('s', "!@s9xLk"); dict_set('t', "pq@!23z");
    dict_set('u', "xx@Lp01"); dict_set('v', "@@z1!lk"); dict_set('w', "mMz@q82"); dict_set('x', "!q@xKk9"); dict_set('y', "ZZ@pl01"); dict_set('z', "vV!@29z");
    dict_set('g', "!!3z@Lp"); dict_set('h', "pp@*8ls");
    
    // uper-case A-Z
    dict_set('A', "$euf*vb"); dict_set('B', "I*Vu*vei");
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
    
    while (1) sleep(0xff);

}
