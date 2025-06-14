#pragma ones // это база

#include <stdint.h> // база
#include <stddef.h>

#define VGA_WIDTH 50
#define VGA_HEIGHT 70
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
    SILENT = 1,    
    NORMAL = 2    
    // soon
}AmmSHFlags;



extern void kprint(char* text);
extern char* username();
extern void str_ascii(char *str, int *arr);
extern int ascii_int(char c);
extern void int_ascii(long integer, char* buffer); 
extern void ascii_str(int *arr, int sizearr, char *out);
extern int diskread(); 
extern int AmmIDE();
extern void removen(char *str, int n);
extern int mkfile(char *filename);
extern int mkdir_cmd(char *dirname);
extern int cd_cmd(char *dirname);
extern int up_path();
extern int ls_cmd();
extern int sizeinfo(char *filename);
extern int cat_cmd(char *filename);
extern int neofetch();
extern int AmmSH(const char *file_to_inter, AmmSHFlags);
extern int AmmSH_execute(char *line, int *col);
extern void removetab(char *str);
extern char* get_username();
extern int echo_cmd(char *msg);
extern void* amm_malloc(int __size);
extern void amm_free(void* ptr, int size);
extern void amm_init();
extern int memload();
extern int rm_cmd(char* dirname);
extern int rf_cmd(char* filename);
extern void KERNEL_PANIC();
extern void sigsegv_handler(int signum);
extern char *catstr(char* s1, char* s2);
extern int ret_int(char* str);
extern void vga_main();
extern void vga_init();
extern int ParseAndExecute(char *inst, int height, int width, char c);

// It's end ... I wona Hardcore so I will continue write functions in .asm (./asm/lowlevel_Ammkernel.asm)

extern void calc(void);
extern void fib(void);


extern void* funcs[];   // total 29 functions 
