/*
 * DISCLAIMER
 *
 * AmmOS is an educational and experimental operating system.
 * It is provided as-is, with no guarantees that it will function
 * correctly or safely in any environment.
 *
 * The author is not responsible for any damage, data loss, or
 * other issues that may result from using or modifying this software.
 *
 * Use at your own risk.
 */


/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 *
 * This file is part of AmmOS.
 *
 * AmmOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AmmOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AmmOS.  If not, see <https://www.gnu.org/licenses/>.
 */


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
     char mainpwd[1024];
     getcwd(mainpwd, 1024);

     char *ospwd = strstr(mainpwd, "/main");
      if (ospwd == NULL){
          printf("AmmOS: FATAL: check the struct of progect -> AmmOS/opens/user/main\n");
          return 0;
     }
      else{
          snprintf(path, MAX_PATH, "~%s", ospwd + strlen("/main"));
      }
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

int ls_cmd() {
    DIR *d = opendir(".");
    struct dirent *dir;
    struct stat st;
    char fpath[512];  // полный путь для stat

    if (!d) {
        perror("opendir");
        return 0;
    }

    printf("\n");
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;

        snprintf(fpath, sizeof(fpath), "./%s", dir->d_name);  // построим путь
        if (stat(fpath, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode))
            printf("%s/   ", dir->d_name);
        else
            printf("%s  ", dir->d_name);
    }

    printf("\n");
    closedir(d);
    return 1;
}


char* strlchr(const char* s, char c) {
    char* last = NULL;
    while (*s != '\0') {
        if (*s == c) last = (char*)s;
        s++;

    }
    return last;
}

int endsWith(char* folder, char* sufix){
    if(folder == NULL || sufix == NULL) return 0;
    int folderlen = strlen(folder), sufixlen = strlen(sufix);
    return (folderlen < sufixlen) ? 0 : (astrcmp(folder + folderlen - sufixlen, sufix) == 0); // okay?
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
char *get_folder_from_path(char *path, char x) {
    char *star = strlchr(path, x);
    
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

char **ls_conter(char* dire, int* out_i, AmmSHFlags mode, char* type){         // build-in function
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


        if ((astrcmp(type, "FILE") == 0 && !S_ISREG(st.st_mode)) ||
            (astrcmp(type, "DIR") == 0 && !S_ISDIR(st.st_mode)))  {
            continue;
        }

        res = (char**)TwoDappend(&len, &cap, (void**)res, dir->d_name);
        
        if(res == NULL){
            printf("fall in ls_counter()\n");
            return NULL;
        }
        (*out_i)++; // count number of valid entries
  

    }


    closedir(d);
    return res;
}

// version 2.5 
char* grep_cmd(char* flag, char* dir, char* filename, char* _s, AmmSHFlags mode) {
    DIR *d = opendir(dir);
    if (!d) return NULL;

    struct dirent *de;
    struct stat st;
    char fullpath[512];

    while ((de = readdir(d))) {
        if (astrcmp(de->d_name, ".") == 0 || astrcmp(de->d_name, "..") == 0) continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, de->d_name);
        if (stat(fullpath, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            char *res = grep_cmd(flag, fullpath, filename, _s, mode);
            if (res) {
                closedir(d);
                return res;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (astrcmp(de->d_name, filename) == 0) {
                if (astrcmp(flag, "-r-file") == 0) {
                    closedir(d);
                    char *copy = amm_malloc(strlen(fullpath) + 1);
                
                    strcpy(copy, fullpath);
                    return copy;
                }

                else if (astrcmp(flag, "-r-str") == 0) {
                    FILE *fp = fopen(fullpath, "r");
                    if (!fp) continue;

                    char line[512];
                    unsigned long line_count = 0;
                    int found = 0;
                    while (fgets(line, sizeof(line), fp)) {
                        line_count++;
                        if (strstr(line, _s)) {
                            found = 1;
                            break;
                        }
                    }
                    fclose(fp);

                    if (found) {
                        closedir(d);
                        char *copy = amm_malloc(strlen(fullpath) + 1);
                    
                        snprintf(copy, strlen(fullpath)+1, "fist conditions met on line \"%ld\"\n", line_count);
                        return copy;
                    }
                }
                else if(!astrcmp(flag, "-r-file-str")){
                    FILE *fp = fopen(fullpath, "r");
                    if (!fp) continue;

                    char line[512];
                    unsigned long line_count = 0;
                    int found = 0;
                    while (fgets(line, sizeof(line), fp)) {
                        line_count++;
                        if (strstr(line, _s)) {
                            found = 1;
                            break;
                        }
                    }
                    fclose(fp);

                    if (found) {
                        closedir(d);
                        int needed = snprintf(NULL, 0, "first conditions met on line \"%lu\" \nPath:%s", line_count, fullpath);
                        char *copy = amm_malloc(needed + 1);
                        snprintf(copy, needed + 1, "first conditions met on line \"%lu\" \nPath:%s", line_count, fullpath);
                        return copy;
                    }                    
                }
            }
        }
    }
    closedir(d);
    return NULL;
}


int sizeinfo(char *filename, AmmSHFlags mode){
    struct stat info;  // вся инфа о filename

    if(stat(filename, &info) == 0){
        fprintf(stdout, "f size: %ld bytes\n", info.st_size);
    }
    else{
        if(mode == NORMAL) printf("AmmSH: No such file or dir.\n");
        return 0;
    }
    return 1;   
}

int cat_cmd(char *filename, AmmSHFlags mode){
    struct stat info;
    char tmp[1024];

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
            
            if (!tmp){
                if (mode == NORMAL) perror("Amm_malloc failed");
                fclose(fl);
                return 0;
            }                            
            while((fgets(tmp, 1024, fl)) != NULL){
                printf("%s", tmp);

            }
  
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
    char *folder = get_folder_from_path(path, '*');
    if (!folder) return 0;
    
    // 2) get suffix of file 
    char *suffix = cut_suffix(path);
    if (!suffix) suffix = "";  // нет суффикса = принимаем все


    // 3) read all files/dirs in folder 
    int count;
    char **files = ls_conter(folder, &count, flag, type);
    if (files == NULL) {
        if(flag == NORMAL) perror("opendir failed");
        amm_free(folder);
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
    amm_free(folder);
    if (suffix && suffix[0] != '\0'){ 
        amm_free(suffix);
    }
    return 1;
}


int echo_cmd(char *msg){

    for(int i=0; msg[i] != 0; ++i){
        putchar(msg[i]);
    }

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
        amm_free(now_path);
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
    amm_free(now_path);
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
    return (mode == BACKGROUND) ? 1 : 0;
}



