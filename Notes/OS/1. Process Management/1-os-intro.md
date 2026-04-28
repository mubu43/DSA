# OS Introduction

---

## 1. What is an Operating System?

An **Operating System (OS)** is a layer of software that sits between user applications and the bare hardware. Its primary role is to make the hardware usable, safe, and efficient for multiple programs running concurrently.

At its core, an OS does three things:

1. **Manages hardware resources** — CPU, memory, disk, network interfaces, peripherals.
2. **Provides abstractions** — processes, files, sockets, virtual memory — so that application programmers don't need to deal with raw hardware.
3. **Enforces isolation and protection** — ensures one buggy or malicious program can't take down the entire machine or spy on other programs.

```
 ┌─────────────────────────────────────────────────────┐
 │                  User Applications                   │
 │          (browsers, editors, servers, etc.)           │
 ├─────────────────────────────────────────────────────┤
 │            System Libraries (libc, etc.)              │
 ├─────────────────────────────────────────────────────┤
 │         ┌──────────────────────────────────┐         │
 │         │      System Call Interface        │         │
 │         ├──────────────────────────────────┤         │
 │         │                                  │         │
 │         │       Operating System Kernel     │         │
 │         │  (process mgmt, memory mgmt,     │         │
 │         │   file systems, device drivers,   │         │
 │         │   networking, scheduling)         │         │
 │         │                                  │         │
 │         └──────────────────────────────────┘         │
 ├─────────────────────────────────────────────────────┤
 │                     Hardware                         │
 │   (CPU, RAM, Disk, NIC, GPU, peripherals, etc.)      │
 └─────────────────────────────────────────────────────┘
```

### Linux Perspective

The **Linux kernel** is a monolithic kernel — all core OS services (scheduling, memory management, file systems, device drivers, networking) run in a single address space in kernel mode. This differs from a microkernel (like Minix or QNX) where drivers and services run in user space.

