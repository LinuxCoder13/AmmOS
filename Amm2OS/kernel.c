#include <stdint.h>

// Hardware definitions
#define VGA ((volatile char*)0xB8000)
#define KEYBOARD 0x60
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define INPUT_BUFFER_SIZE 256
#define NULL (void*)0

// PIC ports
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

// Keyboard controller ports
#define KBC_STATUS 0x64

// IDT and interrupt related
#define IDT_ENTRIES 256
#define IRQ0 32
#define IRQ1 33

// Struct for IDT entry
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

// Struct for IDT pointer
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Interrupt frame (simplified)
struct interrupt_frame {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
};

// Global variables
static uint32_t cursor = 0;
static char input_buffer[INPUT_BUFFER_SIZE];
static uint32_t input_buffer_pos = 0;
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

// Scancode to ASCII conversion table (US QWERTY layout)
static const char scancode_to_char[] = {
    '\0', '\0', '1', '2', '3', '4', '5', '6',  // 0x00-0x07
    '7', '8', '9', '0', '-', '=', '\b', '\t',   // Backspace, Tab
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',     // 0x10-0x17
    'o', 'p', '[', ']', '\n', '\0', 'a', 's',   // Enter, Left Ctrl
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     // 0x20-0x27
    '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',  // Left Shift
    'b', 'n', 'm', ',', '.', '/', '\0', '\0',   // 0x30-0x37
    '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0' // Space, CapsLock
};

// Output byte to port
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Input byte from port
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Set IDT gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// Install IDT
void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;
    
    // Clear out the entire IDT
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);
    
    // Load IDT
    __asm__ volatile ("lidt %0" : : "m"(idtp));
}

// Remap PIC interrupts
void remap_pic() {
    // ICW1 - initialization
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    
    // ICW2 - remap offset
    outb(PIC1_DATA, IRQ0);    // IRQ 0-7 -> INT 32-39
    outb(PIC2_DATA, IRQ0+8);  // IRQ 8-15 -> INT 40-47
    
    // ICW3 - cascade
    outb(PIC1_DATA, 0x04);    // Slave at IRQ2
    outb(PIC2_DATA, 0x02);
    
    // ICW4 - environment info
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    
    // Mask all interrupts except keyboard (IRQ1)
    outb(PIC1_DATA, 0xFD);
    outb(PIC2_DATA, 0xFF);
}

// Clear screen
void clear_screen() {
    for (uint32_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA[i * 2] = ' ';
        VGA[i * 2 + 1] = 0x07;
    }
    cursor = 0;
}

// Print character
void kputc(char c) {
    if (cursor >= VGA_WIDTH * VGA_HEIGHT) {
        cursor = 0;
    }
    
    // Handle newline
    if (c == '\n') {
        cursor = ((cursor / VGA_WIDTH) + 1) * VGA_WIDTH;
        return;
    }
    
    // Handle backspace
    if (c == '\b') {
        if (cursor > 0) {
            cursor--;
            VGA[cursor * 2] = ' ';
        }
        return;
    }
    
    VGA[cursor * 2] = c;
    VGA[cursor * 2 + 1] = 0x07;
    cursor++;
}

// Print string
void kprint(const char* msg) {
    while (*msg) {
        kputc(*msg++);
    }
}

// Simple memset implementation
void memset(void* ptr, int value, int num) {
    uint8_t* p = (uint8_t*)ptr;
    for (int i = 0; i < num; i++) {
        p[i] = (uint8_t)value;
    }
}

// Keyboard interrupt handler
void __attribute__((interrupt)) keyboard_int_handler(struct interrupt_frame* frame) {
    uint8_t status = inb(KBC_STATUS);
    
    // Check if keyboard has data ready
    if (status & 0x01) {
        uint8_t scancode = inb(KEYBOARD);
        
        // Key release - ignore
        if (scancode & 0x80) {
            outb(PIC1_CMD, PIC_EOI);
            return;
        }
        
        // Handle Ctrl+C
        if (scancode == 0x2E) {  // 'C' key
            kprint("\n[KERNEL PANIC] Ctrl+C pressed!\n");
            while (1) __asm__("hlt");
        }
        
        // Convert scancode to ASCII
        char c = (scancode < sizeof(scancode_to_char)) ? scancode_to_char[scancode] : '\0';
        
        // Handle backspace
        if (c == '\b') {
            if (input_buffer_pos > 0) {
                input_buffer_pos--;
                kputc('\b');
            }
            outb(PIC1_CMD, PIC_EOI);
            return;
        }
        
        // Store character in buffer if there's space
        if (c && input_buffer_pos < INPUT_BUFFER_SIZE - 1) {
            input_buffer[input_buffer_pos++] = c;
            kputc(c);
        }
    }
    
    // Send EOI to PIC
    outb(PIC1_CMD, PIC_EOI);
}

// Initialize keyboard
void keyboard_init() {
    // Set up keyboard interrupt handler
    idt_set_gate(IRQ1, (uint32_t)keyboard_int_handler, 0x08, 0x8E);
    
    // Enable interrupts
    __asm__ volatile ("sti");
}

// Blocking input
char* kgets() {
    input_buffer_pos = 0;
    while (1) {
        if (input_buffer_pos > 0 && input_buffer[input_buffer_pos - 1] == '\n') {
            input_buffer[input_buffer_pos - 1] = '\0';
            return input_buffer;
        }
        __asm__("hlt");
    }
}

// Non-blocking input
char* kgets_nonblock() {
    if (input_buffer_pos > 0 && input_buffer[input_buffer_pos - 1] == '\n') {
        input_buffer[input_buffer_pos - 1] = '\0';
        input_buffer_pos = 0;
        return input_buffer;
    }
    return NULL;
}

// Kernel main function
void kmain() {
    clear_screen();
    
    // Initialize IDT and PIC
    idt_install();
    remap_pic();
    keyboard_init();
    
    kprint("Keyboard input test:\n");
    kprint("Type something and press Enter (Ctrl+C to quit):\n");
    
    // Test blocking input
    char* input = kgets();
    kprint("\nYou typed: ");
    kprint(input);
    
    // Test non-blocking input
    kprint("\nNow type more (non-blocking mode):\n");
    while (1) {
        char* tmp = kgets_nonblock();
        if (tmp) {
            kprint("\n> ");
            kprint(tmp);
        }
        __asm__("hlt");
    }
}