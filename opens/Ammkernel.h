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



#pragma ones // это база

#include <stdint.h> // база
#include <stddef.h>
#include <stdatomic.h>

#define VGA_WIDTH 70
#define VGA_HEIGHT 90
#define MAX_PATH 256
#define FS_ROOT "opens/user/main"

#define STAK_START 0x400
#define STAK_END   0x7FF

#define HEAP_START 0x800
extern uint8_t *sp;
extern uint8_t *heap;

#define MEMSIZE (1024 * 64)        
#define BLOCK_SIZE 1               
#define BLOCK_COUNT (MEMSIZE / BLOCK_SIZE)

extern uint8_t bit_map_alloc[BLOCK_COUNT]; 
extern uint8_t *MEMORY;
extern unsigned long amm_malloc_count;
extern unsigned long amm_free_count;

#define MAX_DEMONS 12
#define MAX_VARS 70

extern char path[MAX_PATH];

extern char VGA[VGA_HEIGHT][VGA_WIDTH]; // 90x70

// lets do flags for AmmSh
typedef enum{
    SILENT = 1,   // all functions in AmmOS will not tell about erorr or warnings 
    NORMAL = 2,    // clasic mode
    BACKGROUND = 3
    // soon
}AmmSHFlags;

typedef int (*AmmOpFunc)(char *path, AmmSHFlags mode);

typedef unsigned short Ammpid_t;

typedef struct {
    int pid;      // real pid in linux
    Ammpid_t apid;  // local pid 
    char name[40];  // demon name
    char execfilename[50];  // demon which execute file
    char comment[80];

} AmmDemon;

typedef struct Var {
    enum {INT = 1, CHAR = 2, STRING = 3} type;
    union { 
        int i;
        char c; 
        char s[26]; 
    };
    char varname[16];   // please don't make long name!
} Var;


// go to .bss
extern AmmDemon demons[MAX_DEMONS];
extern int Ammdemon_count;

#define MAX_ENTRIES 100
#define MAX_VALUE_LEN 64

typedef struct {
    char key;
    char value[MAX_VALUE_LEN];
} DictEntry;

extern DictEntry aunicode[MAX_ENTRIES];
extern int aunicode_count;

void aencrypt(char* inputfile, char* outfile);
void adecrypt(char* targetfile, char* outfile);


extern void dict_set(char key, char* value);
extern char* dict_get(char key);

extern void infodemon(AmmDemon *demon);
extern void startdemon(AmmDemon *demon);
extern int AmmINI(char* file_to_inter, AmmDemon *demon);
extern int killdemon(AmmDemon *demon);
extern void savedemon(AmmDemon *demon);

extern int bitmapload(AmmSHFlags mode);

extern char* username();
extern void change_username(const char* newname);
extern void str_ascii(char *str, int *arr);
extern int ascii_int(char c);
extern void int_ascii(long integer, char* buffer); 
extern int diskread(AmmSHFlags mode); 
extern int AmmIDE(AmmSHFlags mode);
extern void removen(char *str, int n);
extern int mkfile(char *filename);
extern int mkdir_cmd(char *dirname);
extern int cd_cmd(char *dirname, AmmSHFlags mode);
extern int up_path();
extern int ls_cmd();
extern int sizeinfo(char *filename, AmmSHFlags mode);
extern int cat_cmd(char *filename, AmmSHFlags mode);
extern int neofetch();
extern int isin(char *str, char c);
extern int is2arrin(char **str, char *str2);
volatile static unsigned long long uptime = 0;

extern int AmmSH(const char *file_to_inter, AmmSHFlags);
extern int AmmSH_execute(char *line);
extern int printf_var(Var var, int type);
extern int variable_initialization(char* name, char* value, int type);


extern int preprocessor(int argc, char **argv); // IMPORTANT FUCTION!!
extern Var vars[MAX_VARS];
extern int var_count;
extern void vars_dump();

extern void removetab(char *str);
extern char* get_username(AmmSHFlags mode);
extern int echo_cmd(char *msg);


// Memory-alloc funcs
extern void* amm_malloc(int __size);
extern void amm_free(void* ptr);
extern void amm_init();
extern void** TwoDappend(int *len, int *cap, void **arr, void* value);
extern void TwoDfree(char **arr, int len);
extern char* append(int *len, int *cap, char* oldarr, char value);
extern char* astrdup(char* p);

extern void amm_push(void* arr, long n);
extern void* amm_pop(void* dest, long n); 

extern int memload(AmmSHFlags mode);
extern int rm_cmd(char* dirname, AmmSHFlags mode);
extern int rf_cmd(char* filename, AmmSHFlags mode);
extern void KERNEL_PANIC();
extern void sigsegv_handler(int signum);
extern char *catstr(char* s1, char* s2);
extern void clean_line(char *line);
extern int cat_expand(char *path, AmmSHFlags flag, AmmOpFunc func, char* type);
extern char* get_folder_from_path(char* path, char x);
extern char* cut_suffix(char* path);
extern int endsWith(char* folder, char* sufix);
extern char* strlchr(const char* s, char c);
extern char **ls_conter(char* dire, int* out_i, AmmSHFlags mode, char* type);
extern char* grep_cmd(char* flag, char* dir, char* filename, char* _s, AmmSHFlags mode);
extern void cut_after_substr(char *path, const char *substr);
extern void GodSays();
extern long long parse_proc_status_kb(char *key);
extern long get_memdat_size();
extern long parse_ammMemory_size();
// VGA
extern void vga_main();
extern void vga_init();
extern int ParseAndExecute(char *inst, int height, int width, char c);

// It's end ... I wona Hardcore so I will continue write functions in .asm (./asm/*)

extern void calc(void);
extern void fib(void);
extern void factoral(void);
// non glibc function (made for high speed)
extern int astrcmp(char* _s1, char* _s2);


extern void* funcs[];   // total 28 functions 
