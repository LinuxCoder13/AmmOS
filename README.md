											      
AmmOS — is free software and pseudo-OS (x86-64), licensed under GNU GPL v3.
It's an experimental modular shell-driven system built for fun, learning, and low-level control.
AmmOS is made for experimental/educational purpose it's can be runed only in Linux(Debian, Ubuntu, ... WSL2)  for compile write in '.' folder ->
(`./make.sh`) and run in main folder (`./run`).

AmmOS is licensed under the GNU General Public License v3.0 or later. See LICENSE for details.

# Dependencies
`nasm` && `gcc`

# AmmOS CLI Commands

> This is the list of available shell commands in AmmOS.

---

## ⚙️ System

### `c`
Clear screen using `\033` escape sequences.

### `ex`
Exit AmmOS and return to Linux/Windows (WSL2 or host shell).

### `sleep [seconds ...]`
Pause execution for the given seconds. Supports multiple delays.

### `getlogin`
Print current username.

### `neofetch`
Display system information about AmmOS.

---

## 📂 File & Directory

### `sizeof [filename]`
Print file size in bytes (supports binary files).

### `mkdir [dirname]`
Create a directory in the current folder.

### `ls`
List directory contents (same as `ls`).

### `touch [filename]`
Create an empty file.

### `nano [filename]`
Open a file using system `nano` editor.

### `go [dirname]`
Change current directory (`chdir`).

### `r [filename]`
Print contents of a file (like `cat`).

### `rf [filename]`
Remove file.

### `rm [dirname]`
Remove a directory and all its contents recursively.

---

## 🔍 Search

### `agrep -r-file <filename> [& start_dir]`
Recursively search for a file starting from directory.

### `agrep -r-str <filename> <pattern> [& start_dir]`
Search for a string pattern inside a file recursively.

---

## 🧮 ASM Programs

### `calc`
Simple calculator (written in assembly).

### `fib`
Print Fibonacci numbers (assembly, supports high values).

---

## 🧱 AmmIDE (Bytecode Editor)

### `AmmIDE`
Enter the bytecode editor.

Inside AmmIDE:
- `push <0-256>` – Push an ASCII value to the stack (`disk.dat`).
- `free` – Pop the last ASCII char from stack.
- `read` – Load and print stack as ASCII chars.

---

## 📦 Disk & Memory

### `diskload`
Load the stack from disk (`disk.dat`) and print it (outside AmmIDE).

### `memload`
Print memory allocated using `amm_malloc()`.

---

## 🖥️ VGA

### `gpu`
Enter VGA text drawing mode.

Example inside:
- `mov [WIDTH] [HEIGHT] [CHAR]` – Draw character at given coords.

---

## 👻 Daemons (`asystemd`)

### `asystemd start <file.ammservice>`
Start a background service defined by `.ammservice` file.

### `asystemd kill <apid>`
Kill a daemon by its assigned `apid`.

### `asystemd list`
List all running demons with their metadata.

---

## 📢 Output

### `say [text ...]`
Print each argument as a new line (custom `echo`).

---

## 🛠 Coming soon...
More tools, shell commands and services will be added in future versions of AmmOS.

> 🛠 AmmOS is self-documented through its codebase.  
> For available CLI commands, see `AmmSH_execute()` in `AmmSH.c`.


 
# AmmSH-scripts

AmmSH interpreter supports basic commands:

1. `if ... else ... endif`  
   Conditional execution based on the result of a command.  
   Example: `if r test.txt` – if the command `r test.txt` doesn't return 1 (i.e., it succeeds), the `if` block is executed.  
   - Cannot contain nested `if ... else` blocks.

2. `loop ... endloop`  
   Repeats a block of code a specified number of times.  
   - Cannot contain `if ... else` inside.

3. `if` + shell commands  
   Example: `if go ..`

4. One-line commands  
   Example: `go ..`

---

## Rules

1. No variables allowed  
2. Interpret script using: `AmmSH [file_name]`  
3. No nested `if` or `loop` blocks allowed

---

## Code Example

```ammsh
if go ..
    print I went 1 dir back!
else
    print fail!
endif

loop 10
    neofetch
endloop

