/*
 * AmmOS - Minimal Modular Operating System
 * Copyright (C) 2025 Ammar Najafli
 * Copyright (C) 2026 Ayano4ka1338
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

#ifndef KERNEL_H
#define KERNEL_H

/*
 * Заголовочний файл ядра – містить оголошення функцій, константи та типи.
 * Здесь объявлены все основные функции для роботи з VGA та клавіатурою.
 */

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

/* VGA buffer address – η διεύθυνση όπου ξεκινά η μνήμη οθόνης. */
#define VGA ((uint8_t*)0xB8000)

/* Screen dimensions – 80 columns, 25 rows, yaar! */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

/* VGA colors (4-bit foreground/background) – रंग कोड (4-बिट फोरग्राउंड/बैकग्राउंड) */
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

/* Глобальний покажчик на поточну позицію у відеобуфері. */
extern volatile uint8_t *p;

/* Port I/O functions – чтение/запись в порты ввода-вывода. */
extern inline uint8_t inb(uint16_t port);
extern inline void outb(uint16_t port, uint8_t value);

/* Output functions – виведення символів, рядків, очищення екрану. */
void kputc(char c);
void kprint(const char* msg) __attribute__((__nonnull__(1)));
void kputs(const char *s)   __attribute__((__nonnull__(1)));
void kcls(void);
void kbackspace(void);
void scroll_up(void);

/* Formatted output – όπως το printf, αλλά για το δικό μας σύστημα. */
void kprintf(const char *fmt, ...) __attribute__((__nonnull__(1)));

/* Read a line from keyboard (with backspace support), na? */
void readline(char *buffer, int max_len);

/* Keyboard poll – returns ASCII code or 0 if no key. */
uint8_t kkeyboard_poll(void);

#endif
