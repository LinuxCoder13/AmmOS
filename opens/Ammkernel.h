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



#pragma ones // это база

#include <stdint.h> // база
#include <stddef.h>

#define VGA_WIDTH 70
#define VGA_HEIGHT 90
#define MAX_PATH 256
#define FS_ROOT "opens/user/main"

#define MEMSIZE (256 * 1024)  // = 262144 байта
#define BLOCK_SIZE 64 // 8 byte memory
#define BLOCK_COUNT (MEMSIZE / BLOCK_SIZE)

#define MAX_DEMONS 12

#define MAX_VARS 70

extern char path[MAX_PATH];
extern unsigned char *MEMORY; // amm_malloc(), amm_free() 
extern short bit_map[MEMSIZE];  // 1, 0

extern char VGA[VGA_HEIGHT][VGA_WIDTH]; // 80x50

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
        char s[256]; 
    };
    char varname[16];   // please don't make long name!
} Var;


// go to .bss
extern AmmDemon demons[MAX_DEMONS];
extern int Ammdemon_count;

extern void infodemon(AmmDemon *demon);
extern void startdemon(AmmDemon *demon);
extern int AmmINI(char* file_to_inter, AmmDemon *demon);
extern int killdemon(AmmDemon *demon);
extern void savedemon(AmmDemon *demon);

extern int bitmapload(AmmSHFlags mode);

extern void kprint(char* text);
extern char* username();
extern void str_ascii(char *str, int *arr);
extern int ascii_int(char c);
extern void int_ascii(long integer, char* buffer); 
extern void ascii_str(int *arr, int sizearr, char *out);
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

extern int AmmSH(const char *file_to_inter, AmmSHFlags);
extern int AmmSH_execute(char *line, int *col);
extern int printf_var(Var var, int type);
extern int variable_initialization(char* name, char* value, int type);
extern Var vars[MAX_VARS];
extern int var_count;

extern void removetab(char *str);
extern char* get_username(AmmSHFlags mode);
extern int echo_cmd(char *msg, int *loop_index, Var* var, int type);


// Memory-alloc funcs
extern void* amm_malloc(int __size);
extern void amm_free(void* ptr, int size);
extern void amm_init();
void** TwoDappend(int *len, int *cap, void **arr, void* value);
void TwoDfree(char **arr, int len);
char* append(int *len, int *cap, char* oldarr, char value);

extern int memload(AmmSHFlags mode);
extern int rm_cmd(char* dirname, AmmSHFlags mode);
extern int rf_cmd(char* filename, AmmSHFlags mode);
extern void KERNEL_PANIC();
extern void sigsegv_handler(int signum);
extern char *catstr(char* s1, char* s2);
extern int ret_int(char* str);
extern void clean_line(char *line);
extern int cat_expand(char *path, AmmSHFlags flag, AmmOpFunc func, char* type);
extern char* get_folder_from_path(char* path);
extern char* cut_suffix(char* path);
extern int endsWith(char* folder, char* sufix);
char* strlchr(const char* s, char c);
char **ls_conter(char* dire, int* out_i, AmmSHFlags mode, char* type);
char* grep_cmd(char* flag, char* dir, char* filename, char* _s, AmmSHFlags mode);


// VGA
extern void vga_main();
extern void vga_init();
extern int ParseAndExecute(char *inst, int height, int width, char c);

// It's end ... I wona Hardcore so I will continue write functions in .asm (./asm/*)

extern void calc(void);
extern void fib(void);


extern void* funcs[];   // total 29 functions 
