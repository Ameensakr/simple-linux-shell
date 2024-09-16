
# Simple Shell Implementation

This repository contains the implementation of a basic shell in C, created as part of a lab exercise to deepen understanding of process management in Linux.

## Features

### Part A: Simple Shell
- **Execute Basic Linux Commands:** The shell accepts user input, forks a child process, executes the command using the `exec` system call, and reaps the child using the `wait` system call.
- **Prompt:** The shell uses `$ ` as the command prompt and continues to run until terminated by Ctrl+C.
- **Error Handling:** Handles errors gracefully, including empty commands and invalid input. Does not crash and displays an appropriate error message.
- **Change Directory (`cd`):** Supports `cd` to change the current working directory.

### Part B: Background Execution
- **Background Processes:** Supports background execution of processes using the `&` operator. Background processes do not block the shell from accepting new commands.
- **Child Reaping:** Reaps background processes periodically when receiving new input.

### Part C: Exit Command
- **`exit` Command:** Allows the shell to exit gracefully by terminating all background processes and cleaning up resources.

### Part D: Ctrl+C Handling
- **Signal Handling:** The shell catches the Ctrl+C signal (SIGINT) and ensures that only the foreground process is terminated. The shell itself continues running.

### Part E: Serial and Parallel Execution
- **Serial Execution:** Commands separated by `&&` are executed sequentially.
- **Parallel Execution:** Commands separated by `&&&` are executed in parallel.

## Getting Started

### Prerequisites
- **Linux Environment:** The shell is developed and tested in a Linux environment.
- **GCC Compiler:** Ensure that GCC is installed to compile the shell.

### Compilation

To compile the shell, run:

```bash
gcc myshell.c -o myshell
```

### Running the Shell

To run the shell, use:

```bash
./myshell
```

Once running, the shell will display the `$ ` prompt, where you can enter commands.

### Supported Commands
- Basic Linux commands like `ls`, `cat`, `echo`, and `sleep`.
- **cd** command to change directories.
- Background execution using `&`.
- Serial execution using `&&`.
- Parallel execution using `&&&`.
- **exit** command to terminate the shell.

### Example Usage
```bash
$ ls
file1.txt  file2.txt
$ sleep 10 &
$ cd ..
$ exit
```
