#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "Ammkernel.h"

#define MAX_PATH 256

char *path[MAX_PATH];

void mkdir_cmd(char *dirname){
   if (mkdir(dirname, 0775) == 0){
        return;
   }
   else{
        perror("AmmSH: ERROR\n");
        return;
   }
}

void cd_cmd(char *dirname){
    if(chdir(dirname) == 0){



    }
}


