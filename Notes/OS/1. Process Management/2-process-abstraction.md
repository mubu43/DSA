# Process Abstraction

---

## 1. What Defines a Process?

A **process** is an instance of a program in execution. While a *program* is a passive entity (an executable file on disk), a *process* is an active entity — the program loaded into memory, along with all the state the OS needs to manage its execution.

A process is fully defined by its:

1. **Address space (memory image)**: The code, data, stack, and heap in virtual memory.
2. **CPU state (register context)**: The current values of all CPU registers — program counter (PC), stack pointer (SP), general-purpose registers, flags register.
3. **OS metadata**: Information the kernel tracks about the process — PID, state, open files, scheduling priority, resource limits, parent process, etc.

```
  What makes up a process?
  ┌─────────────────────────────────────────────┐
  │                                             │
  │  ┌───────────────┐  ┌───────────────────┐   │
  │  │ Address Space  │  │  CPU Context      │   │
  │  │               │  │                   │   │
  │  │ .text (code)  │  │ PC (RIP)          │   │
  │  │ .data         │  │ SP (RSP)          │   │
  │  │ .bss          │  │ General regs      │   │
  │  │ heap          │  │ RFLAGS            │   │
  │  │ stack         │  │ Segment regs      │   │
  │  │ mmap regions  │  │ FP/SSE/AVX regs   │   │
  │  └───────────────┘  └───────────────────┘   │
  │                                             │
  │  ┌─────────────────────────────────────┐    │
  │  │        OS Metadata (PCB/task_struct) │    │
  │  │                                     │    │
  │  │  PID, PPID, UID, GID               │    │
  │  │  Process state (running, sleeping)  │    │
  │  │  Open file descriptors              │    │
  │  │  Signal handlers & pending signals  │    │
  │  │  Scheduling info (priority, policy) │    │
  │  │  Memory management info (mm_struct) │    │
  │  │  Resource limits (rlimits)          │    │
  │  │  Credentials (capabilities)         │    │
  │  └─────────────────────────────────────┘    │
  │                                             │
  └─────────────────────────────────────────────┘
```

### Program vs. Process

| Program | Process |
|---------|---------|
| Passive entity — file on disk | Active entity — executing in memory |
| One program file | Can have multiple processes (instances) |
| No state | Has CPU state, memory, OS metadata |
| Exists until deleted | Exists until terminated |

Multiple processes can run the same program (e.g., two terminal windows both running `bash`). Each is a separate process with its own address space, PID, and state.

---

## 2. States of a Process and Transitions

A process is always in one of several well-defined states. The OS scheduler manages transitions between these states.

### The Five-State Model

```
                       ┌──────────────────────────────┐
                       │                              │
                       │         ┌───────────┐        │
          fork()       │    ┌───►│  READY    │◄───┐   │
          /exec        │    │    │(runnable, │    │   │
            │          │    │    │ in run    │    │   │
            ▼          │    │    │ queue)    │    │   │
     ┌──────────┐      │    │    └─────┬─────┘    │   │
     │  NEW     │──────┘    │          │          │   │
     │(created) │      admitted   scheduled   preempted
     └──────────┘           │     by CPU      (timer
                            │     scheduler    interrupt)
                            │          │          │
                            │          ▼          │
                            │    ┌───────────┐    │
                            │    │  RUNNING   │───┘
                            │    │(executing  │
                            │    │ on CPU)    │
                            │    └──┬────┬───┘
                            │       │    │
                   I/O wait │       │    │  exit() or
                   or sleep │       │    │  signal
                            │       │    │
                            ▼       │    ▼
                      ┌──────────┐  │  ┌──────────────┐
                      │ BLOCKED  │  │  │  TERMINATED   │
                      │(waiting  │  │  │  (zombie /    │
                      │ for I/O, │  │  │   exited)     │
                      │ signal,  │──┘  └──────────────┘
                      │ event)   │
                      └──────────┘
                  I/O complete or
                  event arrives →
                  moves to READY
```

### State Descriptions

