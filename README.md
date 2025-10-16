# AmmOS — Free Pseudo-OS (x86-64)

**AmmOS** is an experimental modular shell-driven system for fun, learning, and low-level control.  

- **License:** MIT License  
- **Purpose:** Educational and experimental  
- **Supported platforms:** Linux (Debian, Ubuntu, WSL2, etc.)  
- **Compile:** Run `./make.sh` in the project root  
- **Run:** Execute `./run` in the project root  

**Dependencies:**  
- `nasm`  
- `gcc`  

---

## 💻 CLI Commands

All commands are implemented in `AmmSH_execute()`. Arguments are space-separated.  
Flags and preprocessing are handled by `parseFlags()` and `preprocessor()`.

---

### ⚙️ System

| Command | Description |
|---------|-------------|
| `c` | Clear screen using ANSI escape codes |
| `ex` | Exit AmmOS and return to the host shell |
| `sleep [seconds ...]` | Pause execution (supports multiple arguments) |
| `getlogin` | Print the current username |
| `neofetch` | Display AmmOS system information |
| `reboot` | Restart AmmOS using `execl()` |
| `chg -un <username>` | Change current username (requires reboot to take effect) |
| `memreg` | Print memory usage, RAM, HDD, and `amm_malloc` statistics |

---

### 📂 File & Directory

| Command | Description |
|---------|-------------|
| `ls` | List current directory contents |
| `mkdir [dirname ...]` | Create directory(ies) |
| `go [dirname]` | Change current directory |
| `touch [filename ...]` | Create an empty file |
| `nano [filename]` | Open a file using `/bin/nano` |
| `r [filename ...]` | Print file contents (like `cat`) |
| `rf [filename ...]` | Remove file(s) |
| `rm [dirname ...]` | Remove a directory and all its contents recursively |

---

### 🔍 Search

| Command | Description |
|---------|-------------|
| `agrep -r-file <filename> [& start_dir]` | Recursively search for a file starting from a directory |
| `agrep -r-str <filename> <pattern> [& start_dir]` | Search for a string pattern in a file recursively |
| `agrep -r-file-str <filename> <pattern> [& start_dir]` | Search for a file containing a pattern recursively |

---

### 🧮 ASM Programs

| Command | Description |
|---------|-------------|
| `calc` | Simple calculator (assembly) |
| `fib` | Print Fibonacci numbers (assembly, supports high values) |
| `fac` | Calculate factorial |

---

### 🧱 AmmIDE (Bytecode Editor)

| Command | Description |
|---------|-------------|
| `AmmIDE` | Enter the bytecode editor |
| Inside AmmIDE: | `push <0-256>` – Push ASCII value to stack (`disk.dat`) |
|  | `free` – Pop last ASCII character from stack |
|  | `read` – Load and print stack as ASCII characters |

---

### 📦 Disk & Memory

| Command | Description |
|---------|-------------|
| `diskload` | Load stack from `disk.dat` and print it |
| `memload` | Print memory allocated using `amm_malloc()` |

---

### 🖥️ VGA

| Command | Description |
|---------|-------------|
| `gpu` | Enter VGA text drawing mode (`mov [WIDTH] [HEIGHT] [CHAR]`) |

---

### 👻 Daemons (`asystemd`)

| Command | Description |
|---------|-------------|
| `asystemd start <file.ammservice>` | Start a daemon from `.ammservice` file |
| `asystemd kill <apid>` | Kill a daemon by `apid` |
| `asystemd list` | List all running daemons with metadata |

---

### 📢 Output

| Command | Description |
|---------|-------------|
| `say [text ...]` | Print each argument as a new line (custom `echo`) |

---

### 🛠 Coming soon...

More tools, shell commands, and services will be added in future versions of AmmOS.

> AmmOS is self-documented through its codebase.  
> For available CLI commands, see `AmmSH_execute()` in `AmmSH.c`.
