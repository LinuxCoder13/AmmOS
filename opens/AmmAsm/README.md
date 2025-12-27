# AmmAsm - x86-64 Assembler

**Version:** 1.4  
**Author:** Ammar Najafli  
**License:** MIT  

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to machine code and produces **ELF executables** for Linux x86-64.

---

## Features

- **Direct x86-64 encoding** - No NASM/GAS dependencies  
- **Multiple operand sizes** - 8/16/32/64-bit registers and immediates  
- **Memory addressing** - Full SIB/ModRM support with explicit syntax  
- **Label support** - Global and local labels with symbol resolution  
- **Inline literals** - Embed strings and data directly in .text section  
- **Control flow** - `jmp`, `call` with relative addressing  
- **Two-pass linker** - Built-in symbol resolution and relocation  
- **Hex/Binary/Octal literals** - `0xDEADBEEF`, `0b1010`, `0o777`  
- **ELF output** - Generates valid Linux x86-64 executables  
- **Clean error messages** - Line numbers and helpful diagnostics  

---

## What's New in v1.4

### Major Features
- **Dynamic Entry Point** - The linker now searches for the `_start` label to set the ELF entry point. If `_start` is not found, the default entry point `0x401000` is used.

---

## Pipeline Stages

### Lexer (LEXER) - Converts text to tokens
- Recognizes instructions, registers, literals, labels
- Handles comments (`//`, `;`, `/* */`)
- Supports multiple number bases (hex, binary, octal, decimal)
- Label scoping (global `label:`, local `.label:`)
- Character literals (`'A'`, `'\n'`, `'\0'`)

### Parser (PARSE) - Builds Abstract Syntax Tree
- Validates instruction operands
- Resolves operand types (REG/IMM/MEM/LABEL/CHAR)
- Handles label scoping (global/local)
- Creates AST nodes for instructions, labels, and data

### Code Generator (parseInst) - Emits x86-64 machine code
- REX prefix generation for 64-bit operations
- ModR/M and SIB byte encoding for memory addressing
- Displacement and immediate value encoding
- Placeholder generation for unresolved labels

### Linker (collect_labels + resolve_labels)
- **First pass**: Assigns addresses to all labels (PC = 0x401000 + offset)
- **Second pass**: Resolves symbol references
  - `MOV r64, label` -> RELOC_ABS64 (absolute 64-bit address)
  - `JMP/CALL label` -> RELOC_REL32 (relative 32-bit offset)

### Compiler (compiler) - Assembles final binary
- Orchestrates all compilation passes
- Collects machine code from AST
- Writes ELF executable with proper headers

---

## Syntax

### Registers
Prefix registers with `%`:
```asm
mov %rax, 42        ; 64-bit
mov %eax, 42        ; 32-bit  
mov %ax, 42         ; 16-bit
mov %al, 42         ; 8-bit
mov %r8, %r15       ; Extended registers (r8-r15)
```

### Numeric Literals
```asm
mov %rax, 42        ; Decimal
mov %rax, 0xff      ; Hexadecimal
mov %rax, 0b1010    ; Binary
mov %rax, 0o777     ; Octal
mov %rax, -10       ; Negative numbers
mov %al, 'A'        ; Character literal
```

### Data Directives
```asm
; Byte arrays
msg: u8 "Hello, World!", 0x0A, 0
bytes: u8 0x01, 0x02, 0x03, 'A', 'B'

; Word arrays (16-bit)
words: u16 100, 200, 300, 0x1234

; Double word arrays (32-bit)
dwords: u32 0xDEADBEEF, 1000000, 0

; Quad word arrays (64-bit)
qwords: u64 0x123456789ABCDEF0, 999999999
```

### Memory Addressing
```asm
; Format: [b=base, i=index, s=scale, d=displacement]
mov %rax, [b=rbx]                     ; [rbx]
mov %rax, [b=rbx, d=16]               ; [rbx + 16]
mov %rax, [b=rbx, i=rcx, s=8]         ; [rbx + rcx*8]
mov %rax, [b=rbx, i=rcx, s=8, d=0x10] ; [rbx + rcx*8 + 0x10]
mov [b=rsp, d=8], %rax                ; Memory destination
```

