# AmmAsm - Minimal x86-64 Assembler

**Version:** 1.0  
**Author:** Ammar Najafli  
**License:** MIT  

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to raw machine code without dependencies on external assemblers.

## ✨ Features

✅ **Direct x86-64 encoding** - No NASM/GAS dependencies  
✅ **Multiple operand sizes** - 8/16/32/64-bit registers and immediates  
✅ **Memory addressing** - Full SIB/ModRM support with explicit syntax  
✅ **Hex/Binary/Octal literals** - `0xDEADBEEF`, `0b1010`, `0o777`  
✅ **JIT-ready output** - Produces executable binary code  
✅ **Clean error messages** - Line numbers and helpful diagnostics  

## 🏗️ Architecture Overview


┌─────────────┐
│ input.asm │ mov %rax, 42
└──────┬──────┘
│
▼
┌─────────────┐
│ LEXER │ Tokenizes source → [T_INS, T_REG64, T_INT]
└──────┬──────┘
│
▼
┌─────────────┐
│ PARSER │ Builds AST → {cmd:"mov", operands:[rax, 42]}
└──────┬──────┘
│
▼
┌─────────────┐
│ CODEGEN │ Generates machine code → [48 b8 2a 00 00 00 00 00 00 00]
└──────┬──────┘
│
▼
┌─────────────┐
│ a.bin │ Raw executable bytes
└─────────────┘


## 🔄 Pipeline Stages

### 🔤 Lexer (LEXER) - Converts text to tokens
- Recognizes instructions, registers, literals, labels
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

### 💾 Compiler (Ammcompiler) - Writes binary output

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

```asm

mov %rax, 42        ; Decimal
mov %rax, 0xff      ; Hexadecimal
mov %rax, 0b1010    ; Binary
mov %rax, 0o777     ; Octal
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

### Example Program

input.asm:
```asm

mov %rax, 42
mov %rbx, %rax
mov [b=rsp, d=-8], %rbx
```

a.bin:

```48 b8 2a 00 00 00 00 00 00 00  ; mov rax, 42 ```
```48 89 c3                        ; mov rbx, rax  ```
```48 89 5c 24 f8                  ; mov [rsp-8], rbx```

### Run:
```bash

./aasm input.asm
./jit a.bin
```

## 🎯 Design Philosophy

AmmAsm prioritizes:

    **Clarity over brevity** - Explicit b=rbx instead of cryptic NASM syntax

    **Zero dependencies** - Pure C99, no external libraries

    **Educational value** - Readable code for learning x86-64 encoding

    **JIT-friendly** - Direct memory execution without ELF overhead

## ⚠️ Known Limitations

    **No ELF/PE output (raw binary only)**

    **No relocations or linking**

    **Limited instruction set (MOV only in v1.0)**

    **No macro system**

    **No floating-point support**

