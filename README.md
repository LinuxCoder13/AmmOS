											      
AmmOS - is open sourse guest OS is made for experimental/educational purpose it's can be runed only in Linux(Debian, Ubuntu, ... WSL2, termux) (Some times in MacOS). for compile write in /opens folder ->
(`gcc Ammshell.c Ammkernel.c AmmFS.c -o Ammshell`) and run in main folder (`./run`).

commands --->

1) c - is clearing screen like "clear" in bash BUT using ASCII to do it.

2) ex - to exit OS and go back to Linux or Windows if you use WSL2 lol ;)

3) sizeof [file name] - it shows how many bytes do your file use (also work with binary file)

4) AmmIDE - it's byte code editor commads ->
	|push <num 0-256> - it's pushing to stack(in memory.dat) the ascii ex.(push 65 -> 'A')
	|free - pop last char in stack(in memory.dat)
	|read - load stack(memory.dat) in that ascii chars

5) load - same as read but you must write not in AmmIDE

6) mkdir [dir_name] - make dir in '.' folder no use (system("mkdir"))

7) ls - you now ;) no use (system("ls"))

8) nano [file_name] - I used system("nano") I did't do my own lmao ;)

9) go [dir_name] - chdir(dir_name)

10) r [file_name] - it's like cat 

11) touch [file_name] - you now ;)

12) neofetch - info about AmmOS

13) getlogin - print currunt username

... SOON
 
#AmmSH-scripts:
AmmSH interpreter has basic commads ->

1) if ... else ... endif - if can take the result of some operation ex.(if r test.txt) if "r test.txt" code doesnt return 1 it will skip till else-block, can't have nested if...else
2) loop <index> ... endloop - doing some code <index> time can't have inside if...else
3) if + Shell-commads
4) one line commads ex.(go ..)

Rulse:
1) No variables (Soon)
2) no tabs or spase at start of line
3) to interpretate file white "AmmSH [file_name]"

code example ->
```
if go ..
print I went 1 dir back!

else 
print fail!

endif

loop 10
neofetch
endloop
```
thats all BUT when you will be develop AmmOS you must be acurate with path and files