### Control Flow
```asm
; Unconditional jumps
jmp label           ; Relative jump (E9 rel32)
jmp %rax            ; Absolute jump via register (FF /4)

; Function calls
call func           ; Relative call (E8 rel32)
call %rbx           ; Indirect call via register (FF /2)

; Loading label addresses
mov %rax, msg       ; Load address of 'msg' into rax
```

### Arithmetic
```asm
add %rax, 42        ; 64-bit
add %eax, 42        ; 32-bit
add %ax, 42         ; 16-bit
add %al, 42         ; 8-bit
```

---

## Example Program N.1 (Hello World)
```asm
_start:
    ; write(1, msg, len)
    mov %rax, 1              ; sys_write
    mov %rdi, 1              ; stdout
    mov %rsi, msg:           ; buffer address
    mov %rdx, 14             ; length
    syscall

    ; exit(0)
    mov %rax, 60             ; sys_exit
    mov %rdi, 0              ; status
    syscall

msg: u8 "Hello, World!", 0x0A
```

## Example Program N.2 (Hello World)
```asm
msg: u8 "Hello, World!\n", 0x0A, 0
_start:
    mov %rcx, -1
    mov %rax, 0
    mov %rdi, msg:

   .a: u8 0xf2, 0xAE       ; repne scasb
   .b: u8 0x48, 0xF7, 0xD1 ; not rcx
   .c: u8 0x48, 0xFF, 0xC9 ; dec rcx

    mov %rax, 1
    mov %rdi, 1
    mov %rsi, msg:
    mov %rdx, %rcx
    syscall

    mov %rax, 60
    mov %rdi, 0
    syscall
```


**Compile and run:**
```bash
./aasm f=hello.asm o=hello
chmod +x hello
./hello
```

---

## Machine Code Example

**Source:**
```asm
_start:
    jmp skip
    mov %rdi, 1
skip:
    mov %rax, 60
    syscall

msg: u8 "Hi", 0
```

**Generated (hexdump):**
```
00001000:  e9 0a 00 00 00              ; jmp +10 (skip label)
00001005:  48 bf 01 00 00 00 00 00 00 00  ; mov rdi, 1
0000100f:  48 b8 3c 00 00 00 00 00 00 00  ; mov rax, 60
00001019:  0f 05                       ; syscall
0000101b:  48 69 00                    ; "Hi\0" (inline literal)
```

**Linker work:**
1. First pass: `skip` at `0x40100F`, `msg` at `0x40101B`
2. Second pass: JMP offset = `0x40100F - (0x401000 + 5) = 0x0A`
3. Patch: `E9 0A 00 00 00`

---

## Design Philosophy

AmmAsm prioritizes:
- **Clarity over brevity** - Explicit `b=rbx` instead of cryptic NASM `[rbx]`
- **Zero dependencies** - Pure C99, no external libraries
- **Educational value** - Readable code for learning x86-64 encoding
- **Simplicity** - Minimal feature set, maximum understanding
- **Hackability** - Clean codebase for experimentation

---

## Known Limitations

- Limited instruction set (MOV, ADD, JMP, CALL, SYSCALL)
- No conditional jumps yet (JE, JNE, JG, etc.) - coming soon
- No multi-file linking
- No macro system
- No floating-point support (FPU)
- No optimization passes (and never will be)
- No .data/.bss sections yet (everything in .text)

---

## Usage
```bash
# Basic compilation
./aasm input.asm

# Specify output file
./aasm f=input.asm o=output

# Alternative syntax
./aasm input.asm o=myprogram
```

**Command-line options:**
- `f=<file>` or `<file>` - Input assembly file
- `o=<file>` - Output executable (default: `a.out`)

---

## Resources

- [Intel 64 and IA-32 Architectures Software Developer's Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [x86-64 Instruction Encoding](https://wiki.osdev.org/X86-64_Instruction_Encoding)
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [System V ABI AMD64](https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf)

---

## Building from Source
```bash
gcc -O2 -Wall -Wextra -std=c99 Aasm.c -o aasm
```

**Requirements:**
- GCC or Clang
- Linux x86-64
- C99 standard library

---

**Note:** Bug fixes and improvements will be released with new versions.

**Made by a 14 y.o system programmer (me)**