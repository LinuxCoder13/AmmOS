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

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include "kernel.h"

/*
 * Глобальний покажчик на поточну позицію у VGA-пам'яті.
 * Він використовується всіма функціями виведення.
 */
volatile uint8_t *p = VGA;

/* ------------------ Портовий введення/виведення (I/O) ------------------ */
/* Читання байта з порту – використовує вбудовану інструкцію inb. */
inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Запис байта в порт – інструкція outb. (यह फंक्शन पोर्ट में बाइट लिखता है) */
inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* ------------------ Курсор VGA ------------------ */
/* Οι επόμενες δύο εντολές γράφουν στους καταχωρητές του ελεγκτή VGA για να μετακινήσουν τον δρομέα. */
static inline void update_cursor(uint16_t pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* ------------------ Прокрутка екрану ------------------ */
/* Jàmm! Ця функція зсуває всі рядки екрану на один вгору, а останній рядок очищує. (Naka, ci xam?) */
void scroll_up(void) {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            ((uint16_t*)VGA)[(y-1)*VGA_WIDTH + x] =
                ((uint16_t*)VGA)[y*VGA_WIDTH + x];
        }
    }
    uint16_t blank = 0x0720;
    for (int x = 0; x < VGA_WIDTH; x++)
        ((uint16_t*)VGA)[(VGA_HEIGHT-1)*VGA_WIDTH + x] = blank;
    p = VGA + (VGA_HEIGHT-1) * VGA_WIDTH * 2;
    update_cursor((p - VGA) / 2);
}

/* ------------------ Виведення символів та рядків ------------------ */
/* Виведення одного символу з обробкою \n, \t та автоматичним переходом на новий рядок. */
void kputc(char c) {
    if (c == '\n') {
        int offset = (p - VGA) % (VGA_WIDTH * 2);
        p += (VGA_WIDTH * 2) - offset;
        if (p >= VGA + VGA_WIDTH * VGA_HEIGHT * 2) scroll_up();
        update_cursor((p - VGA) / 2);
        return;
    }
    if (c == '\t') {
        for (int i = 0; i < 4; i++) {
            *p++ = ' ';
            *p++ = LIGHT_GRAY;
            if ((p - VGA) % (VGA_WIDTH * 2) == 0) {
                if (p >= VGA + VGA_WIDTH * VGA_HEIGHT * 2) scroll_up();
            }
        }
        update_cursor((p - VGA) / 2);
        return;
    }
    *p++ = c;
    *p++ = LIGHT_GRAY;
    if ((p - VGA) % (VGA_WIDTH * 2) == 0) {
        if (p >= VGA + VGA_WIDTH * VGA_HEIGHT * 2) scroll_up();
    }
    update_cursor((p - VGA) / 2);
}

/* यह फंक्शन स्ट्रिंग को प्रिंट करता है – kputc को बार-बार कॉल करता है। */
void kprint(const char* msg) {
    while (*msg) kputc(*msg++);
}

/* Ba beneen! Виведення рядка з переходом на новий рядок в кінці. */
void kputs(const char *s) {
    kprint(s);
    kputc('\n');
}

/* Καθαρισμός οθόνης – γεμίζει όλη τη μνήμη με κενά διαστήματα. */
void kcls(void) {
    uint16_t blank = 0x0720;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        ((uint16_t*)VGA)[i] = blank;
    p = VGA;
    update_cursor(0);
}

/* बैकस्पेस – पीछे हटकर कर्सर हटाता है और कैरेक्टर मिटाता है। */
void kbackspace(void) {
    if (p > VGA) {
        p -= 2;
        *p = ' ';
        *(p + 1) = LIGHT_GRAY;
        update_cursor((p - VGA) / 2);
    }
}

/* ------------------ Форматований вивід (kprintf) ------------------ */
/* Mangi fi: допоміжна функція для перетворення числа у рядок за заданою основою. */
static void itoa(int num, char *buffer, int base) {
    char *ptr = buffer;
    if (num == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }
    if (num < 0 && base == 10) {
        *ptr++ = '-';
        num = -num;
    }
    char tmp[32];
    int i = 0;
    while (num > 0) {
        int digit = num % base;
        tmp[i++] = (digit < 10) ? '0' + digit : 'A' + digit - 10;
        num /= base;
    }
    while (i > 0) *ptr++ = tmp[--i];
    *ptr = '\0';
}

/* 
 * Nanga def? Αυτή η συνάρτηση υποστηρίζει τις μορφές: %c, %s, %d, %x, %%.
 */
