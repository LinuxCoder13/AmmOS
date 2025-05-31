#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include "Ammkernel.h"
#include <signal.h>

char path[MAX_PATH];


int up_path(){
     char *mainpwd = amm_malloc(1024);   // кушаю твой память чтоб не втыкал хотя нет я кушаю свою память. блин :(
     getcwd(mainpwd, 1024);

     char *ospwd = strstr(mainpwd, "/main");
      if (ospwd == NULL){
          printf("AmmSH: permision denied. You can't eascape AmmFS\n");
          amm_free(mainpwd, 1024);
          return 0;
     }
      else{
          memset(path, 0, MAX_PATH); // очищаем буфер полностью
          snprintf(path, MAX_PATH, "~%s", ospwd + strlen("/main"));
      }
      amm_free(mainpwd, 1024); // чють не забыл
      return 1;
  }

int mkfile(char *filename){
   
   FILE *fl = fopen(filename, "w");
   if (fl == NULL){
    perror("AmmSH");
    return 0;
    }
   fclose(fl);
   return 1;
}

int mkdir_cmd(char *dirname){

    struct stat str;

    if(stat(dirname, &str) == 0 && S_ISDIR(str.st_mode)){
         return 1;
    }
    
    if (mkdir(dirname, 0775) == 0){
         return 0;
    }
    else{
        perror("AmmSH");
        return 0;
    }
    return 1;
 }

int cd_cmd(char *dirname){
        char old[256], nw[256];
        getcwd(old, sizeof(old));
 
        if(chdir(dirname) != 0){
          printf("AmmSH: no such folder.\n");
          return 0;
        }
 
        getcwd(nw, sizeof(nw));
        if(!strstr(nw, "/main")){
          chdir(old);
          printf("AmmSH: Access denied. Stay inside AmmFS.\n");
          return 0;
        }
 
        up_path();
        return 1;
 }

int ls_cmd(){
     DIR *d = opendir(".");
     struct dirent *dir;
     struct stat st;
     printf("\n");

     while((dir = readdir(d)) != NULL){
        if(dir->d_name[0] == '.') continue;
        if(S_ISDIR(st.st_mode)) printf("%s  \n", dir->d_name);
        else{
            printf("%s  ", dir->d_name);
     }
        
     }
     printf("\n");
     closedir(d);
     return 1;
}

int sizeinfo(char *filename){
    struct stat info;  // вся инфа о filename

    if(stat(filename, &info) == 0){
        printf("f size: %ld bytes\n", info.st_size);
    }
    else{
        printf("AmmSH: No such file or dir.\n");
        return 0;
    }
    return 1;   
}

int cat_cmd(char *filename){
    struct stat info;
    
    if(stat(filename, &info) != 0){
       printf("AmmSH: I didn't find '%s' go hell\n", filename);
       return 0;
    }

    if(stat(filename, &info) == 0){
        if(S_ISDIR(info.st_mode)){
            printf("AmmSH: That's a papka\n");
            return 0;
        }
        
            FILE *fl = fopen(filename, "r");
            if(fl == NULL){
                perror("AmmSH");
                return 0;
            }
            char *tmp = amm_malloc(1024);    //  надеюсь хватит
            if (!tmp){
                perror("malloc fail");
                fclose(fl);
                return 0;
            }                            
            while((fgets(tmp, 1024, fl)) != NULL){
                printf("%s", tmp);

            }

         amm_free(tmp, 1024); // мусор.  
         fclose(fl);
   }
   return 1;
}

int echo_cmd(char *msg){
    if(msg == NULL){
        return 0;
    }
    printf("%s\n", msg);
    return 1;
}
// <-|-_-|->

int rm_cmd(char* dirname) {
    struct dirent *dir;
    struct stat st;
    char* now_path = amm_malloc(469);
    DIR *d = opendir(dirname);

    if (!d) {
        perror("opendir");
        amm_free(now_path, 469);
        return 0;
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.')
            continue;

        snprintf(now_path, 469, "%s/%s", dirname, dir->d_name);

        if (stat(now_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                rm_cmd(now_path); 
            } else {
                remove(now_path); 
            }
        }
    }

    closedir(d);
    rmdir(dirname); 
    amm_free(now_path, 469);
    return 1;
}


int rf_cmd(char* filename){
    
    if (remove(filename) == 0){
        return 1;
    }

    else{
        fprintf(stderr, "AmmSH: No such fl or dir\n");
        return 0;
    }
}