| State | Description | When Does This Happen? |
|-------|-------------|------------------------|
| **NEW** | Process is being created. Kernel allocating PCB, setting up address space. | `fork()` / `clone()` called |
| **READY** | Process is loaded and ready to run, but waiting for CPU. Sitting in the **run queue**. | Just created, or preempted, or woke up from blocking |
| **RUNNING** | Process is actively executing instructions on a CPU core. At most one process per core is RUNNING. | Scheduler selected this process |
| **BLOCKED (Sleeping)** | Process cannot proceed — waiting for some event (I/O completion, lock acquisition, timer, signal). | Called `read()` on disk, `wait()`, `sleep()`, `lock()`, etc. |
| **TERMINATED (Zombie)** | Process has finished execution but its entry still exists in the process table so the parent can collect its exit status. | Called `exit()`, killed by signal, or returned from `main()` |

### Key Transitions

1. **NEW → READY**: Kernel finishes setting up the process (allocates PCB, maps memory). The process is added to the scheduler's run queue.
2. **READY → RUNNING**: The scheduler picks this process from the run queue and dispatches it to a CPU core. The context switch loads this process's saved registers.
3. **RUNNING → READY**: The process is **preempted** — the timer interrupt fires and the scheduler decides to give the CPU to another process. The current process's state is saved and it goes back to the run queue.
4. **RUNNING → BLOCKED**: The process makes a system call that cannot complete immediately (e.g., `read()` from disk, `wait()` for child). The process is moved to a wait queue associated with the event.
5. **BLOCKED → READY**: The event the process was waiting for occurs (I/O completes, child exits, lock is released). The process is moved from the wait queue back to the run queue.
6. **RUNNING → TERMINATED**: The process calls `exit()`, returns from `main()`, or receives a fatal signal (SIGKILL, SIGSEGV, etc.). The kernel releases most resources but keeps the PCB entry until the parent calls `wait()`.

### Linux Process States

In Linux, the `task_struct` contains a `__state` field with these values (defined in `include/linux/sched.h`):

| Linux State | Constant | Meaning |
|-------------|----------|---------|
| `TASK_RUNNING` | 0 | Either running on CPU or in the run queue (covers both RUNNING and READY) |
| `TASK_INTERRUPTIBLE` | 1 | Sleeping, can be woken by signal or event |
| `TASK_UNINTERRUPTIBLE` | 2 | Sleeping, can only be woken by the specific event (not by signals). Shows as `D` in `ps` / `top`. |
| `__TASK_STOPPED` | 4 | Stopped by a signal (SIGSTOP, SIGTSTP) |
| `__TASK_TRACED` | 8 | Being traced by a debugger (ptrace) |
| `EXIT_ZOMBIE` | 32 | Terminated but parent hasn't called `wait()` yet |
| `EXIT_DEAD` | 16 | Final state — being removed from process table |
| `TASK_IDLE` | | Idle kernel thread |

Note that Linux does not have separate READY and RUNNING states — both are `TASK_RUNNING`. A `TASK_RUNNING` process is either actually on a CPU or sitting in the run queue waiting to be scheduled.

You can observe process states:
```bash
$ ps aux    # STAT column: R=running, S=sleeping, D=uninterruptible, Z=zombie, T=stopped
$ top       # S column shows state
$ cat /proc/<pid>/status   # shows State field
```

**Zombie processes**: When a child exits, it becomes a zombie (`Z` state) until the parent calls `wait()` / `waitpid()`. If the parent never calls `wait()`, zombies accumulate. If the parent exits, zombies are re-parented to `init` (PID 1) or a subreaper, which reaps them.

**Uninterruptible sleep (`D` state)**: This state exists for processes doing I/O that *must not* be interrupted (e.g., waiting for disk I/O in the middle of a critical filesystem operation). You cannot kill a `D`-state process with `kill -9`; you must wait for the I/O to complete. Excessive `D`-state processes often indicate I/O problems (failing disk, NFS stall, etc.).

---

## 3. PCB — Process Control Block

The **Process Control Block (PCB)** is the kernel data structure that represents a process. It contains everything the OS needs to manage the process.

### What's Inside the PCB?

