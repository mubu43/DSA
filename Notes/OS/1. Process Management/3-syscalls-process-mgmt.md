# System Calls for Process Management

---

## 1. OS API for Process Management

The operating system exposes its services to user programs through **system calls (syscalls)**. For process management, the key operations are:

- **Creating** a new process
- **Loading** a new program into a process
- **Terminating** a process
- **Waiting** for a child process to finish

User programs typically don't invoke syscalls directly. Instead, they use **wrapper functions** provided by the C standard library (libc / glibc on Linux). These wrappers handle the low-level details of placing arguments in registers and executing the `syscall` instruction.

```
  ┌─────────────────────────────────────────────────────┐
  │               User Application                       │
  │                                                     │
  │   pid_t child = fork();                             │
  │   if (child == 0) {                                 │
  │       execvp("ls", args);                           │
  │   } else {                                          │
  │       waitpid(child, &status, 0);                   │
  │   }                                                 │
  └─────────────────┬───────────────────────────────────┘
                    │ calls libc wrapper functions
                    ▼
  ┌─────────────────────────────────────────────────────┐
  │          C Library (glibc / musl)                     │
  │                                                     │
  │  fork() wrapper:                                    │
  │    - places syscall number in RAX                   │
  │    - places args in RDI, RSI, RDX, ...              │
  │    - executes 'syscall' instruction                 │
  │    - checks return value, sets errno if error       │
  └─────────────────┬───────────────────────────────────┘
                    │ trap into kernel
                    ▼
  ┌─────────────────────────────────────────────────────┐
  │               Linux Kernel                           │
  │                                                     │
  │  sys_call_table[__NR_clone] → kernel_clone()         │
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

## 2. POSIX Standard

**POSIX (Portable Operating System Interface)** is a family of IEEE standards (IEEE 1003.x) that define a portable API for UNIX-like operating systems. POSIX specifies:

- System call interfaces (`fork`, `exec`, `wait`, `open`, `read`, `write`, etc.)
- Shell and utilities behavior
- Threading API (`pthreads`)
- Signals, IPC mechanisms
- File system semantics

### Why POSIX Matters

- **Portability**: Code written using POSIX APIs can compile and run on any POSIX-compliant system — Linux, macOS, FreeBSD, Solaris, AIX, etc.
- **Standardized behavior**: Guarantees that `fork()` will behave the same way across systems.
- **Interview relevance**: When interviewers ask about process management syscalls, they're typically asking about the POSIX interface.

Linux is largely POSIX-compliant but not formally certified (unlike macOS, which is). Linux extends POSIX with additional syscalls (e.g., `epoll`, `clone`, `io_uring`, `splice`) for performance and features.

### Key POSIX Headers for Process Management

```c
#include <unistd.h>     // fork(), exec*(), _exit(), getpid(), pipe(), dup2()
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait(), waitpid(), WIFEXITED(), WEXITSTATUS()
#include <stdlib.h>     // exit()
#include <signal.h>     // kill(), signal(), sigaction()
```

---

## 3. ABI — Application Binary Interface

While the **API** defines the source-level interface (function names, parameter types), the **ABI (Application Binary Interface)** defines the binary-level contract:

- **How syscall arguments are passed**: Which registers hold which arguments.
- **Calling conventions**: How functions pass arguments and return values at the machine code level.
- **Data type sizes and alignment**: `int` is 4 bytes, `long` is 8 bytes on x86-64 Linux, etc.
- **Struct layout**: Padding and alignment of struct members.
- **ELF binary format**: How executables and shared libraries are structured.

### Linux x86-64 Syscall ABI

| Register | Purpose |
|----------|---------|
| `RAX` | Syscall number (input); return value (output) |
| `RDI` | 1st argument |
| `RSI` | 2nd argument |
| `RDX` | 3rd argument |
| `R10` | 4th argument |
| `R8` | 5th argument |
| `R9` | 6th argument |
| `RCX`, `R11` | Clobbered by the `syscall` instruction (used by CPU for saving RIP and RFLAGS) |

Note: The syscall ABI differs slightly from the regular C function calling convention (System V AMD64 ABI), where the 4th argument goes in `RCX` instead of `R10`.

### Why ABI Matters

- **Binary compatibility**: Changing the ABI breaks all compiled programs. This is why the Linux kernel maintains strict ABI stability for syscalls — once a syscall is added, its interface never changes (Linus Torvalds' rule: "We don't break userspace").
- **Cross-language interop**: Any language (Rust, Go, Python via ctypes) can make Linux syscalls if it follows the ABI.

---

## 4. Core Syscalls for Process Management

### 4.1 `fork()` — Create a Child Process

`fork()` creates a **new process** by duplicating the calling process. The new process (child) is an almost exact copy of the parent.

```c
#include <unistd.h>
pid_t fork(void);
```

**Return values**:
- In the **parent**: returns the PID of the child (a positive number).
- In the **child**: returns 0.
- On **error**: returns -1 (and sets `errno`).

```
  Before fork():
  ┌──────────────────────┐
  │  Parent Process       │
  │  PID: 100             │
  │  Code + Data + Stack  │
  │  Open files           │
  │  PC at fork() call    │
  └──────────────────────┘

  After fork():
  ┌──────────────────────┐     ┌──────────────────────┐
  │  Parent Process       │     │  Child Process        │
  │  PID: 100             │     │  PID: 101             │
  │  fork() returns 101   │     │  fork() returns 0     │
  │  (same code, data,    │     │  (copy of code, data, │
  │   stack, open files)  │     │   stack, open files)  │
  └──────────────────────┘     └──────────────────────┘
