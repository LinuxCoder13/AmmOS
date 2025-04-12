#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "Ammkernel.h"

#define MAX_PATH 256

char path[MAX_PATH];

void mkfile(char *filename){
   FILE *fl = fopen(filename, "w");
   if (!fl){
      perror("NONE\n");
   }
   fclose();
}

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
     // if(chdir(dirname) == 0){
      chdir("opens/user/home");   // must have
      if(chdir(dirname) != 0){ // гарантирую что не выйдет за граници FS
          printf("AmmSH: No such derictory.\n");
          return;
      }
}

// void ls_cmd();

