#ifndef HELLOWORLD
#define HELLOWORLD
#define MAX_PATH 256

extern path[MAX_PATH];

void str_ascii(char *str, int *arr);
int ascii_int(char c);
int int_ascii(int integer);
void ascii_str(int *arr, int sizearr, char *out);
void memload(); 
void AmmIDE();
void removen(char *str, int n);
void create_f(char *filename);
void mkdir_cmd(char *dirname);
void cd_cmd(char *dirname);

#endif