```

**What is copied?**
- The entire address space (code, data, heap, stack) — but in practice, Linux uses **Copy-on-Write (COW)**: pages are shared read-only; a copy is made only when either process writes to a page.
- Open file descriptors (both processes share the same underlying open file descriptions — including the file offset).
- Signal handlers and mask.
- Environment variables.
- Current working directory.

**What is NOT copied?**
- PID (child gets a new unique PID).
- Parent PID (child's PPID = parent's PID).
- Pending signals (child starts with none).
- File locks (child doesn't inherit locks).
- Threads (only the calling thread is replicated in the child).

#### Example

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Before fork. PID = %d\n", getpid());
    
    pid_t child = fork();
    
    if (child < 0) {
        perror("fork failed");
        return 1;
    } else if (child == 0) {
        // Child process
        printf("I am the child.  PID = %d, PPID = %d\n", getpid(), getppid());
    } else {
        // Parent process
        printf("I am the parent. PID = %d, Child PID = %d\n", getpid(), child);
        wait(NULL);  // wait for child to finish
    }
    
    printf("This line is printed by PID %d\n", getpid());
    return 0;
}
```

Possible output:
```
Before fork. PID = 100
I am the parent. PID = 100, Child PID = 101
I am the child.  PID = 101, PPID = 100
This line is printed by PID 101
This line is printed by PID 100
```

Note: The order of parent/child execution after `fork()` is **non-deterministic** — the scheduler decides.

#### Linux Internals

- `fork()` in glibc is implemented via the `clone()` syscall (or more recently `clone3()`).
- The kernel function is `kernel_clone()` (in `kernel/fork.c`), formerly known as `do_fork()`.
- `kernel_clone()` calls `copy_process()`, which:
  1. Allocates a new `task_struct` for the child.
  2. Copies/shares the parent's `mm_struct`, `files_struct`, `fs_struct`, `signal_struct`, etc.
  3. Sets up the child's kernel stack.
  4. Inserts the child into the scheduler's run queue.
- **Copy-on-Write**: `copy_mm()` → `dup_mmap()` copies the parent's page table entries but marks all writable pages as read-only. When either process writes, a page fault occurs, and the kernel copies that page (the `do_wp_page()` handler).

#### `clone()` — Linux-specific Generalized Fork

`clone()` is the underlying Linux syscall that provides fine-grained control over what is shared between parent and child:

```c
int clone(int (*fn)(void *), void *stack, int flags, void *arg, ...);
```