```
  ┌──────────────────────────────────────────────────────┐
  │                   PCB (task_struct)                    │
  ├──────────────────────────────────────────────────────┤
  │ Identification                                        │
  │   - PID (process ID)                                  │
  │   - TGID (thread group ID, = PID of main thread)      │
  │   - PPID (parent PID)                                 │
  │   - UID, GID (user/group)                             │
  │   - Process name (comm[])                             │
  ├──────────────────────────────────────────────────────┤
  │ Scheduling Information                                │
  │   - State (TASK_RUNNING, TASK_INTERRUPTIBLE, ...)     │
  │   - Priority (static, dynamic, nice value)            │
  │   - Scheduling policy (SCHED_NORMAL, SCHED_FIFO, ..) │
  │   - CPU affinity mask                                 │
  │   - Runtime statistics (vruntime for CFS)             │
  │   - Run queue pointers                                │
  ├──────────────────────────────────────────────────────┤
  │ CPU Context (saved on context switch)                  │
  │   - Saved registers (RSP, RIP, general purpose, etc.) │
  │   - FPU / SSE / AVX state                             │
  │   - Kernel stack pointer                              │
  ├──────────────────────────────────────────────────────┤
  │ Memory Management (mm_struct pointer)                  │
  │   - Page table base (pgd — loaded into CR3)           │
  │   - Virtual memory areas (VMAs)                       │
  │   - Code, data, heap, stack boundaries                │
  │   - RSS (resident set size), virtual memory size      │
  ├──────────────────────────────────────────────────────┤
  │ File System Info (files_struct pointer)                │
  │   - File descriptor table (array of pointers          │
  │     to open file objects)                             │
  │   - Current working directory                         │
  │   - Root directory                                    │
  │   - umask                                             │
  ├──────────────────────────────────────────────────────┤
  │ Signal Handling                                       │
  │   - Pending signals                                   │
  │   - Signal handlers (sigaction array)                 │
  │   - Blocked signal mask                               │
  ├──────────────────────────────────────────────────────┤
  │ IPC & Synchronization                                 │
  │   - Futex state                                       │
  │   - Wait queue entries                                │
  ├──────────────────────────────────────────────────────┤
  │ Resource Accounting                                   │
  │   - User time, system time (utime, stime)             │
  │   - Resource limits (rlimits)                         │
  │   - I/O statistics                                    │
  │   - cgroup membership                                 │
  ├──────────────────────────────────────────────────────┤
  │ Relationships                                         │
  │   - Parent pointer                                    │
  │   - Children list                                     │
  │   - Sibling list                                      │
  │   - Thread group list                                 │
  │   - Process group, session                            │
  └──────────────────────────────────────────────────────┘
```

### Linux: `task_struct`

In Linux, the PCB is the `struct task_struct`, defined in `include/linux/sched.h`. It's one of the largest structures in the kernel (~6-8 KB). Key fields:

```c
struct task_struct {
    volatile long           __state;          // Process state
    void                    *stack;           // Kernel stack pointer
    
    pid_t                   pid;              // Process ID
    pid_t                   tgid;             // Thread group ID
    
    struct task_struct      *parent;          // Parent process
    struct list_head        children;         // List of children
    struct list_head        sibling;          // Link in parent's children list
    
    struct mm_struct        *mm;              // Memory descriptor (NULL for kernel threads)
    
    struct files_struct     *files;           // Open file descriptors
    struct fs_struct        *fs;              // Filesystem info (cwd, root)
    struct nsproxy          *nsproxy;         // Namespaces
    
    const struct cred       *cred;            // Credentials (UID, GID, capabilities)
    
    struct signal_struct    *signal;          // Signal handling
    struct sigpending       pending;          // Pending signals
    
    // Scheduling
    int                     prio;             // Dynamic priority
    int                     static_prio;      // Static priority (based on nice)
    unsigned int            policy;           // Scheduling policy
    struct sched_entity     se;               // CFS scheduling entity
    cpumask_t               cpus_mask;        // CPU affinity
    
    char                    comm[TASK_COMM_LEN]; // Process name (16 chars)
    
    // ... many more fields
};
```

Important relationships:
- **`mm_struct`**: Contains the process's page table pointer (`pgd`), list of virtual memory areas (VMAs), and memory usage statistics. Shared among threads in the same process.
- **`files_struct`**: Contains the file descriptor table. Shared among threads; can be shared across processes after `clone()` with `CLONE_FILES`.
- **`thread_info`** / **kernel stack**: Each process/thread has its own kernel stack (typically 8 KB or 16 KB on x86-64). The `thread_info` structure is embedded at the bottom of the kernel stack and contains a pointer back to `task_struct`.

### Accessing the Current Process in Linux

The kernel uses the `current` macro to get the `task_struct` of the currently running process on the current CPU:
```c
struct task_struct *p = current;  // current process
printk("PID: %d, Name: %s\n", p->pid, p->comm);
```

On x86-64, `current` is implemented by reading a per-CPU variable stored in the `GS` segment register, making it an O(1) operation.

