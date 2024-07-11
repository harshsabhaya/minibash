# MiniBash - Custom Shell Implementation

## Overview

MiniBash is a simplified Unix shell implementation in C that supports process management, I/O redirection, piping, and various special operators. The shell provides an interactive command-line interface with support for both simple and complex command execution patterns.

## Features

### 1. Basic Command Execution

Execute any standard Unix command with 1-4 arguments:

```bash
minibash$ ls
minibash$ pwd
minibash$ grep pattern file.txt
minibash$ ls -l -t -a
```

### 2. Word Count (`#`)

Count words in a text file:

```bash
minibash$ # sample.txt
```

### 3. File Concatenation (`~`)

Concatenate up to 4 text files and display output:

```bash
minibash$ file1.txt ~ file2.txt ~ file3.txt
```

### 4. Background Processes (`+`)

Run commands in the background:

```bash
minibash$ sleep 100 +
# Output: This process, [PID], is running in background

minibash$ fore
# Brings last background process to foreground
```

### 5. Piping (`|`)

Chain up to 4 commands with pipes:

```bash
minibash$ ls -l | grep txt | wc
minibash$ cat file.txt | grep pattern | sort | uniq
```

### 6. I/O Redirection (`<`, `>`, `>>`)

```bash
minibash$ grep pattern <input.txt          # Input redirection
minibash$ ls -l >output.txt                 # Output redirection (overwrite)
minibash$ ls -l >>output.txt                # Output redirection (append)
```

### 7. Sequential Execution (`;`)

Execute up to 4 commands sequentially:

```bash
minibash$ date ; pwd ; ls
```

### 8. Conditional Execution (`&&`, `||`)

Execute commands based on previous command's success/failure:

```bash
minibash$ make && ./program                 # AND: run if previous succeeds
minibash$ command1 || command2              # OR: run if previous fails
minibash$ cmd1 && cmd2 || cmd3 && cmd4      # Combined (up to 4 operations)
```

### 9. Exit Shell (`dter`)

Terminate the minibash shell:

```bash
minibash$ dter
```

## How to Run

### Compilation

```bash
gcc -o minibash minibash.c
```

### Execution

```bash
./minibash
```

### Example Session

```bash
$ ./minibash
minibash$ pwd
/home/user/project
minibash$ ls -l | grep txt
-rw-r--r-- 1 user group 1234 Nov 17 10:30 sample.txt
minibash$ echo "Hello" >output.txt
minibash$ cat output.txt
Hello
minibash$ dter
$
```

## Important Rules

1. **Argument Limit**: Each command must have 1-4 arguments (including command name)
2. **Operation Limit**: Maximum 4 operations for piping, concatenation, sequential, and conditional execution
3. **No Operator Mixing**: Cannot combine different special operators in a single command
4. **Resource Cleanup**: Always run `killall -u username` after testing to prevent fork bombs

## Error Messages

The shell displays colored error messages for:

- Invalid argument count: "Maximum 4 arguments are accepted"
- Too many operations: "Maximum 4 piping/sequential operations supported"
- Missing background process: "No such process in background"
- File access errors and command execution failures

## Technical Details

### System Calls Used

- `fork()` - Process creation
- `execvp()` / `execlp()` - Command execution
- `pipe()` - Inter-process communication
- `dup2()` - File descriptor redirection
- `wait()` / `waitpid()` - Process synchronization
- `open()` / `close()` - File operations

### Key Features

- Color-coded prompt (yellow) and error messages (red)
- Proper error handling and validation
- Modular function design for each operation type
- Background process tracking with foreground retrieval

## Limitations

- Only one background process tracked at a time
- Cannot combine special operators (e.g., `ls | grep txt >output.txt`)
- Fixed argument limit of 1-4 per command
- No command history or editing features
