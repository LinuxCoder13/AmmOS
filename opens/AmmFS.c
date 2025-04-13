#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "Ammkernel.h"


char path[MAX_PATH];

 void up_path(){
     char *mainpwd = malloc(sizeof(char) * 1024);   // кушаю твой память

     getcwd(mainpwd, 1024);
     char *ospwd = strstr(mainpwd, "/main");
      if (ospwd == NULL){
          printf("AmmSH: permision denied. You can't eascape AmmFS\n");
          free(mainpwd);
         return;
     }
     else{
       snprintf(path, MAX_PATH,"~%s", ospwd + strlen("/main"));
      }
      free(mainpwd); // чють не забыл
  }

void mkfile(char *filename){
   FILE *fl = fopen(filename, "w");
   if (!fl){
      perror("AmmSH\n");
   }
   fclose();
}

void mkdir_cmd(char *dirname){
    struct stat str;

    if(stat(dirname, &str) == 0 && S_ISDIR(str.st_mode)){
         printf("AmmSH: folder '%s' already exists.\n", dirname);
         return;
    }

    if (mkdir(dirname, 0775) == 0){
         return;
    }
    else{
         perror("AmmSH\n");
        return;
    }
 }

void cd_cmd(char *dirname){
        char old[256], nw[256];
        getcwd(old, sizeof(old));
 
        if(chdir(dirname) != 0){
          printf("AmmSH: no such folder.\n");
          return;
        }
 
        getcwd(nw, sizeof(nw));
        if(!strstr(nw, "/main")){
          chdir(old);
          printf("AmmSH: Access denied. Stay inside AmmFS.\n");
        }
 
        up_path();
 }

void ls_cmd(){
     DIR *d = opendir(".");
     struct dirent *dir;
     struct stat st;
     printf("\n");

     while((dir = readdir(d)) != NULL){
     if(dir->d_name[0] == '.') continue;
     if(S_ISDIR(st.st_mode)) printf("\003[1;34m%s\033[0m  \n", dir->d_name);
     else{
         printf("%s  \n", dir->d_name);
     }

     }
  }

// void ls_cmd();

