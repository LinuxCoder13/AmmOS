#pragma ones // это база

#include <stdint.h> // база
#include <stddef.h>

#define VGA_WIDTH 70
#define VGA_HEIGHT 90
#define MAX_PATH 256
#define FS_ROOT "opens/user/main"
#define MEMSIZE (8192 / 2)
#define BLOCK_SIZE 64 // 8 byte memory

extern char path[MAX_PATH];
extern char *MEMORY; // amm_malloc(), amm_free() 
extern short bit_map[MEMSIZE];  // 1, 0
extern char VGA[VGA_HEIGHT][VGA_WIDTH]; // 80x50

// lets do flags for AmmSh
typedef enum{
    SILENT = 1,   // all functions in AmmOS will not tell about erorr or warnings 
    NORMAL = 2    // clasic mode
    // soon
}AmmSHFlags;

typedef int (*AmmOpFunc)(char *path, AmmSHFlags mode);


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
extern void removetab(char *str);
extern char* get_username(AmmSHFlags mode);
extern int echo_cmd(char *msg, int *index);


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
char **ls_conter(char* dire, int* out_i, int mode, char* type);

// VGA
extern void vga_main();
extern void vga_init();
extern int ParseAndExecute(char *inst, int height, int width, char c);

// It's end ... I wona Hardcore so I will continue write functions in .asm (./asm/*)

extern void calc(void);
extern void fib(void);


extern void* funcs[];   // total 29 functions 
