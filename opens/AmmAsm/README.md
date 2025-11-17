# AmmAsm - Minimal x86-64 Assembler

**Version:** 1.2  
**Author:** Ammar Najafli  
**License:** MIT  

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to machine code and produces **ELF executables** for Linux x86-64.

---

## ✨ Features

✅ **Direct x86-64 encoding** - No NASM/GAS dependencies  
✅ **Multiple operand sizes** - 8/16/32/64-bit registers and immediates  
✅ **Memory addressing** - Full SIB/ModRM support with explicit syntax  
✅ **Label support** - Global and local labels with symbol resolution  
✅ **Control flow** - `jmp`, `call` with relative addressing  
✅ **Two-pass linker** - Built-in symbol resolution and relocation  
✅ **Hex/Binary/Octal literals** - `0xDEADBEEF`, `0b1010`, `0o777`  
✅ **ELF output** - Generates valid Linux x86-64 executables  
✅ **Clean error messages** - Line numbers and helpful diagnostics  

---

## 🆕 What's New in v1.2

### 🎯 Label Support & Linker
- **Global labels** - `main:`, `_start:`, `my_func:`
- **Local labels** - `.loop:`, `.end:` (scoped to previous global label)
- **Two-pass linking** - Placeholder → symbol resolution → machine code patching
- **MOV with labels** - `mov %rax, msg` loads label address into register

### 🔀 Control Flow Instructions
- **JMP** - Unconditional jumps (`jmp label`, `jmp %rax`)
- **CALL** - Function calls (`call func`, `call %rbx`)
- **Relative addressing** - Automatic rel32 offset calculation for labels
- **Absolute jumps** - Register indirect jumps (`jmp %rax`, `call %r15`)

### 🔧 Backend Architecture
- **parseInst()** - Code generator (MOV, ADD, JMP, CALL)
- **collect_labels()** - First pass: gather all label addresses (PC tracking)
- **resolve_labels()** - Second pass: patch relocations (RELOC_ABS64, RELOC_REL32)
- **compiler()** - Orchestrates multi-pass compilation pipeline

---

## 🔄 Pipeline Stages

### 🔤 Lexer (LEXER) - Converts text to tokens
- Recognizes instructions, registers, literals, labels
- Handles comments (`//`, `;`, `/* */`)
- Supports multiple number bases (hex, binary, octal, decimal)
- Label scoping (global `label:`, local `.label:`)

### 🌳 Parser (PARSE) - Builds Abstract Syntax Tree
- Validates instruction operands
- Resolves operand types (REG/IMM/MEM/LABEL)
- Handles label scoping (global/local)
- Creates AST nodes for instructions and labels

### ⚙️ Code Generator (parseInst) - Emits x86-64 machine code
- REX prefix generation for 64-bit operations
- ModR/M and SIB byte encoding for memory addressing
- Displacement and immediate value encoding
- Placeholder generation for unresolved labels

### 🔗 Linker (collect_labels + resolve_labels)
- **First pass**: Assigns addresses to all labels (PC = 0x401000 + offset)
- **Second pass**: Resolves symbol references
  - `MOV r64, label` → RELOC_ABS64 (absolute 64-bit address)
  - `JMP/CALL label` → RELOC_REL32 (relative 32-bit offset)

### 💾 Compiler (compiler) - Assembles final binary
- Orchestrates all compilation passes
- Collects machine code from AST
- Writes ELF executable with proper headers

---

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
mov %rax, -10       ; Negative numbers
```

### Labels
```asm
; Global labels
exit:
    mov %rdi, 42
    syscall
_start:
    mov %rax, 60
    jmp exit
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

**Compile and run:**
```bash
./aasm input.asm
./a.bin # ELF64
```

---

## 🔍 Machine Code Example

**Source:**
```asm
_start:
    jmp skip
    mov %rdi, 1
skip:
    mov %rax, 60
    syscall
```

**Generated (hexdump a.bin):**
```
00001000:  e9 0a 00 00 00              ; jmp +10 (skip label)
00001005:  48 bf 01 00 00 00 00 00 00 00  ; mov rdi, 1
0000100f:  48 b8 3c 00 00 00 00 00 00 00  ; mov rax, 60
00001019:  0f 05                       ; syscall
```

**Linker work:**
1. First pass: `skip` label at address `0x40100F`
2. Second pass: JMP offset = `0x40100F - (0x401000 + 5) = 0x0A`
3. Patch: `E9 0A 00 00 00`

---

## 🎯 Design Philosophy

AmmAsm prioritizes:
- **Clarity over brevity** - Explicit `b=rbx` instead of cryptic NASM `[rbx]`
- **Zero dependencies** - Pure C99, no external libraries
- **Educational value** - Readable code for learning x86-64 encoding
- **Simplicity** - Minimal feature set, maximum understanding

---

## ⚠️ Known Limitations

- Limited instruction set (MOV, ADD, JMP, CALL in v1.2)
- No conditional jumps yet (JE, JNE, JG, etc.) - coming in v1.3
- No multi-file linking
- No macro system
- No floating-point support (FPU)
- No optimization passes (Never will be)


---

## 📚 Resources

- [Intel® 64 and IA-32 Architectures Software Developer's Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [x86-64 Instruction Encoding](https://wiki.osdev.org/X86-64_Instruction_Encoding)
- [ELF Format Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)

---


**Made by a 14 y.o system programmer(me)**