Key flags:
| Flag | Effect |
|------|--------|
| `CLONE_VM` | Share virtual memory (address space) — this is what makes threads |
| `CLONE_FILES` | Share file descriptor table |
| `CLONE_FS` | Share filesystem info (cwd, root, umask) |
| `CLONE_SIGHAND` | Share signal handlers |
| `CLONE_THREAD` | Same thread group (same TGID, different TID) |
| `CLONE_NEWNS` | New mount namespace (used for containers) |
| `CLONE_NEWPID` | New PID namespace |

- `fork()` = `clone()` with no sharing flags.
- `pthread_create()` = `clone()` with `CLONE_VM | CLONE_FILES | CLONE_FS | CLONE_SIGHAND | CLONE_THREAD | ...`
- Container creation uses `clone()` with `CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWNET | ...`

### 4.2 `exec()` Family — Replace Process Image

The `exec` family of functions replaces the current process's code, data, and stack with a new program. The PID stays the same — only the program image changes.

```
  Before exec:                    After exec:
  ┌──────────────────────┐       ┌──────────────────────┐
  │  Process PID: 101     │       │  Process PID: 101     │
  │                       │       │                       │
  │  Code: old program    │  ──►  │  Code: /bin/ls        │
  │  Data: old data       │       │  Data: ls's data      │
  │  Heap: old heap       │       │  Heap: fresh          │
  │  Stack: old stack     │       │  Stack: fresh         │
  │                       │       │                       │
  │  Open FDs: preserved* │       │  Open FDs: preserved* │
  │  PID: unchanged       │       │  PID: unchanged       │
  │  PPID: unchanged      │       │  PPID: unchanged      │
  └──────────────────────┘       └──────────────────────┘
                                  * FDs without FD_CLOEXEC
```

The six variants differ in how they specify the program and arguments:

| Function | Program | Args | Env | Path search? |
|----------|---------|------|-----|-------------|
| `execl(path, arg0, arg1, ..., NULL)` | Path | Variadic list | Inherited | No |
| `execlp(file, arg0, arg1, ..., NULL)` | Filename | Variadic list | Inherited | **Yes** ($PATH) |
| `execle(path, arg0, ..., NULL, envp)` | Path | Variadic list | Explicit | No |
| `execv(path, argv[])` | Path | Array | Inherited | No |
| `execvp(file, argv[])` | Filename | Array | Inherited | **Yes** ($PATH) |
| `execve(path, argv[], envp[])` | Path | Array | Explicit | No |

**`execve` is the actual Linux syscall**. All other variants are libc wrappers that ultimately call `execve`.

```c
// execve prototype — the actual syscall
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**On success, `exec` never returns** — the old program is gone. On failure, it returns -1.

**What is preserved across exec?**
- PID, PPID, process group, session.
- Open file descriptors (unless `FD_CLOEXEC` / `O_CLOEXEC` is set).
- Real UID/GID (effective UID/GID may change if setuid/setgid bit is set on the executable).
- Resource limits.
- Current working directory.

**What is replaced/reset?**
- Entire address space (text, data, heap, stack).
- Signal handlers reset to defaults (since the old handler code no longer exists).
- Memory-mapped files unmapped.
- Threads — all threads except the calling thread are destroyed.

#### Example

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t child = fork();
    
    if (child == 0) {
        // Child: replace with "ls -la /tmp"
        char *args[] = {"ls", "-la", "/tmp", NULL};
        execvp("ls", args);
        // If we get here, exec failed
        perror("execvp failed");
        _exit(1);
    } else {
        // Parent: wait for child
        int status;
        waitpid(child, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }
    }
    return 0;
}
```

#### Linux Internals

- The `execve()` syscall handler is `do_execveat_common()` in `fs/exec.c`.
- It calls `search_binary_handler()` which tries registered binary formats:
  - **`binfmt_elf`**: For ELF executables (the vast majority).
  - **`binfmt_script`**: For scripts starting with `#!` (shebang). It reads the interpreter path and recursively calls exec on the interpreter with the script as an argument.
  - **`binfmt_misc`**: For custom formats registered via `/proc/sys/fs/binfmt_misc/`.
