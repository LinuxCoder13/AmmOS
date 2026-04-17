# AmmAsm — x86-64 Assembler

**Version:** 1.5  
**Author:** Ammar Najafli  
**License:** MIT  

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to machine code and produces **ELF executables** for Linux x86-64.

---

## What's New in v1.5

### New Instructions
- **`cmp`** — Compare instruction for all operand sizes (reg/reg, reg/imm), with proper opcode selection (`0x80`/`0x81`/`0x83`/`0x39`/`0x3A`) and short form for `al`/`ax`/`eax`/`rax`
- **JCC** — Full set of conditional jumps with `rel32` encoding:


### Backend Refactoring
- **`encode_mov_rm_rm()`** — Unified memory-operand encoder now handles all 10 ModRM/SIB addressing cases in a single function, replacing the scattered per-case logic from v1.4
- **`parse_addr_expr()`** — Address expression parser rewritten for clarity; validates all constraints (scale without index, `rsp` as index, empty expression) at parse time with precise error messages
- **`encode_mov_reg_imm()`** / **`encode_mov_reg_reg()`** — Extracted as standalone helpers, usable beyond `mov` (prep for `add`, `sub`, etc.)
- **REX prefix generation** — Centralized: `REX_BASE | REX_W | REX_R | REX_X | REX_B` flags composed consistently across all encoders
- **`indexof()` / `find_reg*_index()`** — Register lookup split into human-readable helpers vs. raw CPU encoding, no longer mixed

---

## Features

- **Direct x86-64 encoding** — No NASM/GAS dependencies
- **Multiple operand sizes** — 8/16/32/64-bit registers and immediates
- **Memory addressing** — Full SIB/ModRM support with explicit key-value syntax
- **Label support** — Global and local labels with two-pass symbol resolution
- **Inline literals** — Embed strings and data directly in `.text`
- **Control flow** — `jmp`, `call`, conditional jumps with relative addressing
- **Two-pass linker** — Built-in symbol resolution and relocation
- **Numeric literals** — `0xDEADBEEF`, `0b1010`, `0o777`, decimal, negative
- **ELF output** — Generates valid Linux x86-64 executables

---

> Assembling x86-64 code → generating machine code → running binary

