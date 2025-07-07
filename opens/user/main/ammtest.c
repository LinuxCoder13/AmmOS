#include "/home/user/.local/ANG/alibc/unistd.h"
#include "/home/user/.local/ANG/alibc/string.h"

int _start(void){
    char* buff = "Hello from ANG!\n";
    char* buff2 = "Hello from ANG!\n";

    if(strncmp(buff, buff2, 5) == 0){
        write(1, "hello, world\n", 14);
    }
    else{
        write(1, "goodbuy, world\n", 16);
    }

    _exit(0);
}