Key characteristics of Linux:
- **Monolithic but modular**: Kernel modules (`.ko` files) can be loaded/unloaded at runtime via `insmod` / `rmmod` / `modprobe`.
- **POSIX-compliant**: Provides the standard UNIX system call interface.
- **Open source**: The entire kernel source is available at [kernel.org](https://kernel.org).

---

## 2. Fundamental OS Concepts: Virtualization, Concurrency, and Persistence

These are the three pillars around which OS design revolves:

### 2.1 Virtualization

The OS creates the *illusion* that each process has its own dedicated CPU and its own private memory, even though the physical machine has limited CPUs and a single shared memory.

- **CPU Virtualization**: The OS time-shares the CPU among many processes. Each process thinks it has the CPU to itself. The mechanism: **context switching** + **scheduling**.
- **Memory Virtualization**: Each process gets its own **virtual address space**. The OS + hardware (MMU) translate virtual addresses to physical addresses. Each process believes it has a contiguous block of memory starting from address 0.

### 2.2 Concurrency

Multiple activities happen at the same time — multiple processes running, multiple threads within a process, I/O operations proceeding in parallel with computation. The OS must manage shared resources correctly:
- **Race conditions** arise when concurrent entities access shared data.
- OS provides **synchronization primitives** (locks, semaphores, condition variables) to coordinate access.

### 2.3 Persistence

Data must survive power loss and crashes. The OS provides:
- **File systems** — organize data on persistent storage (HDD, SSD).
- **Crash consistency** mechanisms — journaling, logging, copy-on-write — to ensure data integrity even if the system crashes mid-write.

---

## 3. What is a Program? How Does a C Program Run on Hardware?

### From Source Code to Execution

```
  source.c
     │
     ▼
 ┌──────────┐
 │ Compiler  │   gcc -S source.c → source.s (assembly)
 └──────────┘
     │
     ▼
 ┌──────────┐
 │ Assembler │   as source.s -o source.o (object file, machine code)
 └──────────┘
     │
     ▼
 ┌──────────┐
 │  Linker   │   ld source.o -lc -o program (resolves symbols, links libraries)
 └──────────┘
     │
     ▼
  program      ← Executable file on disk (e.g., ELF format on Linux)
```

1. **Preprocessing** (`cpp`): Handles `#include`, `#define`, macro expansion.
2. **Compilation** (`cc1`): Translates C to assembly for the target architecture.
3. **Assembly** (`as`): Converts assembly mnemonics to machine code (object file `.o`).
4. **Linking** (`ld`): Combines object files, resolves external symbol references (e.g., `printf` from libc), produces an executable.

The resulting executable is an **ELF (Executable and Linkable Format)** file on Linux. It contains:
- `.text` section — machine code instructions
- `.data` section — initialized global/static variables
- `.bss` section — uninitialized global/static variables (zeroed at load time)
- `.rodata` section — read-only data (string literals, constants)
- Symbol tables, relocation entries, etc.

### Role of the OS in Running a Process

When you type `./program` in a shell:
1. The shell calls `fork()` to create a child process.
2. The child calls `execve("./program", ...)` — this is a system call.
3. The kernel's ELF loader reads the executable, sets up the virtual address space:
   - Maps `.text`, `.data`, `.rodata`, `.bss` into memory.
   - Sets up the stack and heap regions.
   - Sets up the program counter (PC) to the entry point (`_start`, which calls `main()`).
4. The kernel returns to user mode, and the process begins executing.

### Linux Details

- **ELF loader**: The kernel function `load_elf_binary()` (in `fs/binfmt_elf.c`) parses the ELF headers and sets up memory mappings.
- **Dynamic linking**: Most programs use shared libraries. The kernel maps the dynamic linker (`ld-linux.so`) into the process address space, which then loads required `.so` files at runtime.
- You can inspect ELF files with: `readelf -a program`, `objdump -d program`.

---

## 4. Hardware Organization, CPU Instructions, and CPU ISA

### Hardware Organization

A modern computer consists of:

```
          ┌─────────────────────────────────────────────────────────┐
          │                     CPU Package                         │
          │  ┌───────┐ ┌───────┐ ┌───────┐ ┌───────┐               │
          │  │ Core 0│ │ Core 1│ │ Core 2│ │ Core 3│  ...           │
          │  │ (ALU, │ │       │ │       │ │       │               │
          │  │  FPU, │ │       │ │       │ │       │               │
          │  │ Regs, │ │       │ │       │ │       │               │
          │  │L1 I/D)│ │       │ │       │ │       │               │
          │  └───┬───┘ └───┬───┘ └───┬───┘ └───┬───┘               │
          │      └────┬────┘         └────┬────┘                    │
          │           │ L2 Cache         │ L2 Cache                 │
          │           └────────┬─────────┘                          │
          │                    │ L3 Cache (shared)                   │
          └────────────────────┼────────────────────────────────────┘
                               │  Memory Bus / Interconnect
                    ┌──────────┴──────────┐
                    │     Main Memory      │
                    │       (DRAM)         │
                    └──────────┬──────────┘
                               │  I/O Bus (PCIe, etc.)
              ┌────────────────┼────────────────┐
              │                │                │
        ┌─────┴─────┐  ┌──────┴──────┐  ┌──────┴──────┐
        │   Disk     │  │    NIC      │  │    GPU      │
        │ Controller │  │ Controller  │  │             │
        └───────────┘  └─────────────┘  └─────────────┘
```

- **CPU**: Fetches, decodes, and executes instructions. Contains registers (general-purpose, special-purpose like PC/IP, SP, status/flags register).
- **Memory (DRAM)**: Stores both code and data. Volatile — contents lost on power off.
- **I/O Devices**: Disks, network cards, keyboards, displays — connected via buses (PCIe, USB, etc.).
- **DMA Engine**: Specialized hardware that transfers data between memory and I/O devices without CPU involvement.

### CPU Instructions and Instruction Cycle

The CPU operates in a **fetch-decode-execute** cycle:

```
  ┌──────────┐      ┌──────────┐      ┌──────────┐
  │  FETCH   │ ──►  │  DECODE  │ ──►  │ EXECUTE  │ ──► (repeat)
  │ (read    │      │ (figure  │      │ (perform │
  │  instr   │      │  out     │      │  the     │
  │  at PC)  │      │  opcode  │      │  operation)
  └──────────┘      │  & operands)    └──────────┘
                    └──────────┘
```

1. **Fetch**: Read the instruction from memory at the address in the Program Counter (PC).
2. **Decode**: Determine what operation to perform and what operands to use.
3. **Execute**: Perform the operation (arithmetic, load/store, branch, etc.).
4. Update PC to the next instruction (unless a branch/jump).

### CPU ISA (Instruction Set Architecture)

The ISA is the contract between hardware and software. It defines:
- The set of instructions the CPU understands (add, sub, load, store, branch, etc.).
- The registers available.
- The memory addressing modes.
- The data types and sizes.
- The privilege levels.

Two major ISA families:
- **x86-64 (AMD64)**: CISC — complex, variable-length instructions. Dominant in desktops/servers.
- **ARM (AArch64)**: RISC — simpler, fixed-length instructions. Dominant in mobile, growing in servers (AWS Graviton, Apple M-series).

**Linux supports many ISAs**: x86, ARM, RISC-V, MIPS, PowerPC, s390x, etc. The kernel abstracts away ISA-specific details through architecture-specific code in `arch/` directories (e.g., `arch/x86/`, `arch/arm64/`).

### Key Registers (x86-64)

| Register | Purpose |
|----------|---------|
| `RAX`–`RDX`, `RSI`, `RDI`, `R8`–`R15` | General-purpose |
| `RIP` | Instruction pointer (Program Counter) |
| `RSP` | Stack pointer |
| `RBP` | Base pointer (frame pointer) |
| `RFLAGS` | Status flags (zero, carry, overflow, etc.) |
| `CR0`–`CR4` | Control registers (paging enable, protection mode, etc.) |
| `CR3` | Page table base register |
| `CS`, `DS`, `SS`, etc. | Segment registers (used for privilege levels in x86-64) |

---

## 5. Concurrent Execution and CPU Virtualization

Even on a single-core machine, the OS gives the illusion that many processes run simultaneously. This is **CPU virtualization** via **time-sharing**.

### How It Works

```
 Time ──────────────────────────────────────────►

 CPU:  ┌──P1──┐┌──P2──┐┌──P3──┐┌──P1──┐┌──P2──┐
       │ run  ││ run  ││ run  ││ run  ││ run  │ ...
       └──────┘└──────┘└──────┘└──────┘└──────┘
              ↑       ↑       ↑       ↑
          context  context  context  context
          switch   switch   switch   switch
```

- The OS **scheduler** decides which process gets the CPU next.
- A **timer interrupt** fires periodically (e.g., every 1–10 ms), giving the OS an opportunity to preempt the running process and switch to another.
- A **context switch** saves the current process's CPU state (registers, PC, stack pointer) and loads another process's saved state.

On a **multi-core** system, multiple processes truly run in parallel — one per core. The OS scheduler assigns processes to cores.

### Linux Specifics

- The Linux scheduler is the **Completely Fair Scheduler (CFS)** (since kernel 2.6.23), which uses a red-black tree to track process runtimes and ensures fair CPU distribution. (As of kernel 6.6+, the **EEVDF** scheduler has replaced CFS as the default.)
- Timer interrupts on Linux are typically configured via `CONFIG_HZ` (commonly 250 or 1000 Hz). With **tickless** (NO_HZ) kernels, timer interrupts are suppressed when the CPU is idle or running a single task, saving power.
- `sched_setaffinity()` syscall pins a process to specific CPU cores.

---

## 6. Memory Image of a Process / Address Space

When a process is running, its virtual address space is laid out as follows:

```
  High addresses
  ┌─────────────────────────┐  0xFFFFFFFFFFFFFFFF (on 64-bit)
  │                         │
  │    Kernel Space          │  (mapped into every process's
  │    (not accessible       │   address space, but accessible
  │     from user mode)      │   only in kernel mode)
  │                         │
  ├─────────────────────────┤  Typically 0x00007FFFFFFFFFFF boundary
  │         Stack            │  ← grows downward
  │    (local variables,     │
  │     return addresses,    │
  │     function args)       │
  │           │              │
  │           ▼              │
  │                         │
  │      (unmapped gap)      │
  │                         │
  │           ▲              │
  │           │              │
  │         Heap             │  ← grows upward
  │    (dynamically alloc'd  │     via malloc/brk/mmap
  │     memory)              │
  ├─────────────────────────┤
  │   Uninitialized Data     │  (.bss) — zeroed globals/statics
  ├─────────────────────────┤
  │   Initialized Data       │  (.data) — initialized globals/statics
  ├─────────────────────────┤
  │       Text (Code)        │  (.text) — machine instructions
  ├─────────────────────────┤
  │    (unmapped/reserved)   │  Address 0x0 — null pointer dereferences
  └─────────────────────────┘  trap here (SIGSEGV)
  Low addresses
```

Each process gets its own virtual address space. Two processes can have the same virtual address (e.g., 0x400000) mapping to completely different physical memory locations. The hardware **Memory Management Unit (MMU)** performs address translation using **page tables** set up by the OS.

### Linux Specifics

- You can inspect a process's memory layout via `/proc/<pid>/maps`:
  ```
  $ cat /proc/self/maps
  55a34c000000-55a34c001000 r--p 00000000 08:01 1234  /usr/bin/cat
  55a34c001000-55a34c005000 r-xp 00001000 08:01 1234  /usr/bin/cat    <- .text
  55a34c005000-55a34c007000 r--p 00005000 08:01 1234  /usr/bin/cat    <- .rodata
  55a34c009000-55a34c00a000 rw-p 00008000 08:01 1234  /usr/bin/cat    <- .data/.bss
  7f8a12000000-7f8a12200000 rw-p 00000000 00:00 0                      <- heap/anon
  7ffc9e300000-7ffc9e321000 rw-p 00000000 00:00 0     [stack]
  ...
  ```
- **ASLR (Address Space Layout Randomization)**: Linux randomizes the base addresses of stack, heap, mmap region, and shared libraries on each execution to mitigate exploits. Controlled via `/proc/sys/kernel/randomize_va_space`.
- The kernel is mapped into the upper portion of every process's virtual address space (on x86-64, typically above `0xFFFF800000000000`). User processes cannot access this region — the page table entries have the supervisor bit set.

---

## 7. Isolation and Privilege Levels: User Mode and Kernel Mode

### Why Two Modes?

If every application could directly access hardware, modify page tables, or disable interrupts, a single bug or malicious program could:
- Overwrite another process's memory.
- Monopolize the CPU forever (disable timer interrupts).
- Corrupt the file system.

The CPU hardware supports (at minimum) **two privilege levels**:

| Mode | Also Called | Can Do |
|------|-----------|--------|
| **Kernel Mode** | Supervisor mode, Ring 0 (x86) | Execute ALL instructions, access ALL memory, configure hardware |
| **User Mode** | Ring 3 (x86) | Execute non-privileged instructions only, access only its own virtual memory |

```
  ┌─────────────────────────────────┐
  │          User Mode (Ring 3)      │
  │    Application code runs here.   │
  │    Cannot:                       │
  │      - Access hardware directly  │
  │      - Modify page tables        │
  │      - Disable interrupts        │
  │      - Execute privileged instr. │
  └──────────────┬──────────────────┘
                 │ System Call (trap instruction)
                 ▼
  ┌─────────────────────────────────┐
  │         Kernel Mode (Ring 0)     │
  │    OS kernel code runs here.     │
  │    Can:                          │
  │      - Access all hardware       │
  │      - Manage page tables        │
  │      - Handle interrupts         │
  │      - Schedule processes        │
  └─────────────────────────────────┘
```

### x86 Protection Rings

x86 technically has 4 rings (Ring 0–3), but in practice only Ring 0 (kernel) and Ring 3 (user) are used by Linux. (Rings 1 and 2 are unused.)

### How the Transition Happens

- **User → Kernel**: Via a **trap** instruction (`syscall` on x86-64, `svc` on ARM64). This is a synchronous, intentional transition. Also happens on hardware **interrupts** (timer, I/O device) or **exceptions** (page fault, divide by zero).
- **Kernel → User**: Via a return-from-trap instruction (`sysret` on x86-64, `eret` on ARM64). The CPU restores the saved user-mode state and drops privilege.

### Linux Implementation

- The **Current Privilege Level (CPL)** is stored in the lowest 2 bits of the `CS` (code segment) register on x86-64. CPL = 0 means kernel mode; CPL = 3 means user mode.
- On `syscall` instruction:
  1. CPU saves `RIP` and `RFLAGS` into `RCX` and `R11`.
  2. CPU loads the kernel's code segment and instruction pointer from MSR registers (`LSTAR`).
  3. CPU switches to Ring 0.
  4. Kernel entry code (`entry_SYSCALL_64` in `arch/x86/entry/entry_64.S`) saves the rest of the user registers onto the kernel stack.
- `sysret` reverses the process.

---

## 8. System Calls, Interrupts, and Interrupt Handling

### System Calls

A **system call (syscall)** is the controlled mechanism by which a user-mode process requests a service from the kernel.

The flow:
```
  User Process                        Kernel
  ─────────────                       ──────
  1. Load syscall number into RAX
     (e.g., 1 = write, 0 = read)
  2. Load arguments into registers
     (RDI, RSI, RDX, R10, R8, R9)
  3. Execute `syscall` instruction
            ─── trap ──────────►  4. CPU switches to kernel mode
                                  5. Kernel looks up syscall number
                                     in sys_call_table[]
                                  6. Calls the handler (e.g., sys_write)
                                  7. Handler executes, puts return
                                     value in RAX
            ◄── sysret ─────────  8. Return to user mode
  9. User process reads return
     value from RAX
```

**Common Linux syscalls** (there are ~450+ on x86-64):

| Syscall | Number (x86-64) | Purpose |
|---------|-----------------|---------|
| `read` | 0 | Read from a file descriptor |
| `write` | 1 | Write to a file descriptor |
| `open` | 2 | Open a file |
| `close` | 3 | Close a file descriptor |
| `fork` | 57 | Create a child process |
| `execve` | 59 | Replace process image with new program |
| `exit` | 60 | Terminate the process |
| `wait4` | 61 | Wait for child process |
| `mmap` | 9 | Map memory |
| `brk` | 12 | Change data segment size |
| `ioctl` | 16 | Device control |

You can trace syscalls of a running program with `strace`:
```bash
$ strace -c ls    # summary of syscalls
$ strace -e trace=open,read,write ls    # filter specific syscalls
```

The syscall table is defined in the kernel source at `arch/x86/entry/syscalls/syscall_64.tbl`.

### Interrupts

An **interrupt** is an asynchronous signal from hardware (or software) that causes the CPU to stop what it's doing and run an **interrupt handler**.

Types of interrupts:
1. **Hardware interrupts (IRQs)**: Generated by I/O devices (keyboard, disk, NIC, timer). These are **asynchronous** — they can arrive at any time.
2. **Software interrupts / Exceptions**: Generated by the CPU itself:
   - **Faults**: Recoverable errors (e.g., page fault — the OS can load the page and retry).
   - **Traps**: Intentional (e.g., `syscall` instruction, breakpoint `int 3`).
   - **Aborts**: Unrecoverable errors (e.g., hardware failure).

### Differences: System Calls vs. Interrupts

| Aspect | System Call | Hardware Interrupt |
|--------|------------|--------------------|
| **Source** | User process (intentional) | Hardware device (asynchronous) |
| **Timing** | Synchronous — process explicitly invokes it | Asynchronous — can happen at any time |
| **Purpose** | Request OS service | Notify CPU of external event |
| **Mechanism** | `syscall` instruction | IRQ line → Interrupt Controller → CPU |
| **Handler lookup** | `sys_call_table[syscall_nr]` | `IDT[interrupt_vector]` |

Both transitions result in the CPU switching to kernel mode and jumping to a handler.

### Interrupt Handling in Detail

```
  I/O Device
     │
     │  IRQ signal
     ▼
  ┌──────────────────┐
  │ Interrupt         │     (APIC on modern x86 — Advanced
  │ Controller        │      Programmable Interrupt Controller)
  │ (PIC / APIC)     │
  └────────┬─────────┘
           │  interrupt vector number
           ▼
  ┌──────────────────┐
  │      CPU          │
  │  1. Finish        │
  │     current instr │
  │  2. Save state    │
  │     (push RIP,    │
  │      RFLAGS, RSP  │
  │      onto kernel  │
  │      stack)       │
  │  3. Look up IDT   │
  │     [vector]      │
  │  4. Jump to       │
  │     handler       │
  │  5. Switch to     │
  │     Ring 0        │
  └──────────────────┘
           │
           ▼
  ┌──────────────────┐
  │  Interrupt        │
  │  Handler (ISR)    │
  │  - Acknowledge    │
  │    interrupt      │
  │  - Process event  │
  │  - Wake up        │
  │    waiting proc   │
  │  - EOI signal     │
  └──────────────────┘
           │
           ▼
  Return from interrupt (iret)
  → resume interrupted code
```

### Linux Interrupt Handling

- **IDT (Interrupt Descriptor Table)**: A table of 256 entries on x86-64. Each entry contains the address of the handler for that interrupt vector. The kernel sets this up at boot via `idt_setup_apic_and_irq_gates()`.
- **APIC (Advanced Programmable Interrupt Controller)**: Modern x86 systems use a **Local APIC** per CPU core and an **I/O APIC** for routing device interrupts.
- Interrupt handlers in Linux are often split into:
  - **Top half (hardirq)**: Runs immediately with interrupts disabled on that CPU. Does minimal work — acknowledges the interrupt, copies critical data.
  - **Bottom half (softirq / tasklet / workqueue)**: Deferred processing done later with interrupts enabled. Handles the bulk of the work.
- You can inspect interrupt counts per CPU: `cat /proc/interrupts`.
- You can see registered IRQ handlers: `cat /proc/irq/<N>/smp_affinity` to check CPU affinity for an IRQ.

---

## 9. I/O Devices, Device Controllers, and Device Drivers

### I/O Device Model

Every I/O device has two components:

1. **Device Controller (Hardware)**: The electronics that manage the device. Exposes a set of **registers** (status, command, data) that the CPU can read/write.
2. **Device Driver (Software)**: OS code that knows how to talk to a specific device controller. Translates generic OS I/O requests into device-specific register manipulations.

```
  ┌─────────────┐     ┌──────────────────────┐
  │ Application  │     │     Kernel            │
  │  read()     │────►│  VFS / Block Layer    │
  │             │     │         │              │
  └─────────────┘     │    Device Driver       │
                      │    (e.g., nvme, e1000e)│
                      │         │              │
                      └─────────┼──────────────┘
                                │  register read/write
                                │  (MMIO or port I/O)
                      ┌─────────┼──────────────┐
                      │  Device Controller      │
                      │  (hardware on the device)│
                      └─────────┼──────────────┘
                                │
                      ┌─────────┼──────────────┐
                      │  Physical Device         │
                      │  (disk platters, flash   │
                      │   cells, antenna, etc.)  │
                      └────────────────────────┘
```

### How the OS Communicates with Devices

Two mechanisms:

1. **Port-mapped I/O (PIO)**: Uses special CPU instructions (`in` / `out` on x86) to read/write device registers at specific I/O port addresses. These are privileged instructions.
   ```
   outb(0x60, data);   // write 'data' to port 0x60
   val = inb(0x64);    // read from port 0x64
   ```

2. **Memory-mapped I/O (MMIO)**: Device registers are mapped into the physical address space. The CPU reads/writes them like regular memory addresses. The memory controller routes these accesses to the device instead of DRAM.
   ```
   volatile uint32_t *reg = (uint32_t *)0xFE000000;  // device register address
   *reg = command;      // write to device
   status = *reg;       // read from device
   ```

Modern devices (PCIe) predominantly use MMIO. The device's **BAR (Base Address Register)** in its PCIe configuration space tells the OS where in physical memory the device registers are mapped.

### Linux Device Driver Framework

- Drivers are typically kernel modules (`.ko` files).
- Character device drivers register with the VFS via `register_chrdev()`, providing a `file_operations` struct with function pointers for `open`, `read`, `write`, `ioctl`, etc.
- Block device drivers register via `register_blkdev()` and integrate with the block I/O layer.
- Network drivers register with the networking subsystem via `register_netdev()`.
- Inspect loaded drivers: `lsmod`, `lspci -k` (shows driver bound to each PCI device).

---

## 10. Memory and Storage Hierarchy

The memory hierarchy exploits **locality of reference** — programs tend to access the same data (temporal locality) or nearby data (spatial locality) repeatedly.

```
                    ┌───────────────┐
                    │   CPU Regs    │  ~0.3 ns    ~KBs
                    ├───────────────┤
                    │   L1 Cache    │  ~1 ns      ~64 KB per core
                    ├───────────────┤
                    │   L2 Cache    │  ~3-10 ns   ~256 KB-1 MB per core
                    ├───────────────┤
                    │   L3 Cache    │  ~10-30 ns  ~8-64 MB shared
                    ├───────────────┤
                    │  Main Memory  │  ~50-100 ns ~8-512 GB
                    │   (DRAM)      │
                    ├───────────────┤
                    │    SSD        │  ~10-100 μs ~256 GB - 8 TB
                    ├───────────────┤
                    │    HDD        │  ~3-10 ms   ~1-20 TB
                    ├───────────────┤
                    │  Tape/Cloud   │  seconds+   ~unlimited
                    └───────────────┘
              Faster, Smaller, ▲         ▼  Slower, Larger,
              More Expensive                Less Expensive
```

**Key principle**: Each level acts as a **cache** for the level below it.

- **CPU caches** (L1/L2/L3): Managed by hardware. Cache lines are typically 64 bytes. L1 is split into instruction cache (L1i) and data cache (L1d).
- **Main memory (DRAM)**: Managed by the OS (via virtual memory / paging). Acts as a cache for disk.
- **Disk (SSD/HDD)**: Persistent storage. SSDs use NAND flash (no moving parts, ~100x faster than HDDs for random access). HDDs use spinning magnetic platters.

### Linux Specifics

- **Page cache**: Linux caches disk blocks in unused DRAM. `free -h` shows buffer/cache usage. The page cache is managed by the kernel (see `mm/filemap.c`).
- **TLB (Translation Lookaside Buffer)**: A hardware cache for page table entries. TLB misses trigger page table walks. Linux manages TLB flushes on context switches and page table updates.
- **`/proc/cpuinfo`** shows cache sizes. **`lscpu`** gives a quick summary.
- **`perf stat`** can measure cache hit/miss rates:
  ```bash
  $ perf stat -e cache-references,cache-misses ./my_program
  ```
- **`dmidecode`** shows physical memory configuration (requires root).

---

## Summary Table: Key Concepts at a Glance

| Concept | What It Is | Linux Mechanism |
|---------|-----------|-----------------|
| CPU Virtualization | Illusion of dedicated CPU per process | CFS/EEVDF scheduler, timer interrupts, context switching |
| Memory Virtualization | Private address space per process | Page tables, MMU, `CR3` register, `/proc/<pid>/maps` |
| Privilege Levels | Kernel mode vs. User mode | Ring 0 / Ring 3, CPL in CS register |
| System Calls | User→Kernel service requests | `syscall`/`sysret` instructions, `sys_call_table` |
| Interrupts | Async hardware notifications | IDT, APIC, `/proc/interrupts`, top/bottom half |
| Device Drivers | OS code to manage devices | Kernel modules, `file_operations`, MMIO/PIO |
| Memory Hierarchy | Layered storage with caching | Page cache, TLB, CPU caches |
