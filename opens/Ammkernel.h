#ifndef HELLOWORLD
#define HELLOWORLD 1



#define MAX_PATH 256

#define FS_ROOT "opens/user/main"
extern char path[MAX_PATH];

char* username();
void str_ascii(char *str, int *arr);
int ascii_int(char c);
int int_ascii(int integer);
void ascii_str(int *arr, int sizearr, char *out);
int memload(); 
int AmmIDE();
void removen(char *str, int n);
int mkfile(char *filename);
int mkdir_cmd(char *dirname);
int cd_cmd(char *dirname);
int up_path();
int ls_cmd();
int sizeinfo(char *filename);
int cat_cmd(char *filename);
int neofetch();
void AmmSH();
int get_username();
int echo_cmd(char *msg);

#endif
