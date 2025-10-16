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
extern volatile uint8_t *p;

extern inline uint8_t inb(uint16_t port);
extern inline void outb(uint16_t port, uint8_t value);

extern uint8_t kkeyboard_poll(void);

#define VGA ((uint8_t*)0xB8000) // VGA text buffer base address

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// VGA text mode colors (4-bit, background/foreground)
typedef enum {
    BLACK        = 0x0,  
    BLUE         = 0x1,  
    GREEN        = 0x2,  
    CYAN         = 0x3, 
    RED          = 0x4, 
    MAGENTA      = 0x5,
    BROWN        = 0x6,  
    LIGHT_GRAY   = 0x7,  
    DARK_GRAY    = 0x8,  
    LIGHT_BLUE   = 0x9,  
    LIGHT_GREEN  = 0xA,  
    LIGHT_CYAN   = 0xB,  
    LIGHT_RED    = 0xC,  
    LIGHT_MAGENTA= 0xD,  
    YELLOW       = 0xE,  
    WHITE        = 0xF   
} VGAColor;



// print to VGA msg
void kprint(const char* msg)
    __attribute__((__nonnull__(1)));

// put c to VGA
void kputc(char c);

// clear the screen
void kcls(void);

// print to VGA s, end=\n
void kputs(const char *s)
    __attribute__((__nonnull__(1)));

// delete the last char in VGA
void kbackspace();