- For ELF binaries, `load_elf_binary()` (in `fs/binfmt_elf.c`):
  1. Reads ELF headers.
  2. Flushes the old address space (`exec_mmap()` → replaces `mm_struct`).
  3. Maps `.text`, `.data`, `.bss` segments.
  4. If dynamically linked, maps the dynamic linker (`ld-linux-x86-64.so.2`).
  5. Sets up the stack with `argv`, `envp`, and the ELF auxiliary vector.
  6. Sets the instruction pointer to the entry point.

### 4.3 `exit()` and `_exit()` — Terminate a Process

```c
#include <stdlib.h>
void exit(int status);     // libc function — flushes stdio buffers, runs atexit handlers

#include <unistd.h>
void _exit(int status);    // direct syscall — immediate termination, no cleanup
```

**What happens on exit:**
1. All open file descriptors are closed.
2. Children of the dying process are re-parented to `init` (PID 1) or a subreaper.
3. A `SIGCHLD` signal is sent to the parent.
4. The process enters `EXIT_ZOMBIE` state — it's dead but its `task_struct` remains so the parent can read the exit status.
5. The parent calls `wait()` / `waitpid()` to collect the exit status (this "reaps" the zombie and frees the `task_struct`).

**Exit status convention**: 0 means success, non-zero means failure. The lower 8 bits of the status are available to the parent.

**`_exit()` vs `exit()`**: After `fork()`, in the child process, prefer `_exit()` over `exit()` if the child is about to call `exec()` anyway, to avoid flushing the parent's buffered stdio data twice.

#### Linux Syscall

The actual syscall is `exit_group()` (terminates all threads in the thread group). `_exit()` maps to `exit_group()` in glibc. The kernel function is `do_exit()` in `kernel/exit.c`.

### 4.4 `wait()` and `waitpid()` — Wait for Child Process

```c
#include <sys/wait.h>

pid_t wait(int *wstatus);
// Blocks until ANY child terminates. Returns its PID.

pid_t waitpid(pid_t pid, int *wstatus, int options);
// Wait for a specific child (or -1 for any child).
// options: WNOHANG (non-blocking), WUNTRACED (also report stopped children)
```

The `wstatus` value encodes how the child terminated. Use macros to decode:

| Macro | Meaning |
|-------|---------|
| `WIFEXITED(status)` | True if child terminated normally (called `exit()`) |
| `WEXITSTATUS(status)` | The exit code (0-255) if `WIFEXITED` is true |
| `WIFSIGNALED(status)` | True if child was killed by a signal |
| `WTERMSIG(status)` | The signal number that killed the child |
| `WIFSTOPPED(status)` | True if child is stopped (only with `WUNTRACED`) |
| `WSTOPSIG(status)` | The signal that stopped the child |

```
  Parent                          Child
  ──────                          ─────
  fork()  ──────────────────►  (child created)
    │                               │
    │  waitpid(child, ...)          │  does work...
    │  (parent BLOCKS)              │
    │                               │  exit(42)
    │                               │
    │  ◄── SIGCHLD ────────────     │  → ZOMBIE state
    │                               │
    │  waitpid returns              │
    │  WEXITSTATUS = 42             │  → process reaped
    │                                   (task_struct freed)
    ▼
  continues...
```

#### Why Separating fork and exec Is Powerful

The UNIX design of separating process creation (`fork`) from program loading (`exec`) is deliberate and powerful. Between `fork` and `exec`, the child process can:

- **Redirect I/O** (close/open/dup file descriptors)
- **Set up pipes** (for shell pipelines like `ls | grep foo`)
- **Change environment variables**
- **Change the working directory**
- **Drop privileges** (setuid/setgid)
- **Set resource limits**

All of this setup happens *before* the new program starts, so the new program doesn't need to know about any of it. This is the foundation of the UNIX shell's power.

---

## 5. Foreground and Background Execution

In a shell:

- **Foreground process**: The shell waits for it to complete before showing the next prompt.
  ```bash
  $ sleep 10      # shell blocks here for 10 seconds
  ```
  
- **Background process**: The shell immediately returns the prompt. The process runs concurrently.
  ```bash
  $ sleep 10 &    # shell returns immediately
  [1] 12345       # job number and PID
  $ # can type more commands
  ```

### How the Shell Implements This

