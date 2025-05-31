#include <stdio.h>
#include <stdlib.h>
#include <string.h> // нужно для strcmp()
#include "Ammkernel.h"
#include <locale.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>


int main(void) {
    amm_init(); // init memory
    signal(SIGSEGV, sigsegv_handler); // segfalt handler

    chdir(FS_ROOT);
    up_path();

    char *user = username();

    for(;;) {   // I am old Unix man 
        char command[128];

        printf("[%s]AmmOS %s$: ", user, path);

        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        AmmSH_execute(command, NULL); // CLI mode
        
        
    }
    return 0;
}
