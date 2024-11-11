## Overview
This project implements a **shell program in C** that can **parse, execute, and manage commands**. It supports built-in commands like changing directories (`cd`), displaying process status (`status`), and exiting the program (`exit`). Additionally, it can handle **foreground and background processes** and manage custom signal handling for **SIGINT** (`Ctrl+C`) and **SIGTSTP** (`Ctrl+Z`).

## Features

### Built-in Commands
- `cd`: Changes the current directory.
- `status`: Displays the exit status or termination signal of the last process.
- `exit`: Exits the shell, terminating any remaining child processes.

### Additional Commands
Commands other than the built-in ones are executed by **forking a child process**. The shell supports **input/output redirection** and **background processes**:

- **Input/Output Redirection**: Use `<` for input files and `>` for output files.
- **Background Processes**: Place `&` at the end of a command to execute it in the background (only if **foreground-only mode** is not active).

### Signal Handling
- **SIGINT** (`Ctrl+C`): Ignored by the shell but can be handled by foreground child processes.
- **SIGTSTP** (`Ctrl+Z`): Toggles between **foreground-only** and **background** modes.

## File Structure

- `parseCommands()`: Parses and tokenizes user input.
- `handle_SIGTSTP()`: Handles **SIGTSTP** (`Ctrl+Z`) to toggle **foreground-only mode**.
- `else_command_block()`: Executes commands based on user input.
- `leave()`: Exits the shell and terminates background processes.
- `enter_directory()`: Changes the directory.
- `status_of_pro()`: Displays the last process's status.
- `otherCommands()`: Executes non-built-in commands, handling **background/foreground mode**.
- `fork_c_process()`: Sets up file redirection and executes the command in a child process.
- `p_processes_p()`: Waits for a child process to complete.
- `execute_timedatectl()`: Executes the `timedatectl` command.

## Compilation
To compile the program, run:

```bash
gcc -o shell_program shell_program.c
Usage
After compiling, start the program by running:

bash
Copy code
./shell_program
The shell program will display : as a prompt. Enter commands as you would in a standard shell, and the program will execute them. Use exit to close the shell.