```c
// Simplified shell loop (pseudocode)
while (1) {
    char *cmd = read_command();
    int background = ends_with_ampersand(cmd);
    
    pid_t child = fork();
    if (child == 0) {
        // Child: run the command
        execvp(cmd_name, cmd_args);
        _exit(1);
    } else {
        if (!background) {
            // Foreground: wait for child
            waitpid(child, &status, 0);
        } else {
            // Background: don't wait, print job info
            printf("[1] %d\n", child);
        }
    }
}
```

### Job Control Signals

| Signal | Keyboard | Effect |
|--------|----------|--------|
| `SIGINT` | Ctrl+C | Interrupt — terminates the foreground process |
| `SIGTSTP` | Ctrl+Z | Stop — suspends the foreground process |
| `SIGCONT` | `fg` or `bg` command | Resume a stopped process |
| `SIGQUIT` | Ctrl+\\ | Quit — terminates with core dump |

```bash
$ sleep 100
^Z                    # Ctrl+Z: suspend
[1]+  Stopped  sleep 100
$ bg                  # resume in background
[1]+ sleep 100 &
$ fg                  # bring back to foreground
sleep 100
^C                    # Ctrl+C: kill it
```

The shell uses **process groups** and **sessions** to manage job control. The foreground process group receives keyboard signals. `tcsetpgrp()` sets which process group is in the foreground.

---

## 6. I/O Redirection and Pipes

### I/O Redirection

Every process starts with three open file descriptors:

| FD | Stream | Default |
|----|--------|---------|
| 0 | `stdin` | Terminal (keyboard) |
| 1 | `stdout` | Terminal (screen) |
| 2 | `stderr` | Terminal (screen) |

**Redirection** changes where these file descriptors point:

```bash
$ ls > output.txt       # stdout → file (truncate)
$ ls >> output.txt      # stdout → file (append)
$ sort < input.txt      # stdin ← file
$ ls 2> errors.txt      # stderr → file
$ ls > out.txt 2>&1     # both stdout and stderr → file
$ ls &> out.txt         # same (bash shorthand)
```

#### How the Shell Implements Redirection (fork + dup2 + exec)

```c
pid_t child = fork();
if (child == 0) {
    // Redirect stdout to a file
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(fd, STDOUT_FILENO);  // fd 1 now points to output.txt
    close(fd);                 // original fd no longer needed
    
    // Now exec — the new program's stdout goes to output.txt
    // and it doesn't even know about the redirection!
    execlp("ls", "ls", "-la", NULL);
    _exit(1);
}
waitpid(child, &status, 0);
```

**`dup2(oldfd, newfd)`**: Makes `newfd` a copy of `oldfd`. If `newfd` was already open, it's closed first. After `dup2(fd, 1)`, file descriptor 1 (stdout) points to wherever `fd` points.

```
  Before dup2(fd, 1):
    FD 0 → terminal (stdin)
    FD 1 → terminal (stdout)
    FD 2 → terminal (stderr)
    FD 3 → output.txt

  After dup2(3, 1):
    FD 0 → terminal (stdin)
    FD 1 → output.txt          ← redirected!
    FD 2 → terminal (stderr)
    FD 3 → output.txt          (same file, close this)

  After close(3):
    FD 0 → terminal (stdin)
    FD 1 → output.txt          ← all writes to stdout go to file
    FD 2 → terminal (stderr)
```

### Pipes

A **pipe** is a unidirectional communication channel between two processes. One process writes to one end, and the other reads from the other end.

```bash
$ ls -la | grep ".txt" | wc -l
```

This creates a pipeline of three processes connected by two pipes:

```
  ┌──────┐  pipe1   ┌──────────┐  pipe2   ┌───────┐
  │  ls  │ ──────►  │  grep    │ ──────►  │  wc   │
  │ -la  │ stdout   │ ".txt"   │ stdout   │  -l   │
  └──────┘  →pipe   └──────────┘  →pipe   └───────┘
            write     read/write    read
```

#### Creating a Pipe

```c
int pipe(int pipefd[2]);
// pipefd[0] = read end
// pipefd[1] = write end
```

```
  After pipe(pipefd):

  ┌───────────────────────────────────────┐
  │            Kernel Pipe Buffer          │
  │        (typically 64 KB on Linux)      │
  └──────────┬─────────────────┬──────────┘
             │                 │
     read end (pipefd[0])    write end (pipefd[1])
```

