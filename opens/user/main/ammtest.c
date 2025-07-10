#include "/home/user/.local/ANG/alibc/unistd.h"
#include "/home/user/.local/ANG/alibc/string.h"

int _start(void){
    char buff[100] = "Hello from ANG!\n";
    char buff2[100];
    strncpy(buff2, buff, 13);
    for (int i = 0; i < 17; i++) {
    if (buff2[i] < 32 || buff2[i] > 126) buff2[i] = '.'; // заменим непечатаемые
}
    write(1, buff2, 17);
    _exit(0);
}