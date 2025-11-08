# AmmAsm - Minimal x86-64 Assembler

**Version:** 1.1  
**Author:** Ammar Najafli  
**License:** MIT  

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to machine code and now can produce **raw binaries or ELF executables**.

## ✨ Features

✅ **Direct x86-64 encoding** - No NASM/GAS dependencies  
✅ **Multiple operand sizes** - 8/16/32/64-bit registers and immediates  
✅ **Memory addressing** - Full SIB/ModRM support with explicit syntax  
✅ **Hex/Binary/Octal literals** - `0xDEADBEEF`, `0b1010`, `0o777`  
✅ **ELF output support** - Minimal ELF loader compatible with Linux x86-64  
✅ **New instructions** - `add reg, imm` for 8/16/32/64-bit registers  
✅ **JIT-ready output** - Produces executable binary code  
✅ **Clean error messages** - Line numbers and helpful diagnostics  

## 🔄 Pipeline Stages

### 🔤 Lexer (LEXER) - Converts text to tokens
- Recognizes instructions, registers, literals
- Handles comments (`//`, `;`, `/* */`)
- Supports multiple number bases (hex, binary, octal)

### 🌳 Parser (PARSE) - Builds Abstract Syntax Tree
- Validates instruction operands
- Resolves operand types (REG/IMM/MEM/LABEL)
- Handles label scoping (global/local)

### ⚙️ Code Generator (parseInst) - Emits x86-64 bytes
- REX prefix generation
- ModR/M and SIB byte encoding
- Displacement and immediate value encoding
- New support for `add reg, imm` (all register sizes)

### 💾 Compiler (Ammcompiler) - Writes binary output
- Raw binary output (previously)
- **ELF output** with `.text` segment for Linux x86-64

## 📝 Syntax

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

```
mov %rax, 42        ; Decimal
mov %rax, 0xff      ; Hexadecimal
mov %rax, 0b1010    ; Binary
mov %rax, 0o777     ; Octal
```

### Memory Addressing

```
; Format: [b=base, i=index, s=scale, d=displacement]
mov %rax, [b=rbx]                     ; [rbx]
mov %rax, [b=rbx, d=16]               ; [rbx + 16]
mov %rax, [b=rbx, i=rcx, s=8]         ; [rbx + rcx*8]
mov %rax, [b=rbx, i=rcx, s=8, d=0x10] ; [rbx + rcx*8 + 0x10]
mov [b=rsp, d=8], %rax                ; Memory destination
```

### New Instructions: ADD

```
add %rax, 42        ; 64-bit
add %eax, 42        ; 32-bit
add %ax, 42         ; 16-bit
add %al, 42         ; 8-bit
```

### Example Program

input.asm:

```
mov %rax, 42
add %rax, 58
mov %rbx, %rax
mov [b=rsp, d=-8], %rbx
```

a.bin (ELF):
- 48 b8 2a 00 00 00 00 00 00 00  ; mov rax, 42
- 48 05 3a 00 00 00              ; add rax, 58
- 48 89 c3                        ; mov rbx, rax
- 48 89 5c 24 f8                  ; mov [rsp-8], rbx

### Run:

```
./aasm input.asm
./a.bin       # ELF executable
```

### 🎯 Design Philosophy

AmmAsm prioritizes:

- Clarity over brevity - Explicit b=rbx instead of cryptic NASM syntax

- Zero dependencies - Pure C99, no external libraries

- Educational value - Readable code for learning x86-64 encoding

### ⚠️ Known Limitations

- Limited instruction set (MOV, ADD in v1.1)

- No relocations or linking

- No macro system

- No floating-point support