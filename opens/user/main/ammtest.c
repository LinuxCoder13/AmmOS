#include "/home/user/.local/ANG/alibc/unistd.h"
#include "/home/user/.local/ANG/alibc/string.h"

int _start(void){
    char buff[] = "Hello from ANG!\n";

    write(1, buff, 17);
    _exit(0);
}