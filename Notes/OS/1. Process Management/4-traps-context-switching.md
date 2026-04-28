# Process Execution Mechanisms: Trap Handling and Context Switching

---

## 1. Concurrent Management of Multiple Processes

The OS must run many processes concurrently on a limited number of CPU cores. This raises two fundamental questions:

1. **How does the OS regain control of the CPU** from a running user process? (A user process is running instructions — the OS code isn't running at all!)
2. **How does the OS switch from one process to another?** (Context switching)

The answers involve a careful interplay between **hardware mechanisms** (privilege levels, trap instructions, timer interrupts) and **OS software** (trap handlers, scheduler, context switch code).

```
  The fundamental challenge:

  Time ──────────────────────────────────────────────────►

  ┌──────────────────┐       ┌────────────────────┐
  │  Process A runs   │       │  Process B runs     │
  │  (user mode)      │       │  (user mode)        │
  │                  │       │                    │
  │  OS is NOT       │       │  OS is NOT         │
  │  running!        │       │  running!          │
  └────────┬─────────┘       └────────┬───────────┘
           │                          │
           ▼                          ▼
     How does OS                How does OS
     get CPU back?              switch to B?
     
  Answer: TRAPS                 Answer: CONTEXT
  (syscalls, interrupts,        SWITCH (save A's
   exceptions)                   state, load B's state)
```

---

## 2. User Mode vs. Kernel Mode (Recap and Deeper Dive)

Recall: the CPU supports at least two privilege levels:

- **User Mode (Ring 3 on x86)**: Restricted. Cannot execute privileged instructions, cannot access kernel memory, cannot disable interrupts.
- **Kernel Mode (Ring 0 on x86)**: Unrestricted. Can do anything — access all memory, configure hardware, manage page tables, handle interrupts.

The CPU checks the **Current Privilege Level (CPL)** on every instruction. On x86-64, the CPL is stored in the lower 2 bits of the `CS` register:
- `CS & 0x3 == 0` → Kernel mode (Ring 0)
- `CS & 0x3 == 3` → User mode (Ring 3)

Privileged instructions attempted in user mode cause a **General Protection Fault (#GP)** — the CPU traps into the kernel, which typically kills the offending process with `SIGSEGV`.

### What Requires Kernel Mode?

| Operation | Why Privileged? |
|-----------|----------------|
| Modifying page tables (`CR3`) | Could access other processes' memory |
| Enabling/disabling interrupts (`cli`/`sti`) | Could hog CPU forever |
| I/O port access (`in`/`out`) | Could corrupt devices |
| Setting IDT (`lidt`) | Could hijack interrupt handling |
| `hlt` (halt CPU) | Could DoS the system |
| Writing MSR registers (`wrmsr`) | Could change syscall entry point |
| Loading GDT/LDT | Could escalate privilege |

---

## 3. Function Call vs. System Call

Both involve transferring control to another piece of code, but they differ fundamentally in privilege and mechanism:

```
  Function Call (e.g., calling printf)     System Call (e.g., calling write)
  ─────────────────────────────────────    ──────────────────────────────────
  
  User Mode ──► User Mode                 User Mode ──► Kernel Mode
  (no privilege change)                    (privilege escalation)
  
  Mechanism:                               Mechanism:
    CALL instruction                         SYSCALL instruction (x86-64)
    - Push return address on stack           - CPU saves RIP → RCX, RFLAGS → R11
    - Jump to function address               - Loads kernel CS, RIP from MSRs
                                             - Switches to Ring 0
  
  Return:                                  Return:
    RET instruction                          SYSRET instruction
    - Pop return address, jump back          - Restores RIP from RCX, RFLAGS from R11
                                             - Switches to Ring 3
  
  Stack:                                   Stack:
    Uses the user stack                      Switches to the kernel stack
    (same address space)                     (different stack entirely)
  
  Cost:                                    Cost:
    ~1-5 ns                                  ~50-200 ns (mode switch, TLB effects,
                                             cache pollution, pipeline flush)
```

### Why is a System Call Expensive?

1. **Mode switch overhead**: The CPU must save and restore additional state, change privilege level.
2. **Pipeline flush**: The CPU pipeline may be flushed on mode transitions.
3. **TLB/cache effects**: Kernel code and data displace user cache lines. On return, user data must be re-fetched.
4. **Kernel entry/exit code**: The kernel must validate arguments (user pointers could be invalid), check permissions, etc.
5. **Spectre/Meltdown mitigations**: Modern kernels perform additional work on transitions (KPTI page table switching, retpoline, IBPB, etc.) that adds significant overhead.

### Measuring Syscall Overhead on Linux

```bash
# A minimal syscall: getpid() (returns cached PID, very little work)
# Use a benchmark to measure overhead:
$ sudo perf stat -e cycles,instructions -- ./getpid_benchmark

# Or use strace with timing:
$ strace -c -e trace=getpid ./my_program
```

Typical `getpid()` round-trip on modern x86-64 Linux: ~100-300 ns (heavily influenced by Spectre mitigations).

---

## 4. Switching to Kernel Mode: Kernel Stack, IDT, and Hardware Trap

### The Trap Mechanism

A **trap** is a synchronous or asynchronous event that causes the CPU to transfer control from user mode to kernel mode. There are three sources of traps:

```
  ┌─────────────────────────────────────────────────────────┐
  │                    TRAP SOURCES                          │
  ├───────────────────┬──────────────────┬──────────────────┤
  │                   │                  │                  │
  │   System Call     │    Exception     │    Interrupt     │
  │   (intentional)   │   (error/fault)  │   (external HW) │
  │                   │                  │                  │
  │ - syscall instr   │ - Page fault     │ - Timer          │
  │ - int 0x80        │ - Div by zero    │ - Disk I/O done  │
  │   (legacy x86)    │ - Invalid opcode │ - Network packet │
  │                   │ - GP fault       │ - Keyboard       │
  │                   │ - Breakpoint     │                  │
  │                   │                  │                  │
  │  Synchronous      │  Synchronous     │  Asynchronous    │
  │  (requested by    │  (caused by      │  (external,      │
  │   the process)    │   the process)   │   any time)      │
  └───────────────────┴──────────────────┴──────────────────┘
```

All three go through a similar hardware mechanism: the CPU looks up a handler in the **IDT (Interrupt Descriptor Table)** and jumps to it in kernel mode.

### The Kernel Stack

Every process (and every thread) has **two stacks**:

1. **User stack**: In the process's virtual address space. Used during normal user-mode execution.
2. **Kernel stack**: A separate, small stack in kernel memory. Used when the process is executing kernel code (handling a syscall, interrupt, or exception).

```
  Process virtual address space:

  ┌─────────────────────┐ High addresses
  │    Kernel Space      │  (not accessible from user mode)
  │                     │
  │  ┌─────────────┐    │  Each process has its own
  │  │ Kernel Stack │    │  kernel stack (8-16 KB on
  │  │ (per-process)│    │  x86-64 Linux).
  │  └─────────────┘    │
  │                     │
  ├─────────────────────┤
  │                     │
  │    User Stack       │  ← RSP points here in user mode
  │    (grows down)     │
  │                     │
  │    ...              │
  │    Heap, Data, Text │
  └─────────────────────┘ Low addresses
```

**Why a separate kernel stack?**
- **Security**: If the kernel used the user stack, a malicious program could craft a stack that compromises the kernel.
- **Reliability**: The user stack might be corrupted, overflowed, or unmapped. The kernel needs a guaranteed-valid stack.
- **Isolation**: Kernel stack contents (return addresses, saved registers, sensitive data) must not be visible to user code.

#### Linux Kernel Stack Details

- Size: Typically **16 KB** on x86-64 (4 pages), configurable via `THREAD_SIZE`.
- Allocated in `alloc_thread_stack_node()` during process/thread creation.
- Stored in `task_struct->stack`.
- The `thread_info` structure sits at the base of the kernel stack (or is in the `task_struct` directly on newer kernels).
- **Stack overflow protection**: Linux uses `CONFIG_VMAP_STACK` to allocate kernel stacks with guard pages, detecting stack overflows instead of silently corrupting memory.

### The IDT (Interrupt Descriptor Table)

The IDT is a table in memory that maps **interrupt/exception vector numbers** (0-255) to **handler addresses**. The CPU uses this table to find where to jump when a trap occurs.

```
  IDT (256 entries on x86-64):
  ┌─────┬───────────────────────────────────────────┐
  │  #  │  Handler                                   │
  ├─────┼───────────────────────────────────────────┤
  │  0  │  divide_error        (Divide by Zero)      │
  │  1  │  debug               (Debug Exception)     │
  │  2  │  nmi_interrupt       (Non-Maskable Int.)   │
  │  3  │  int3                (Breakpoint)           │
  │  6  │  invalid_opcode      (Invalid Opcode)      │
  │  8  │  double_fault        (Double Fault)         │
  │ 13  │  general_protection  (General Prot. Fault)  │
  │ 14  │  page_fault          (Page Fault)           │
  │     │  ...                                        │
  │ 32  │  timer interrupt handler                    │
  │ 33  │  keyboard interrupt handler                 │
  │     │  ...                                        │
  │128  │  ia32_syscall (legacy int 0x80 syscall)     │  (vector 0x80)
  │     │  ...                                        │
  │255  │  spurious_interrupt_bug                     │
  └─────┴───────────────────────────────────────────┘
```

Each IDT entry (called a **gate descriptor**) contains:
- The address of the handler function.
- The kernel code segment selector (Ring 0).
- The gate type (interrupt gate, trap gate).
- The DPL (Descriptor Privilege Level) — which ring is allowed to trigger this entry via `int N` (e.g., the `int 0x80` entry has DPL=3 so user code can invoke it; the page fault entry has DPL=0 since only the CPU itself triggers it).

The IDT is pointed to by the `IDTR` register, loaded by the `lidt` instruction (privileged) at boot time.

#### Linux IDT Setup

The IDT is initialized in `arch/x86/kernel/idt.c`:
- `idt_setup_early_traps()` — sets up critical exception handlers early in boot.
- `idt_setup_traps()` — sets up all exception handlers.
- `idt_setup_apic_and_irq_gates()` — sets up interrupt handlers for devices.
- Entry points are assembly stubs in `arch/x86/entry/entry_64.S`.

### Hardware Trap Instruction: What Happens Step by Step

When a trap occurs (syscall, exception, or interrupt), the **CPU hardware** does the following automatically (no software involvement):

```
  ┌─────────────────────────────────────────────────────────────────┐
  │              HARDWARE ACTIONS ON TRAP (x86-64)                   │
  │                                                                 │
  │  1. Determine the vector number:                                │
  │     - Syscall: special (uses MSRs, not IDT — see below)         │
  │     - Exception: fixed vector (e.g., #14 for page fault)        │
  │     - Interrupt: from APIC (maps IRQ to vector)                 │
  │                                                                 │
  │  2. Look up IDT[vector] to get the handler address              │
  │                                                                 │
  │  3. Check privilege: if transitioning from Ring 3 → Ring 0:     │
  │     a. Load the kernel stack pointer from the TSS               │
  │        (Task State Segment — contains per-CPU RSP0)             │
  │     b. Switch to the kernel stack                               │
  │                                                                 │
  │  4. Push state onto the KERNEL stack (hardware does this):      │
  │     ┌──────────────────┐  ← new RSP (kernel stack top)         │
  │     │   User SS         │  (stack segment selector)             │
  │     │   User RSP        │  (user stack pointer)                 │
  │     │   User RFLAGS     │  (flags register)                     │
  │     │   User CS         │  (code segment — Ring 3)              │
  │     │   User RIP        │  (instruction pointer — return addr)  │
  │     │   Error Code       │  (for some exceptions, e.g., PF)     │
  │     └──────────────────┘                                        │
  │                                                                 │
  │  5. Load CS with kernel code segment (Ring 0)                   │
  │  6. Load RIP with handler address from IDT entry                │
  │  7. Clear IF flag if interrupt gate (disables further ints)     │
  │                                                                 │
  │  CPU is now in Ring 0, executing the kernel handler!             │
  └─────────────────────────────────────────────────────────────────┘
```

**The TSS (Task State Segment)**: On x86-64, the TSS is a per-CPU structure that contains the kernel stack pointer (`RSP0`). When the CPU transitions from Ring 3 to Ring 0, it reads `RSP0` from the TSS to know where the kernel stack is. The kernel sets this up for each process during context switch: `task->thread.sp0` is loaded into the TSS.

### The `syscall` Instruction (Fast Path)

Modern x86-64 Linux uses the `syscall`/`sysret` instructions instead of `int 0x80` for system calls. These are faster because they bypass the IDT lookup:

```
  User code executes: SYSCALL
  
  CPU automatically:
  ┌─────────────────────────────────────────────┐
  │ 1. RCX ← RIP    (save return address)       │
  │ 2. R11 ← RFLAGS (save flags)                │
  │ 3. RIP ← IA32_LSTAR MSR (kernel entry point)│
  │ 4. CS  ← IA32_STAR MSR bits [47:32]  (Ring 0)│
  │ 5. SS  ← IA32_STAR MSR bits [47:32] + 8      │
  │ 6. RFLAGS &= ~IA32_FMASK MSR (mask flags)    │
  └─────────────────────────────────────────────┘
  
  NOTE: SYSCALL does NOT switch the stack!
  The kernel entry code must switch to the kernel stack manually.
```

The kernel entry point for `syscall` on x86-64 Linux is `entry_SYSCALL_64` in `arch/x86/entry/entry_64.S`. This assembly code:
1. Swaps `RSP` to the kernel stack (using the per-CPU `cpu_tss_rw.x86_tss.sp1` or `SWITCH_TO_KERNEL_CR3` for KPTI).
2. Pushes all user registers onto the kernel stack (creates a `pt_regs` structure).
3. Calls `do_syscall_64()` which dispatches to `sys_call_table[rax]`.
4. On return, restores registers from `pt_regs` and executes `sysret`.

### Return from Trap: `iret` and `sysret`

- **`iret`** (Interrupt Return): Used to return from exceptions and interrupts. Pops `RIP`, `CS`, `RFLAGS`, `RSP`, `SS` from the kernel stack. Restores user mode.
- **`sysret`**: Used to return from `syscall`. Faster than `iret` — restores `RIP` from `RCX`, `RFLAGS` from `R11`, switches to Ring 3. But has security concerns (the kernel must be careful about the values in `RCX`/`R11`).

---

## 5. Context Switching: The OS Scheduler's Role

A **context switch** is the process of saving the state of the currently running process and loading the state of the next process to run. It's the mechanism behind CPU virtualization.

### When Does a Context Switch Occur?

1. **Voluntary**: The running process calls a blocking syscall (read, wait, sleep, lock). The process can't continue, so the scheduler picks another.
2. **Involuntary (Preemption)**: The timer interrupt fires. The OS decides the current process has had enough CPU time and switches to another.
3. **Wake-up preemption**: A higher-priority process becomes READY (e.g., I/O completes). The scheduler may preempt the current process immediately.

### The Full Picture: Timer Interrupt → Scheduler → Context Switch

```
  Process A is running in user mode
  ────────────────────────────────────

  ┌─────────────────────────────────────────────────────────────┐
  │ 1. TIMER INTERRUPT fires (from APIC/HPET)                   │
  │    Hardware automatically:                                   │
  │    - Saves A's RIP, RSP, RFLAGS, CS, SS on A's kernel stack │
  │    - Switches to kernel mode (Ring 0)                        │
  │    - Jumps to timer interrupt handler in IDT                 │
  └─────────────────────┬───────────────────────────────────────┘
                        │
                        ▼
  ┌─────────────────────────────────────────────────────────────┐
  │ 2. TIMER INTERRUPT HANDLER (kernel code, running on A's      │
  │    behalf, using A's kernel stack)                            │
  │    - Acknowledges the interrupt (send EOI to APIC)           │
  │    - Updates A's time accounting (vruntime for CFS)          │
  │    - Checks if A has exceeded its time slice                 │
  │    - If yes: sets TIF_NEED_RESCHED flag on A's thread_info   │
  └─────────────────────┬───────────────────────────────────────┘
                        │
                        ▼
  ┌─────────────────────────────────────────────────────────────┐
  │ 3. RETURN PATH checks TIF_NEED_RESCHED                       │
  │    Before returning to user mode, the kernel checks if        │
  │    rescheduling is needed.                                    │
  │    If TIF_NEED_RESCHED is set → call schedule()               │
  └─────────────────────┬───────────────────────────────────────┘
                        │
                        ▼
  ┌─────────────────────────────────────────────────────────────┐
  │ 4. SCHEDULER: schedule() → pick_next_task()                   │
  │    - Removes A from CPU (moves to run queue if still READY)   │
  │    - Consults scheduling policy (CFS: pick process with       │
  │      lowest vruntime from the red-black tree)                 │
  │    - Selects process B as the next to run                     │
  └─────────────────────┬───────────────────────────────────────┘
                        │
                        ▼
  ┌─────────────────────────────────────────────────────────────┐
  │ 5. CONTEXT SWITCH: context_switch(A, B)                       │
  │                                                               │
  │    a. switch_mm(A->mm, B->mm):                                │
  │       - Load B's page table into CR3                          │
  │       - This changes the entire virtual address space!         │
  │       - TLB entries for A are invalidated (or use PCID        │
  │         to avoid full flush)                                   │
  │                                                               │
  │    b. switch_to(A, B):                                        │
  │       - Save A's kernel register context (RSP, RBP, RBX,     │
  │         R12-R15, etc.) into A's task_struct->thread            │
  │       - Load B's kernel register context from B's              │
  │         task_struct->thread                                    │
  │       - Switch kernel stack: RSP now points to B's            │
  │         kernel stack                                           │
  │       - Update the TSS with B's kernel stack pointer (RSP0)   │
  │                                                               │
  │    c. Update 'current' per-CPU variable to point to B          │
  └─────────────────────┬───────────────────────────────────────┘
                        │
                        ▼
  ┌─────────────────────────────────────────────────────────────┐
  │ 6. RETURN TO USER MODE                                        │
  │    Now running on B's kernel stack, the kernel:                │
  │    - Restores B's user-mode registers from B's kernel stack   │
  │      (from the pt_regs saved when B was last interrupted)     │
  │    - Executes iret/sysret to return to user mode              │
  │    - B resumes exactly where it left off!                      │
  └─────────────────────────────────────────────────────────────┘

  Process B is now running in user mode
  ────────────────────────────────────
```

### What State is Saved/Restored?

There are two levels of state saving:

**Level 1: Hardware saves (on trap entry)**
- `RIP`, `CS`, `RFLAGS`, `RSP`, `SS` — pushed onto the kernel stack automatically by the CPU.
- The kernel entry code then saves the remaining general-purpose registers (`RAX`–`R15`) into a `struct pt_regs` on the kernel stack.

**Level 2: Software saves (on context switch)**
- The `switch_to()` macro saves/restores the callee-saved registers (`RBX`, `RBP`, `R12`–`R15`, `RSP`) that weren't already saved by the standard C calling convention.
- FPU/SSE/AVX state is saved lazily or eagerly (depending on `CONFIG_X86_FPU`) using `XSAVE`/`XRSTOR` instructions.
- The kernel stack pointer is saved in `task_struct->thread.sp`.

```
  Context switch state saving (two layers):

  ┌─────────────────────────────────────────────┐
  │  A's Kernel Stack                            │
  │                                             │
  │  ┌─────────────────────┐ ← saved by HW      │
  │  │ User SS, RSP        │   on trap entry     │
  │  │ User RFLAGS         │                     │
  │  │ User CS, RIP        │                     │
  │  ├─────────────────────┤                     │
  │  │ General-purpose     │ ← saved by kernel   │
  │  │ regs (pt_regs)      │   entry code        │
  │  │ RAX, RBX, RCX, RDX │                     │
  │  │ RSI, RDI, RBP       │                     │
  │  │ R8-R15              │                     │
  │  ├─────────────────────┤                     │
  │  │ Kernel function     │ ← normal kernel     │
  │  │ call frames         │   execution         │
  │  │ (schedule, ...)     │                     │
  │  ├─────────────────────┤                     │
  │  │ switch_to saves:    │ ← saved by          │
  │  │ kernel RSP, RBP     │   context_switch()  │
  │  │ callee-saved regs   │                     │
  │  └─────────────────────┘                     │
  │                                             │
  │  task_struct->thread.sp = kernel RSP         │
  │  task_struct->thread.sp0 → TSS.RSP0          │
  └─────────────────────────────────────────────┘
```

### Context Switch Cost

A context switch is expensive due to:

| Cost Source | Impact |
|-------------|--------|
| **Direct cost**: Saving/restoring registers | ~microseconds |
| **Indirect cost**: TLB flush | Major — subsequent memory accesses incur TLB misses |
| **Indirect cost**: Cache pollution | Kernel + new process code/data displaces old process's cache lines |
| **Indirect cost**: Pipeline flush | CPU pipeline is flushed on privilege level changes |
| **Indirect cost**: Branch predictor pollution | New process has different branch patterns |

Typical total context switch time: **1-10 μs** on modern hardware, but the indirect costs (cache/TLB misses) can cause slowdowns for hundreds of microseconds after the switch.

### Linux PCID Optimization

**PCID (Process Context Identifier)** (Intel) / **ASID (Address Space Identifier)** (ARM) allows the TLB to hold entries from multiple address spaces simultaneously, tagged by an identifier. On context switch, instead of flushing the entire TLB, the kernel just changes the PCID/ASID. TLB entries from the old process remain valid and will be reused if the kernel switches back to that process.

Linux enables PCID support on x86-64 (when available) to significantly reduce context switch overhead. This is handled in `arch/x86/mm/tlb.c`.

---

## 6. Putting It All Together: Complete Lifecycle of a Trap

### Example: Process A Makes a `read()` Syscall

```
  ┌─────────────────────────────────────────────────────────────┐
  │ USER MODE — Process A                                        │
  │                                                             │
  │   n = read(fd, buf, count);                                  │
  │         │                                                   │
  │         ▼                                                   │
  │   glibc wrapper:                                            │
  │     mov $0, %rax         // syscall number 0 = read          │
  │     mov fd, %rdi         // arg1: file descriptor            │
  │     mov buf, %rsi        // arg2: buffer                     │
  │     mov count, %rdx      // arg3: count                      │
  │     syscall              // TRAP!                            │
  └──────────────────────┬──────────────────────────────────────┘
                         │
  ═══════════════════════╪══════════════════════════════════════
    Privilege transition │ (Ring 3 → Ring 0)
  ═══════════════════════╪══════════════════════════════════════
                         │
  ┌──────────────────────▼──────────────────────────────────────┐
  │ KERNEL MODE — running on A's kernel stack                    │
  │                                                             │
  │ entry_SYSCALL_64:          // arch/x86/entry/entry_64.S      │
  │   - Switch to kernel stack                                   │
  │   - Save user registers into pt_regs on kernel stack         │
  │   - Call do_syscall_64()                                     │
  │       └─ sys_call_table[0] → ksys_read(fd, buf, count)      │
  │                                                             │
  │ ksys_read():                                                 │
  │   - Look up file descriptor → struct file                    │
  │   - Call file->f_op->read() (VFS dispatch)                   │
  │   - If data is in page cache → copy to user buffer, return   │
  │   - If data not cached → submit I/O request to disk          │
  │                                                             │
  │ *** DATA NOT IN CACHE — I/O NEEDED ***                       │
  │                                                             │
  │   - Submit block I/O request to disk driver                  │
  │   - Set A's state to TASK_UNINTERRUPTIBLE                    │
  │   - Add A to the I/O wait queue                              │
  │   - Call schedule()                                          │
  │       │                                                     │
  │       ├─ pick_next_task() → selects Process B                │
  │       ├─ context_switch(A, B)                                │
  │       │    ├─ switch_mm(): load B's page tables into CR3     │
  │       │    ├─ switch_to(): save A's kernel regs,             │
  │       │    │               load B's kernel regs              │
  │       │    │               RSP now = B's kernel stack         │
  │       │    └─ update 'current' to B                          │
  │       │                                                     │
  │       └─ NOW RUNNING ON B's KERNEL STACK                     │
  │                                                             │
  │ B's kernel return path:                                      │
  │   - Restore B's user registers from B's pt_regs              │
  │   - sysret/iret → return to B's user mode                   │
  └─────────────────────────────────────────────────────────────┘

  ═══════════════════════════════════════════════════════════════
    ... time passes, disk completes I/O ...
  ═══════════════════════════════════════════════════════════════

  ┌─────────────────────────────────────────────────────────────┐
  │ DISK INTERRUPT — arrives while some process C is running     │
  │                                                             │
  │ Timer or disk interrupt fires:                               │
  │   - CPU traps into kernel (saves C's state)                  │
  │   - Interrupt handler: disk I/O is complete!                  │
  │   - DMA'd data is now in page cache                          │
  │   - Wake up Process A: move from TASK_UNINTERRUPTIBLE        │
  │     to TASK_RUNNING, add to run queue                        │
  │   - Return from interrupt (back to C, or schedule A if        │
  │     A has higher priority)                                   │
  └─────────────────────────────────────────────────────────────┘

  ═══════════════════════════════════════════════════════════════
    ... eventually A gets scheduled again ...
  ═══════════════════════════════════════════════════════════════

  ┌─────────────────────────────────────────────────────────────┐
  │ KERNEL MODE — Process A resumes                              │
  │                                                             │
  │ context_switch(?, A):                                        │
  │   - Load A's page tables into CR3                            │
  │   - Restore A's kernel registers from task_struct->thread    │
  │   - RSP now = A's kernel stack                               │
  │                                                             │
  │ A resumes in schedule(), after the switch_to() call          │
  │   - schedule() returns back to ksys_read()                   │
  │   - Data is now in page cache                                │
  │   - Copy data from page cache to user buffer (buf)           │
  │   - Return number of bytes read                              │
  │                                                             │
  │ Return to user mode:                                         │
  │   - Restore A's user registers from pt_regs                  │
  │   - sysret → Ring 3                                         │
  └─────────────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────────────┐
  │ USER MODE — Process A                                        │
  │                                                             │
  │   n = read(fd, buf, count);   // returns with n = bytes read │
  │   // A has no idea it was sleeping! It just sees read() return│
  └─────────────────────────────────────────────────────────────┘
```

---

## 7. Key Linux Kernel Source References

| Component | Source Location | Description |
|-----------|---------------|-------------|
| Syscall entry (x86-64) | `arch/x86/entry/entry_64.S` | Assembly entry point for `syscall` instruction |
| Syscall dispatch | `arch/x86/entry/common.c` (`do_syscall_64()`) | Looks up and calls handler from `sys_call_table` |
| Syscall table | `arch/x86/entry/syscalls/syscall_64.tbl` | Maps syscall numbers to handler functions |
| IDT setup | `arch/x86/kernel/idt.c` | Initializes the Interrupt Descriptor Table |
| Exception handlers | `arch/x86/kernel/traps.c` | Handlers for CPU exceptions (divide error, GP fault, etc.) |
| Page fault handler | `arch/x86/mm/fault.c` (`do_page_fault()`) | Handles page faults |
| Context switch | `kernel/sched/core.c` (`context_switch()`) | Orchestrates mm switch and register switch |
| Register switch | `arch/x86/kernel/process_64.c` (`__switch_to()`) | Saves/restores x86-64 specific state |
| Stack switch asm | `arch/x86/entry/entry_64.S` (`switch_to` macro) | Assembly for switching kernel stacks |
| TSS management | `arch/x86/kernel/process.c` | Per-CPU Task State Segment |
| Timer interrupt | `kernel/time/tick-common.c`, `kernel/sched/core.c` | Timer tick handling and scheduler invocation |

---

## Summary

| Concept | Key Idea | Mechanism |
|---------|----------|-----------|
| User/Kernel modes | Isolation — user code can't access hardware directly | CPL in CS register, Ring 0/3 |
| System call | Controlled entry into kernel | `syscall`/`sysret` instructions, MSR registers |
| Exception | CPU-generated trap on error | IDT lookup, automatic stack switch |
| Hardware interrupt | External device notification | APIC → IDT lookup, kernel stack switch |
| Kernel stack | Safe stack for kernel execution | Per-process, 16 KB, address in TSS |
| IDT | Maps vectors to handlers | 256 entries, loaded with `lidt` |
| Context switch | Save old process state, load new | `switch_mm()` (page tables) + `switch_to()` (registers) |
| Timer interrupt | Enables preemption | APIC timer → `TIF_NEED_RESCHED` → `schedule()` |
