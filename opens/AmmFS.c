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

int cd_cmd(char *dirname, AmmSHFlags mode){
        char old[256], nw[256];
        getcwd(old, sizeof(old));
 
        if(chdir(dirname) != 0){
          if (mode == NORMAL) printf("AmmSH: no such folder.\n");
          return 0;
        }
 
        getcwd(nw, sizeof(nw));
        if(!strstr(nw, "/main")){
          chdir(old);
          if (mode == NORMAL) printf("AmmSH: Access denied. Stay inside AmmFS.\n");
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

char* strlchr(const char* s, char c) {
    char* last = NULL;
    while (*s) {
        if (*s == c) last = (char*)s;
        s++;

    }
    return last;
}

int endsWith(char* folder, char* sufix){
    if(folder == NULL || sufix == NULL) return 0;
    int folderlen = strlen(folder), sufixlen = strlen(sufix);
    return (folderlen < sufixlen) ? 0 : (strcmp(folder + folderlen - sufixlen, sufix) == 0);
}

// search '.' and cut arter it and return suffix name
char* cut_suffix(char* path){

    char* dotp = strlchr(path, '.'); // we will newer use free()
    if(dotp == NULL) return NULL;

    int len = strlen(dotp);
    char* suffix = (char*)amm_malloc(len + 1); // +1 for '\0'
    
    strncpy(suffix, dotp, len);
    suffix[len] = 0;
    
    return suffix;
}

//  "path/to/dir/*" -> "path/to/dir"
//  "path/to/file/*.txt" -> "path/to/file/(ALL FILES WITH .txt ENDING)" -> return
char *get_folder_from_path(char *path) {
    char *star = strchr(path, '*');
    
    if (star == NULL) return NULL;
    int len = star - path; // from index 0 to index before '*' char
    
    if (len == 0) {  
        char *dot = amm_malloc(2);
        dot[0] = '.';
        dot[1] = '\0';
        return dot;
    
    }
    
    if (path[len - 1] == '/') len--; // на всякий случай
    char *folder = amm_malloc(len + 1);
    strncpy(folder, path, len);
    folder[len] = '\0';


    return folder;
}

char **ls_conter(char* dire, int* out_i, int mode, char* type){         // build-in function
    DIR *d = opendir(dire);
    if(d == NULL){
        if(mode == NORMAL) perror("opendir failed");
        return NULL;
    }

    struct dirent *dir;
    struct stat st;

    char **res = NULL;
    int cap = 0, len = 0;
    *out_i = 0;



    while((dir = readdir(d)) != NULL){
 
        if(dir->d_name[0] == '.') continue;
        
        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dire, dir->d_name);
        if (stat(fullpath, &st) != 0) continue;

        if ((strcmp(type, "FILE") == 0 && !S_ISREG(st.st_mode)) ||
            (strcmp(type, "DIR") == 0 && !S_ISDIR(st.st_mode))) {
            continue;
        }

        int namelen = strlen(dir->d_name) + 1;       // +1 for '\0'
        char* dirname = (char*)amm_malloc(namelen);  
        strncpy(dirname, dir->d_name, namelen);

        res = (char**)TwoDappend(&len, &cap, (void**)res, dirname);
        (*out_i)++; // count number of valid entries
  

    }


    closedir(d);
    return res;
}

int sizeinfo(char *filename, AmmSHFlags mode){
    struct stat info;  // вся инфа о filename

    if(stat(filename, &info) == 0){
        printf("f size: %ld bytes\n", info.st_size);
    }
    else{
        if(mode == NORMAL) printf("AmmSH: No such file or dir.\n");
        return 0;
    }
    return 1;   
}

int cat_cmd(char *filename, AmmSHFlags mode){
    struct stat info;
    
    if(stat(filename, &info) != 0){
       if (mode == NORMAL) printf("AmmSH: I didn't find '%s', no such file\n", filename);
       return 0;
    }

    if(stat(filename, &info) == 0){
        if(S_ISDIR(info.st_mode)){
            if (mode == NORMAL) printf("AmmSH: That's a papka\n");
            return 0;
        }
        
            FILE *fl = fopen(filename, "r");
            if(fl == NULL){
                if (mode == NORMAL) perror("AmmSH");
                return 0;
            }
            char *tmp = amm_malloc(1024);    //  надеюсь хватит
            if (!tmp){
                if (mode == NORMAL) perror("Amm_malloc failed");
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

// call function for each file, dir in path
int cat_expand(char *path, AmmSHFlags flag, AmmOpFunc func, char* type) {

    char *star = strchr(path, '*');
    if (!star) {
        return func(path, flag);
    }

    // 1) cut path before '*'
    char *folder = get_folder_from_path(path);
    if (!folder) return 0;
    
    // 2) get suffix of file 
    char *suffix = cut_suffix(path);
    if (!suffix) suffix = "";  // нет суффикса = принимаем все


    // 3) read all files/dirs in folder 
    int count;
    char **files = ls_conter(folder, &count, flag, type);
    if (files == NULL) {
        if(flag == NORMAL) perror("opendir failed");
        amm_free(folder, strlen(folder) + 1);
        return 0;
    }

    // 6) do it whith all objects in folder
    for (int i = 0; i < count; i++) {
        // 4) try to get every file whith given sufix
        if (!strstr(files[i], suffix)) continue;

        char fullpath[256]; 
        // 5) merger path and file whith targeted sufix
        snprintf(fullpath, sizeof(fullpath), "%s/%s", folder, files[i]);
        func(fullpath, flag);
    }

    // 7) free all becouse we don't need this info eny more
    TwoDfree(files, count);
    amm_free(folder, strlen(folder) + 1);
    return 1;
}





int echo_cmd(char *msg, int *loop_index){


    if(msg == NULL){
        return 0;
    }

    if(loop_index == NULL){ // cli mode
        printf("%s\n", msg);
        return 1;
    } 
    
    for (int i = 0; msg[i]; i++){ // bro want's to use index of loop? ok.
        if (msg[i] == '^') {
            printf("%d", *loop_index);
            continue;
        }
        putchar(msg[i]);
    }

    putchar('\n');
    return 1;
}
// <-|-_-|->

int rm_cmd(char* dirname, AmmSHFlags mode) {
    struct dirent *dir;
    struct stat st;
    char* now_path = amm_malloc(469);
    DIR *d = opendir(dirname);

    if (!d) {
        if (mode == NORMAL) perror("opendir");
        amm_free(now_path, 469);
        return 0;
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.')
            continue;

        snprintf(now_path, 469, "%s/%s", dirname, dir->d_name);

        if (stat(now_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                rm_cmd(now_path, mode); 
            } else {
                rf_cmd(now_path, mode); 
            }
        }
    }

    closedir(d);
    rmdir(dirname); 
    amm_free(now_path, 469);
    return 1;
}


int rf_cmd(char* filename, AmmSHFlags mode){
    
    if (remove(filename) == 0){
        return 1;
    }

    else{
        if (mode == NORMAL) fprintf(stderr, "AmmSH: No such fl or dir\n");
        return 0;
    }
}



