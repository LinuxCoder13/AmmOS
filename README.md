											      
AmmOS - is open sourse guest OS(x86-64) is made for experimental/educational purpose it's can be runed only in Linux(Debian, Ubuntu, ... WSL2)  for compile write in '.' folder ->
(`./make.sh`) and run in main folder (`./run`).

CLI commands --->

1) c - is clearing screen like "clear" in bash BUT using \033 to do it.

2) ex - to exit OS and go back to Linux or Windows if you use WSL2 

3) sizeof [file name] - it shows how many bytes do your file use (also work with binary file)

4) AmmIDE - it's byte code editor commads ->
	|push <num 0-256> - it's pushing to stack(in disk.dat) the ascii ex.(push 65 -> 'A')
	|free - pop last char in stack(in disk.dat)
	|read - load stack(disk.dat) in that ascii chars

5) diskload - same as read but you must write not in AmmIDE

6) mkdir [dir_name] - make dir in '.' folder no use (system("mkdir"))

7) ls - you now ;) no use (system("ls"))

8) nano [file_name] - I used system("nano")

9) go [dir_name] - chdir(dir_name)

10) r [file_name] - it's like cat 

11) touch [file_name] - you now ;)

12) neofetch - info about AmmOS

13) getlogin - print currunt username

14) rf [file_name] - remove file

15) rm [dir_name] - remove dir and sub dirs and files in

16) calc - calculator (wrote in asembly btw)

17) fib - fibonacci numbers (wrote in asembly for high seed)

18) gpu - it's VGA[50][70]
    | mov [WIDTH] [HEIGHT] [CHAR]

19) memload - loads all memory which was amm_malloced

... SOON
 
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