[![Demo](https://img.youtube.com/vi/P-bdMZXXyVg/0.jpg)](https://youtube.com/watch?v=P-bdMZXXyVg)

---

## Pipeline Stages

### 1. Lexer (`LEXER`)
Converts source text to a flat token stream.

- Recognizes instructions, registers (`%rax`), literals, labels, directives
- Comments: `//`, `;`, `/* ... */`
- Number bases: hex (`0x`), binary (`0b`), octal (`0o`), decimal
- Label scoping: global `label:`, local `.label:` (scoped to last global)
- Character literals: `'A'`, `'\n'`, `'\0'`

### 2. Parser (`PARSE`)
Builds the Abstract Syntax Tree.

- Validates operand combinations per instruction
- Resolves operand types: `O_REG8/16/32/64`, `O_IMM`, `O_MEM`, `O_LABEL`, `O_CHAR`
- Produces typed AST nodes: `AST_INS`, `AST_LABEL`, `AST_U8/16/32/64`, `AST_SECTION`, etc.

### 3. Code Generator (`parseInst`)
Emits x86-64 machine code per AST node.

- REX prefix construction
- ModR/M and SIB encoding via `encode_mov_rm_rm()`
- Displacement and immediate encoding (little-endian via `As16/32/64`)
- Placeholder bytes (`0x00000000`) for unresolved label references

### 4. Linker (`collect_labels` + `resolve_labels`)
Two-pass symbol resolution.

- **Pass 1** — Walks AST, assigns `vaddr` to each `AST_LABEL` (base `0x401000`)
- **Pass 2** — Patches placeholders:
  - `MOV r64, label` → absolute 64-bit address (8 bytes at `mc[2]`)
  - `JMP/CALL/JCC label` → `rel32 = target - (current_pc + inst_size)`

### 5. Compiler (`compiler`)
Orchestrates all passes and writes the final binary buffer.

---

## Syntax Reference

### Registers
Prefix with `%`:
```asm
mov %rax, 42        ; 64-bit
mov %eax, 42        ; 32-bit
mov %ax,  42        ; 16-bit
mov %al,  42        ; 8-bit
mov %r8,  %r15      ; Extended regs r8–r15
```

### Numeric Literals
```asm
mov %rax, 42            ; Decimal
mov %rax, 0xff          ; Hexadecimal
mov %rax, 0b1010        ; Binary
mov %rax, 0o777         ; Octal
mov %rax, -10           ; Negative
mov %al,  'A'           ; Character literal
```

### Memory Addressing
Unlike NASM, AmmAsm uses an explicit key-value format inside `[...]`:

| Key | Meaning | Example |
|-----|---------|---------|
| `b=REG` | Base register | `b=rbx` |
| `i=REG` | Index register | `i=rcx` |
| `s=N` | Scale (1/2/4/8) | `s=4` |
| `d=N` | Displacement | `d=0x10` |

```asm
mov %rax, [b=rbx]                       ; [rbx]
mov %rax, [b=rbx, d=16]                 ; [rbx + 16]
mov %rax, [b=rbx, i=rcx, s=8]           ; [rbx + rcx*8]
mov %rax, [b=rbx, i=rcx, s=8, d=0x10]   ; [rbx + rcx*8 + 16]
mov [b=rsp, d=8], %rax                  ; store to [rsp+8]
```

### Data Directives
```asm
msg:    u8  "Hello, World!", 0x0A, 0
bytes:  u8  0x01, 0x02, 0x03, 'A', 'B'
words:  u16 100, 200, 300, 0x1234
dwords: u32 0xDEADBEEF, 1000000
qwords: u64 0x123456789ABCDEF0, 0
```

### Comparison
```asm
cmp %rax, 42        ; rax vs immediate (64-bit)
cmp %eax, %ebx      ; register vs register (32-bit)
cmp %al,  'A'       ; 8-bit with char literal
```

### Conditional Jumps
```asm
cmp %rax, 0
je  done:            ; jump if equal
jne loop:            ; jump if not equal
jl  less:            ; jump if less (signed)
jg  greater:         ; jump if greater (signed)
jb  below:           ; jump if below (unsigned)
ja  above:           ; jump if above (unsigned)
```

### Unconditional Control Flow
```asm
jmp  label          ; relative jump  (E9 rel32)
jmp  %rax           ; indirect jump  (FF /4)
call func           ; relative call  (E8 rel32)
call %rbx           ; indirect call  (FF /2)
```

### Load Label Address
```asm
mov %rax, msg:      ; load virtual address of 'msg' into rax
```

### Arithmetic
```asm
add %rax, 42        ; 64-bit add
add %eax, 100       ; 32-bit add
add %ax,  0xff      ; 16-bit add
add %al,  1         ; 8-bit add
```

---

## Examples

### Hello World
```asm
_start:
    mov %rax, 1         ; sys_write
    mov %rdi, 1         ; stdout
    mov %rsi, msg:      ; buffer
    mov %rdx, 14        ; length
    syscall

    mov %rax, 60        ; sys_exit
    mov %rdi, 0
    syscall

msg: u8 "Hello, World!", 0x0A
```


### strlen via inline machine code
```asm
msg: u8 "Hello, World!\n", 0

_start:
    mov %rcx, -1
    mov %rax,  0
    mov %rdi,  msg:

   .a:  u8 0xF2, 0xAE           ; repne scasb
   .b:  u8 0x48, 0xF7, 0xD1     ; not rcx
   .c:  u8 0x48, 0xFF, 0xC9     ; dec rcx

    mov %rax, 1
    mov %rdi, 1
    mov %rsi, msg:
    mov %rdx, %rcx
    syscall

    mov %rax, 60
    mov %rdi, 0
    syscall
```

---


## Building & Usage

```bash
# Build
gcc -O2 -std=c99 Aasm.c -o aasm

# Compile assembly
./aasm input.asm
./aasm f=input.asm o=output
./aasm input.asm o=myprogram

# Run
chmod +x output && ./output
```

**Options:**

| Flag | Description |
|------|-------------|
| `f=<file>` or `<file>` | Input `.asm` file |
| `o=<file>` | Output executable (default: `a.out`) |

---

## Known Limitations

- Limited instruction set — `mov`, `add`, `cmp`, `jmp`, `call`, `jcc`, `syscall` (more coming)
- `add reg, reg` not yet implemented
- No `sub`, `mul`, `div`, `xor`, `or`, `and`, `shl`, `shr` encoding yet (parsed but not emitted)
- No multi-file linking
- No `.data` / `.bss` sections (everything in `.text`)
- No macro system
- No floating-point (FPU/SSE)
- No optimization passes (intentional)

---

## Resources

- [Intel SDM — IA-32/x86-64 Developer Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [x86-64 Instruction Encoding — OSDev Wiki](https://wiki.osdev.org/X86-64_Instruction_Encoding)
- [ELF Specification](https://refspecs.linuxfoundation.org/elf/elf.pdf)
- [System V AMD64 ABI](https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf)

---

**Made by a 14 y.o. systems programmer.**
