# AmmAsm — x86-64 Assembler

**Version:** 1.6
**Author:** Ammar Najafli
**License:** MIT

AmmAsm is a lightweight, handwritten x86-64 assembler designed for simplicity and clarity. It compiles assembly code directly to machine code and produces ELF executables for Linux x86-64.

---

## What's New in v1.6

added new bugs.. lol I'm joking... or not...

### RIP-Relative Addressing (Full Support)

AmmAsm v1.6 now fully supports RIP-relative addressing for all memory operands. When a label is used as the base in an address expression, the assembler automatically emits `[rip + disp32]` encoding.

RIP-relative examples:

```
mov %rax, [b=msg]           ; lea rax, [rip + offset]
mov %rcx, [b=array, d=8]    ; [rip + array + 8]
add %rdx, [b=counter]       ; add rdx, [rip + counter]
```
*Important*: Local labels require full qualification

When accessing a local label (starting with dot) in an address expression, you must use the full path: [b=global.local]

```asm
_start:
    mov %rax, [b=_start.msg]    ; correct - full qualification
    ; mov %rax, [b=.msg]        ; wrong - will fail

    mov %rax, 1
    mov %rdi, 1
    mov %rsi, msg:
    syscall

.msg: u8 "Hello!", 0
```

---

> Assembling x86-64 code → generating machine code → running binary

