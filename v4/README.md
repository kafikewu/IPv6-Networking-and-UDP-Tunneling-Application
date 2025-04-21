# Files and functions of this code
---
## remotecmds.bin
The binary file for the server. It opens the cmdfifo in read mode. Then it waits for the client (remotecmdc.bin) to write commands into the FIFO. Afterwards, it reads the command, parses it with the parse_token method, crates a child process and executes it with execvp method in the child process.
## remotecmd.c
The code of remotecmds.bin.
### char** parse_token(char arg[]);
This method takes a string as an argument. Afterwards, it splits the string by space into an array. Moreeover, it appends a NULL pointer at the end of the array and returns the array.
Example:
String: ls -l -a
Tokenized Array: {'ls', '-l', '-a', NULL}
## remotecmdc.bin
The binary file for the server. It opens the cmdfifo in write mode. Then it takes one input with maximum length of 30 characters and writes them into the FIFO.