### Viewing PCB Info from Userspace

```bash
# Via /proc filesystem
$ cat /proc/<pid>/status      # human-readable summary
$ cat /proc/<pid>/stat        # raw numeric fields
$ cat /proc/<pid>/statm       # memory usage
$ ls -la /proc/<pid>/fd/      # open file descriptors
$ cat /proc/<pid>/maps        # memory mappings (VMAs)
$ cat /proc/<pid>/cgroup      # cgroup membership
$ cat /proc/<pid>/limits      # resource limits
```

---

## 4. The CPU Scheduler

The **CPU scheduler** (also called the **short-term scheduler**) decides which READY process gets the CPU next. It runs whenever a scheduling decision needs to be made:

- A process transitions from RUNNING to BLOCKED (voluntary yield — process calls a blocking syscall).
- A process transitions from RUNNING to READY (preemption — timer interrupt fires).
- A process transitions from BLOCKED to READY (I/O completes — the newly ready process might preempt the current one).
- A process is created (NEW → READY) or terminated (RUNNING → TERMINATED).

### Scheduling Concepts

- **Run queue**: The data structure holding all READY processes. Each CPU core has its own run queue in Linux.
- **Dispatch**: The act of giving the CPU to a process — involves a context switch.
- **Preemption**: Forcibly taking the CPU away from a running process (via timer interrupt) to give it to another. A **preemptive** scheduler can do this; a **non-preemptive** (cooperative) scheduler waits for the process to voluntarily yield.
- **Time quantum (time slice)**: The maximum time a process can run before being preempted.
- **Context switch overhead**: Saving/restoring register state, flushing TLB entries (or using ASIDs/PCIDs), cache pollution. Typically 1–10 μs on modern hardware.

### Linux Scheduler Overview

Linux's scheduler has evolved:
- **O(n) scheduler** (kernel 2.4): Scanned all processes to find the best. Poor scalability.
- **O(1) scheduler** (kernel 2.6): Used active/expired priority arrays. Good for throughput, poor for interactive responsiveness.
- **CFS — Completely Fair Scheduler** (kernel 2.6.23): Models an ideal fair CPU where each process gets exactly 1/N of the CPU. Uses `vruntime` (virtual runtime) tracked in a red-black tree. The process with the lowest `vruntime` is selected next.
- **EEVDF — Earliest Eligible Virtual Deadline First** (kernel 6.6+): Replacement for CFS. Adds a deadline concept to improve latency guarantees while maintaining fairness.

Linux supports multiple scheduling classes (highest priority wins):
1. **SCHED_DEADLINE**: Earliest deadline first — for hard real-time tasks.
2. **SCHED_FIFO / SCHED_RR**: Real-time policies — fixed priority, run-to-completion or round-robin.
3. **SCHED_NORMAL (SCHED_OTHER)**: Default — CFS/EEVDF for regular processes.
4. **SCHED_BATCH**: For CPU-bound batch jobs — less preemption.
5. **SCHED_IDLE**: Very low priority — runs only when nothing else wants the CPU.

```bash
# View scheduling policy and priority of a process
$ chrt -p <pid>

# Set a process to SCHED_FIFO with priority 50
$ sudo chrt -f -p 50 <pid>

# View nice value
$ nice -n 10 ./my_program   # launch with niceness +10
$ renice -5 -p <pid>        # change niceness of running process
```

The scheduler is deeply tied to the timer subsystem. The kernel's **tick** (timer interrupt) is the heartbeat that drives preemption.

---

## 5. The Boot Process

Understanding the boot process shows how the kernel and the very first process come into existence.

### Boot Sequence (x86 / UEFI Linux)

