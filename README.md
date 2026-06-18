# Simple Shell

A Unix-style command-line shell written in C. This project demonstrates process creation, command execution, input parsing, and command pipelines using Linux system calls.

## Features

* Runs Linux commands from a custom shell prompt
* Supports command-line arguments
* Supports custom prompts
* Supports command pipelines using `|`
* Creates child processes with `fork()`
* Executes programs with `execvp()`
* Waits for child processes with `waitpid()`
* Prints child process status after execution
* Handles empty input and end-of-file gracefully

## Technologies Used

* C
* Linux/Unix System Calls
* Makefile
* Process Management
* Pipes
* Standard Input/Output Redirection

## Build

```bash
make
```

## Run

```bash
make run
```

## Example Commands

```bash
ls
ps
cat commands.txt | wc -l
exit
```

## What I Learned

This project helped me understand how Unix shells work internally, including how processes are created, how programs are executed, and how pipes connect the output of one command to the input of another.