[![Demo](https://img.youtube.com/vi/P-bdMZXXyVg/0.jpg)](https://youtube.com/watch?v=P-bdMZXXyVg)

---

How it works:

- When `b=LABEL` (a label name instead of register) -> parse_addr_expr() sets is_rip_rel = 1
- Encoder emits ModR/M: mod=00, r/m=101 (RIP-relative encoding)
- Displacement placeholder 0x00000000 is emitted
- Linker (resolve_labels()) calculates disp32 = target_addr - (current_pc + inst_size) + user_disp
- Final disp32 patched into the 4-byte displacement field

### Instruction add — Completed (All Forms)

The add instruction is now fully implemented in v1.6 (was partial in v1.5):

| Form | Encoding | Status |
|------|----------|--------|
| add reg, imm | 0x80/0x81/0x83 + ModR/M | Full |
| add reg, reg | 0x00/0x01 + ModR/M (mod=11) | Full |
| add reg, [addr] | 0x02/0x03 + SIB/ModRM | Full |
| add [addr], reg | 0x00/0x01 + SIB/ModRM | Full |
| add rax, imm (short form) | 0x04/0x05 | Full |
| All sizes (8/16/32/64) | REX.W + operands | Full |
| Extended registers r8-r15 | REX.B/R/R/X | Full |

What "completion" means:

- Previously: add had limited forms (only add reg, imm worked)
- v1.6: Every combination of register/memory/immediate works identically to mov
- All 10 ModRM/SIB addressing modes supported via unified encode_inst_rm_rm()

### Full Change Summary (v1.5 -> v1.6)

| Feature | v1.5 | v1.6 |
|---------|------|------|
| RIP-relative addressing | Not supported | Full support |
| add reg, imm | Partial | Full |
| add reg, reg | Not implemented | Full |
| add reg, mem | Not implemented | Full |
| add mem, reg | Not implemented | Full |
| add short forms (rax) | Not implemented | Full |

---

### Backend Refactoring

- encode_inst_rm_rm() — Unified memory-operand encoder now handles all 10 ModRM/SIB addressing cases in a single function
- Register lookup — Split into human-readable helpers vs. raw CPU encoding

---

## Features

- Direct x86-64 encoding — No NASM/GAS dependencies
- Multiple operand sizes — 8/16/32/64-bit registers and immediates
- Memory addressing — Full SIB/ModRM support with explicit key-value syntax
- RIP-relative addressing — Automatic for label bases (v1.6)
- Label support — Global and local labels with two-pass symbol resolution
- Inline literals — Embed strings and data directly in .text
- Control flow — jmp, call, conditional jumps with relative addressing
- Two-pass linker — Built-in symbol resolution and relocation
- Numeric literals — 0xDEADBEEF, 0b1010, 0o777, decimal, negative
- ELF output — Generates valid Linux x86-64 executables

---

## Pipeline Stages

### 1. Lexer (LEXER)

Converts source text to a flat token stream.

- Recognizes instructions, registers (%rax), literals, labels, directives
- Comments: //, ;, /* ... */
- Number bases: hex (0x), binary (0b), octal (0o), decimal
- Label scoping: global label:, local .label: (scoped to last global)
- Character literals: 'A', '\n', '\0'

### 2. Parser (PARSE)

Builds the Abstract Syntax Tree.

- Validates operand combinations per instruction
- Resolves operand types: O_REG8/16/32/64, O_IMM, O_MEM, O_LABEL, O_CHAR
- Produces typed AST nodes: AST_INS, AST_LABEL, AST_U8/16/32/64, AST_SECTION, etc.

### 3. Code Generator (parseInst)

Emits x86-64 machine code per AST node.

- REX prefix construction
- ModR/M and SIB encoding via encode_inst_rm_rm()
- Displacement and immediate encoding (little-endian via As16/32/64)
- Placeholder bytes (0x00000000) for unresolved label references

### 4. Linker (collect_labels + resolve_labels)

Two-pass symbol resolution.

- Pass 1 — Walks AST, assigns vaddr to each AST_LABEL (base 0x401000)
- Pass 2 — Patches placeholders:
  - MOV r64, label -> absolute 64-bit address (8 bytes at mc[2])
  - JMP/CALL/JCC label -> rel32 = target - (current_pc + inst_size)
  - RIP-relative -> disp32 = target - (current_pc + inst_size) + user_disp

### 5. Compiler (compiler)

Orchestrates all passes and writes the final binary buffer.

---

## Syntax Reference

### Registers

Prefix with %:

```
mov %rax, 42        ; 64-bit
mov %eax, 42        ; 32-bit
mov %ax,  42        ; 16-bit
mov %al,  42        ; 8-bit
mov %r8,  %r15      ; Extended regs r8-r15
```

### Numeric Literals

```
mov %rax, 42            ; Decimal
mov %rax, 0xff          ; Hexadecimal
mov %rax, 0b1010        ; Binary
mov %rax, 0o777         ; Octal
mov %rax, -10           ; Negative
mov %al,  'A'           ; Character literal
```

### Memory Addressing

Unlike NASM, AmmAsm uses an explicit key-value format inside [...]:

| Key | Meaning | Example |
|-----|---------|---------|
| b=REG | Base register | b=rbx |
| i=REG | Index register | i=rcx |
| s=N | Scale (1/2/4/8) | s=4 |
| d=N | Displacement | d=0x10 |

```
mov %rax, [b=rbx]                       ; [rbx]
mov %rax, [b=rbx, d=16]                 ; [rbx + 16]
mov %rax, [b=rbx, i=rcx, s=8]           ; [rbx + rcx*8]
mov %rax, [b=rbx, i=rcx, s=8, d=0x10]   ; [rbx + rcx*8 + 16]
mov [b=rsp, d=8], %rax                  ; store to [rsp+8]

; RIP-relative (new in v1.6)
mov %rax, [b=msg]                       ; load from msg
mov %rax, [b=msg, d=4]                  ; msg + 4
```

### Data Directives

```
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
jmp  label:         ; relative jump  (E9 rel32)
jmp  %rax           ; indirect jump  (FF /4)
call func:          ; relative call  (E8 rel32)
call %rbx           ; indirect call  (FF /2)
```

### Load Label Address
```asm
mov %rax, msg:      ; load virtual address of 'msg' into rax
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

- So today AmmAsm can compile any Thuring full assembly code 

**Made by a 14 y.o. systems programmer.**