```
  Power On
     │
     ▼
  ┌──────────────────────────┐
  │  1. Firmware (UEFI/BIOS)  │  CPU starts in real mode, executes firmware
  │     - POST (Power-On      │  from a fixed address (0xFFFFFFF0 on x86).
  │       Self Test)           │  Initializes hardware, finds boot device.
  │     - Load bootloader      │
  └────────────┬──────────────┘
               │
               ▼
  ┌──────────────────────────┐
  │  2. Bootloader (GRUB2)    │  Loaded from disk (EFI System Partition
  │     - Display boot menu   │  in UEFI mode, or MBR in legacy BIOS).
  │     - Load kernel image   │  Passes kernel command line parameters.
  │       (vmlinuz) into RAM  │  Loads initramfs (initial RAM filesystem).
  │     - Load initramfs      │
  └────────────┬──────────────┘
               │
               ▼
  ┌──────────────────────────┐
  │  3. Kernel Initialization │  start_kernel() in init/main.c
  │     - Decompress kernel   │  Sets up:
  │     - Setup page tables   │  - Memory management (page allocator)
  │     - Initialize memory   │  - Interrupt handlers (IDT)
  │       management          │  - Scheduler
  │     - Init scheduler      │  - Device drivers
  │     - Mount initramfs     │  - Root filesystem
  │     - Start init process  │
  └────────────┬──────────────┘
               │
               ▼
  ┌──────────────────────────┐
  │  4. Init Process (PID 1)  │  First user-space process.
  │     (systemd on modern    │  Spawns all other user-space services.
  │      Linux distros)       │  Remains running as parent of orphans.
  │     - Mount filesystems   │
  │     - Start services      │
  │     - Reach target/runlvl │
  └────────────┬──────────────┘
               │
               ▼
  ┌──────────────────────────┐
  │  5. Login / User Session  │  getty → login → shell
  │     User can interact     │  Or display manager → desktop
  └──────────────────────────┘
```

### Key Details

**Firmware (UEFI/BIOS)**:
- On power-on, the CPU jumps to a hardcoded address and begins executing firmware code from flash ROM.
- UEFI (Unified Extensible Firmware Interface) is the modern replacement for legacy BIOS. It understands GPT partition tables, FAT32 filesystems, and can directly load EFI executables.

**Bootloader (GRUB2)**:
- GRUB (Grand Unified Bootloader) is the most common Linux bootloader.
- It loads the compressed kernel image (`vmlinuz`) and the initial ramdisk (`initramfs` / `initrd`) into memory.
- Kernel command-line parameters (e.g., `root=/dev/sda1 quiet splash`) are passed here.
- Configuration: `/boot/grub/grub.cfg`.

**Kernel Initialization**:
- The kernel's `start_kernel()` function (in `init/main.c`) is the main initialization entry point.
- It initializes subsystems in order: memory, interrupts, timers, scheduler, VFS, etc.
- After initialization, it executes `kernel_init()`, which mounts the root filesystem and runs the init program.
- The kernel tries to execute (in order): the init= parameter from command line, `/sbin/init`, `/etc/init`, `/bin/init`, `/bin/sh`.

**Init Process (PID 1)**:
- **systemd** is the default init system on most modern Linux distributions (Fedora, Ubuntu, RHEL, Arch, Debian).
- systemd reads unit files (service definitions) from `/etc/systemd/system/` and `/usr/lib/systemd/system/`.
- It manages service dependencies, parallelizes startup, provides logging (`journalctl`), and much more.
- PID 1 has a special role: it adopts orphaned processes and reaps zombies.

**Kernel threads**:
- During boot, the kernel also spawns several kernel threads (visible in `ps` with square brackets):
  - `[kthreadd]` (PID 2) — parent of all kernel threads.
  - `[ksoftirqd/N]` — handles soft interrupts on CPU N.
  - `[kworker/N:M]` — kernel workqueue threads.
  - `[migration/N]` — handles process migration between CPUs.
  - `[rcu_preempt]` — RCU (Read-Copy-Update) grace period management.

```bash
# View boot messages
$ dmesg | head -50

# View systemd boot analysis
$ systemd-analyze
$ systemd-analyze blame    # time taken by each service

# View the init process
$ ps -p 1 -f

# View kernel threads
$ ps aux | grep '\[.*\]'
```

---

## Summary

| Concept | Key Idea | Linux Details |
|---------|----------|---------------|
| Process | Running instance of a program: address space + CPU state + OS metadata | `task_struct` in `include/linux/sched.h` |
| Process States | NEW, READY, RUNNING, BLOCKED, TERMINATED | `TASK_RUNNING`, `TASK_INTERRUPTIBLE`, `TASK_UNINTERRUPTIBLE`, `EXIT_ZOMBIE` |
| PCB | Kernel data structure holding all process info | `task_struct` (~6-8 KB), accessed via `current` macro |
| CPU Scheduler | Picks which ready process runs next | CFS/EEVDF, per-CPU run queues, red-black tree |
| Boot Process | Firmware → Bootloader → Kernel → Init → Services | UEFI → GRUB2 → `start_kernel()` → systemd (PID 1) |
