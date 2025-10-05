#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include <time.h>

int main(void) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char username[64] = {0};
    DWORD size = sizeof(username);

    if (!GetUserNameA(username, &size)) {
        FILE *fl = fopen("log.txt", "a");
        fprintf(fl, "GetUserNameA failed (%lu)\n", GetLastError());
        return 1;
    }

    // путь к исполняемому файлу в автозагрузке — без внешних кавычек
    char appPath[MAX_PATH];
    snprintf(appPath, sizeof(appPath),
             "C:\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\qemui286x.exe",
             username);

    for (;;) { // while(1)
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        BOOL ok = CreateProcessA(
            appPath,  
            NULL,       
            NULL,      
            NULL,      
            FALSE,      
            0,       
            NULL,       
            NULL,      
            &si,
            &pi
        );

        if (!ok) {
            FILE *fl = fopen("log.txt", "a");
            if (fl) {
                fprintf(fl, "[%lu] CreateProcess failed for '%s' (err=%lu)\n",
                        (unsigned long)time(NULL), appPath, GetLastError());
                fclose(fl);
            }
            Sleep(1000);
            continue;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);


        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        Sleep(1000);
    }

    // unreachable, но для порядка:
    return 0;
}
