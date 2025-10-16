/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 *
 * This file is part of AmmOS.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include "kernel.h"

volatile uint8_t *p = VGA;

inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static const char scancode_table[128] = {
    0,27,'1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
    'z','x','c','v','b','n','m',',','.','/',0,'*',0,' ',

    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};


static const char scancode_table_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};


static inline void update_cursor(uint16_t pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void kprint(const char* msg){
    while(*msg) kputc(*msg++);
}

void kputc(char c){
    if( c == '\n' ){
        p += (VGA_WIDTH * 2) - ((p - VGA) % (VGA_WIDTH * 2));
        return;
    }
    if( c == '\t'){
        for(int i=0; i<4; ++i){ 
            *p++ = ' '; 
            *p++ = LIGHT_GRAY; 
        }
        return;
    }
    *p++ = c;    
    *p++ = LIGHT_GRAY;
}

void kputs(const char *s){
    while(*s) kputc(*s++);
    kputc('\n');
}


void kcls(void){
    uint8_t *q = VGA;
    while(q < VGA + VGA_WIDTH * VGA_HEIGHT * 2){
        *q++ = ' ';
        *q++ = BLACK;
    }
    p = VGA;
}

void inline kbackspace(void) {
    if (p > VGA) {
        p -= 2;           
        *p = ' ';          
        *(p + 1) = LIGHT_GRAY;
        uint16_t pos = (p - VGA) / 2;
    }
}


uint8_t kkeyboard_poll(void) {
    static int idle_counter = 0;
    static uint8_t shift = 0;
    static uint8_t ctrl  = 0;
    static uint8_t alt   = 0;
    static uint8_t caps  = 0;
    uint8_t c = 0;

    if (inb(0x64) & 1) {
        uint8_t sc = inb(0x60);
        idle_counter = 0;

        if (sc == 0x2A || sc == 0x36) shift = 1;
        else if (sc == 0xAA || sc == 0xB6) shift = 0;
        else if (sc == 0x1D) ctrl = 1;
        else if (sc == 0x9D) ctrl = 0;
        else if (sc == 0x38) alt = 1;
        else if (sc == 0xB8) alt = 0;
        else if (sc == 0x3A) caps ^= 1;

        if (!(sc & 0x80) && sc < 128) {
            c = shift ? scancode_table_shift[sc] : scancode_table[sc];
            if (caps && c >= 'a' && c <= 'z') c -= 32;
            if (ctrl && c >= 'a' && c <= 'z') c = c - 'a' + 1;

            if (c && c != '\b') kputc(c);
            else if (c == '\b') kbackspace();

            update_cursor((p - VGA) / 2);
        }
    }
    else idle_counter++;

    /* спи моя радость усни :( */
    if      (idle_counter > 1000)for (volatile int i = 0; i < 100000000; i++) __asm__ __volatile__ ("pause");
    else if (idle_counter > 100)for (volatile int i = 0; i < 10000000; i++) __asm__ __volatile__ ("pause");   
    else if (idle_counter > 10) for (volatile int i = 0; i < 1000000; i++) __asm__ __volatile__ ("pause");
    else                        for (volatile int i = 0; i < 100000; i++) __asm__ __volatile__ ("pause");
    

    return c;
}





void kmain(void) {
    kcls();
    kprint("Amm2OS: ");
    
    while (1) {
        kkeyboard_poll();
    }
}