#### Implementing `ls | grep .txt` in C

```c
int pipefd[2];
pipe(pipefd);   // create pipe

pid_t pid1 = fork();
if (pid1 == 0) {
    // Child 1: ls
    close(pipefd[0]);              // close unused read end
    dup2(pipefd[1], STDOUT_FILENO); // stdout → pipe write end
    close(pipefd[1]);
    execlp("ls", "ls", "-la", NULL);
    _exit(1);
}

pid_t pid2 = fork();
if (pid2 == 0) {
    // Child 2: grep
    close(pipefd[1]);              // close unused write end
    dup2(pipefd[0], STDIN_FILENO); // stdin ← pipe read end
    close(pipefd[0]);
    execlp("grep", "grep", ".txt", NULL);
    _exit(1);
}

// Parent: close both ends (parent doesn't use the pipe)
close(pipefd[0]);
close(pipefd[1]);

// Wait for both children
waitpid(pid1, NULL, 0);
waitpid(pid2, NULL, 0);
```

**Critical rule**: Always close the unused ends of the pipe in each process. If the write end isn't closed in the reader, the reader will never see EOF and will hang forever waiting for more data.

### Pipe Internals in Linux

- Pipes are implemented as a circular buffer in kernel memory (`fs/pipe.c`).
- Default pipe buffer size: **64 KB** (16 pages × 4 KB). Configurable per-pipe via `fcntl(fd, F_SETPIPE_SZ, size)` up to `/proc/sys/fs/pipe-max-size`.
- If the pipe is **full**: `write()` blocks (or returns `EAGAIN` with `O_NONBLOCK`).
- If the pipe is **empty**: `read()` blocks (or returns `EAGAIN` with `O_NONBLOCK`).
- If all write ends are closed: `read()` returns 0 (EOF).
- If all read ends are closed: `write()` generates `SIGPIPE` (which terminates the writer by default).

### Named Pipes (FIFOs)

Regular pipes are anonymous — they exist only between related processes (parent/child). **Named pipes (FIFOs)** are visible in the filesystem:

```bash
$ mkfifo /tmp/my_pipe              # creates a named pipe (special file)
$ ls -la /tmp/my_pipe
prw-r--r-- 1 user user 0 ... /tmp/my_pipe    # 'p' type = pipe

# Terminal 1:
$ echo "hello" > /tmp/my_pipe      # blocks until someone reads

# Terminal 2:
$ cat /tmp/my_pipe                 # reads "hello"
```

Named pipes are created with `mkfifo()` syscall or the `mkfifo` command. They allow unrelated processes to communicate.

---

## Summary: The fork-exec-wait Pattern

This is the fundamental pattern for running programs in UNIX:

```
  Shell Process (Parent)
    │
    ├── fork()
    │     │
    │     ├── [child] Set up redirections (dup2, close, pipe)
    │     ├── [child] exec(program)
    │     │     └── program runs with inherited FDs
    │     │
    │     └── [child] _exit(status)   ← if exec fails
    │
    ├── [parent] waitpid(child)       ← foreground
    │   OR
    ├── [parent] continue             ← background
    │
    └── [parent] read exit status
```

| Syscall | What It Does | Key Linux Function |
|---------|-------------|-------------------|
| `fork()` | Creates child (COW copy) | `kernel_clone()` / `copy_process()` in `kernel/fork.c` |
| `clone()` | Generalized fork (fine-grained sharing) | `kernel_clone()` with flags |
| `execve()` | Replaces process image | `do_execveat_common()` in `fs/exec.c` |
| `exit()` / `_exit()` | Terminates process | `do_exit()` in `kernel/exit.c` |
| `wait()` / `waitpid()` | Waits for child, reaps zombie | `do_wait()` in `kernel/exit.c` |
| `pipe()` | Creates unidirectional channel | `do_pipe2()` in `fs/pipe.c` |
| `dup2()` | Duplicates file descriptor | `ksys_dup3()` in `fs/file.c` |
| `kill()` | Sends signal to process | `kill_something_info()` in `kernel/signal.c` |
