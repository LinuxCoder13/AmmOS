#include "Ammkernel.h" 
#include <stdio.h>     
#include <string.h>


char VGA[VGA_WIDTH][VGA_HEIGHT];


int ParseAndExecute(char *inst, int height, int width, char c){
    if(strcmp(inst, "mov") == 0){
        VGA[height][width] = c;  // lol
        return 1;
    }
    return 0;
}

void vga_init(){

    for(int i=0; i<VGA_HEIGHT; ++i){
        for(int j=0; j<VGA_WIDTH; ++j){
            VGA[i][j] = ' ';
        }
        printf("\n");
    }    
}

void vga_main(){
    vga_init();

    while(1){
        printf("\033[2J\033[H");
        for(int i=0; i<VGA_HEIGHT; ++i){
            for(int j=0; j<VGA_WIDTH; ++j){
                printf("%c", VGA[i][j]);
            }
            printf("\n");
        }

        char buff[30];
        printf("-> ");
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = 0;

        char inst[4];
        int height, width;
        char c;

        int res = sscanf(buff, "%3s %d %d %c", inst, &height, &width, &c);
        if(res == 4){
            if(height < 0 || width < 0 || height >= VGA_HEIGHT || width >= VGA_WIDTH){
                puts("what are you trying to do?");
            }
            else{
                if(!ParseAndExecute(inst, height, width, c)){
                    puts("something went wrong");
                }
            }
        }
        else if(strcmp(buff, "exit") == 0){
            return;
        }
        else {
            puts("command not found!");
        }
    }
}