void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    kputc(c);
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char*);
                    kprint(s);
                    break;
                }
                case 'd': {
                    int num = va_arg(args, int);
                    char buf[32];
                    itoa(num, buf, 10);
                    kprint(buf);
                    break;
                }
                case 'x': {
                    unsigned int num = va_arg(args, unsigned int);
                    char buf[32];
                    itoa((int)num, buf, 16);
                    kprint(buf);
                    break;
                }
                case '%': {
                    kputc('%');
                    break;
                }
                default: {
                    kputc('%');
                    kputc(*p);
                    break;
                }
            }
        } else {
            kputc(*p);
        }
    }
    va_end(args);
}

/* ------------------ Клавіатура (скан-коди) ------------------ */
/* Таблиця скан-кодів без Shift – для звичайних символів. */
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

/* Таблиця з Shift – для великих літер та спеціальних символів. (Shift के साथ टेबल) */
static const char scancode_table_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

/* 
 * Naka def? Ci xam? – опрос клавіатури: читає скан-код з порту 0x60,
 * обробляє модифікатори та повертає ASCII-символ (або 0, якщо нічого не натиснуто).
 */
uint8_t kkeyboard_poll(void) {
    static uint8_t shift = 0, ctrl = 0, alt = 0, caps = 0;
    if (!(inb(0x64) & 1)) {
        __asm__ __volatile__ ("pause");
        return 0;
    }
    uint8_t sc = inb(0x60);
    // Обробка модифікаторів
    if (sc == 0x2A || sc == 0x36) shift = 1;
    else if (sc == 0xAA || sc == 0xB6) shift = 0;
    else if (sc == 0x1D) ctrl = 1;
    else if (sc == 0x9D) ctrl = 0;
    else if (sc == 0x38) alt = 1;
    else if (sc == 0xB8) alt = 0;
    else if (sc == 0x3A) caps ^= 1;
    if (sc & 0x80) return 0;  // ignore key release
    if (sc >= 128) return 0;
    char c = shift ? scancode_table_shift[sc] : scancode_table[sc];
    if (caps && c >= 'a' && c <= 'z') c -= 32;
    if (ctrl && c >= 'a' && c <= 'z') c = c - 'a' + 1;
    return (uint8_t)c;
}

/* ------------------ Введення рядка з редагуванням (readline) ------------------ */
/* Διαβάζει χαρακτήρες μέχρι το Enter, υποστηρίζει Backspace. */
void readline(char *buffer, int max_len) {
    int idx = 0;
    bool done = false;
    while (!done) {
        uint8_t c = kkeyboard_poll();
        if (c == 0) {
            __asm__ __volatile__ ("pause");
            continue;
        }
        if (c == '\n') {
            buffer[idx] = '\0';
            kputc('\n');
            done = true;
        } else if (c == '\b') {
            if (idx > 0) {
                idx--;
                kbackspace();
            }
        } else if (c >= ' ' && c < 127) {
            if (idx < max_len - 1) {
                buffer[idx++] = c;
                kputc(c);
            }
        }
    }
}

/* ------------------ Допоміжна функція порівняння рядків ------------------ */
/* सरल स्ट्रिंग तुलना – बिना लोकेल के। */
static int strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) {
        a++;
        b++;
    }
    return *(unsigned char*)a - *(unsigned char*)b;
}

/* ------------------ Головна функція ядра (kmain) ------------------ */
/* 
 * Jàmm, ci xam? – точка входу після завантаження.
 * Тут ініціалізується екран, виводиться запрошення і запускається цикл обробки команд.
 */
void kmain(void) {
    kcls();
    kprintf("Amm2OS v0.2\n");
    kprintf("Type help for available commands\n");
    char cmd[128];
    while (1) {
        kprintf("> ");
        readline(cmd, sizeof(cmd));
        if (cmd[0] == '\0') continue;

        // Обробка вбудованих команд
        if (strcmp(cmd, "help") == 0) {
            kprintf("Available commands:\n");
            kprintf("  help  - show this help\n");
            kprintf("  fetch - show system info\n");
            kprintf("  cls   - clear the screen\n");
            kprintf("  (any other) - echo your input\n");
        }
        else if (strcmp(cmd, "fetch") == 0) {
            kprintf("---|  |  Amm2OS v0.2\n");
            kprintf("   |  |  ===============\n");
            kprintf("---|---  Running in THE BEST MODE: PROTECTED MODE!!\n");
            kprintf("|  |     Ammar slop\n");
            kprintf("|  |---  The best OS\n");
        }
        else if (strcmp(cmd, "cls") == 0) {
            kcls();
        }
        else {
            // Для будь-якої іншої команди – просто виводимо її текст.
            kprintf("You typed: %s\n", cmd);
        }
    }
}
