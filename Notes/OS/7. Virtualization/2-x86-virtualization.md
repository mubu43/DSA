# x86 Virtualization Techniques

---

## 1. The x86 Virtualization Problem — Recap

Classic x86 cannot be virtualized with pure trap-and-emulate because **17 sensitive instructions do not trap in Ring 3**. Three major solutions emerged:

```
  x86 Virtualization Solutions Timeline:

  1999-2003            2003               2005-2006
  ─────────────────────────────────────────────────────►

  Paravirtualization   Full Virtualization  Hardware-Assisted
  (Xen, 2003)         (VMware, 1999)       (Intel VT-x, AMD-V)
  │                    │                    │
  │ Modify guest OS    │ Binary translation │ CPU hardware support
  │ to avoid problem   │ of problem         │ eliminates the problem
  │ instructions       │ instructions       │ entirely
```

---

## 2. Paravirtualization

### Concept

Instead of trying to run an unmodified guest OS, **modify the guest OS source code** to avoid executing problematic instructions. Replace them with explicit **hypercalls** — direct calls to the hypervisor, analogous to how syscalls invoke the kernel.

```
  Paravirtualization:

  Normal OS (bare metal):            Paravirtualized OS:

  Guest OS code:                     Guest OS code (modified):
  ┌─────────────────────────┐        ┌─────────────────────────┐
  │ CLI  (disable interrupts)│       │ hypercall(DISABLE_IRQ)  │
  │ MOV CR3, eax (set PT)   │        │ hypercall(SET_PT, eax)  │
  │ POPF (restore flags)    │        │ hypercall(SET_FLAGS, x) │
  │ OUT dx, al (I/O port)   │        │ hypercall(IO_OUT, dx,al)│
  └─────────────────────────┘        └──────────┬──────────────┘
                                                │
  Executes directly on CPU            Calls trap to hypervisor
  (Ring 0)                            (Ring 0) which emulates
                                      the operation safely
```

### How Hypercalls Work

A hypercall is essentially a software trap to the VMM, similar to `INT 0x80` or `SYSCALL` for OS system calls:

```
  Hypercall Mechanism:

  Guest OS (Ring 1 or Ring 3):
  ┌─────────────────────────────┐
  │ Instead of:  CLI            │
  │ Guest does:                 │
  │   mov eax, HYPERCALL_CLI    │
  │   vmcall  (or INT xx)       │──── Trap to VMM
  └─────────────────────────────┘
                                        │
  VMM (Ring 0):                         ▼
  ┌─────────────────────────────────────────┐
  │ hypercall_handler:                      │
  │   switch(eax):                          │
  │     case HYPERCALL_CLI:                 │
  │       guest_state[vm].irq_enabled = 0;  │
  │       return to guest;                  │
  │     case HYPERCALL_SET_PT:              │
  │       validate_and_update_pt(arg);      │
  │       return to guest;                  │
  └─────────────────────────────────────────┘
```

### Xen Hypervisor (Primary Example)

Xen is the classic paravirtualized hypervisor (University of Cambridge, 2003). Used in early AWS EC2.

```
  Xen Architecture:

  ┌──────────┐ ┌──────────┐ ┌──────────┐
  │  Dom U   │ │  Dom U   │ │  Dom U   │  ◄── Unprivileged VMs
  │ (Guest)  │ │ (Guest)  │ │ (Guest)  │      (paravirtualized OS)
  │ Modified │ │ Modified │ │ Modified │
  │  Linux   │ │  Linux   │ │  Linux   │
  └────┬─────┘ └────┬─────┘ └────┬─────┘
       │            │            │
       │  hypercalls│            │ hypercalls
       ▼            ▼            ▼
  ┌─────────────────────────────────────────┐
  │               Xen Hypervisor             │  ◄── Ring 0 (or VMX root)
  │         (minimal, thin layer)            │
  └──────────────────┬──────────────────────┘
                     │
  ┌──────────────────┴──────────────────────┐
  │                 Dom 0                    │  ◄── Privileged VM
  │          (Management Domain)             │      Runs device drivers
  │          Full Linux kernel               │      Manages other VMs
  │          Device drivers here             │
  └─────────────────────────────────────────┘
```

**Dom 0** (Domain 0): A special privileged VM that runs the full Linux kernel with real device drivers. It manages other VMs and handles I/O on their behalf.

**Dom U** (Domain Unprivileged): Guest VMs running modified OS kernels. They use hypercalls for privileged operations and split drivers for I/O.

### Pros and Cons

| Pros | Cons |
|------|------|
| Near-native performance (minimal overhead) | Requires modifying guest OS source code |
| Simple hypervisor (thin layer) | Cannot run unmodified/proprietary OSes (e.g., Windows) |
| Guest-aware optimization (batched hypercalls) | Maintenance burden: each OS needs porting effort |

---

## 3. Full Virtualization (Binary Translation)

### Concept

Run an **unmodified** guest OS. The hypervisor scans the guest's instruction stream and **dynamically translates** problematic instructions into safe equivalents at runtime.

This was pioneered by **VMware** (1999) — the first commercial x86 virtualization product.

### How Binary Translation Works

```
  Binary Translation:

  Guest OS code (unmodified):
  ┌──────────────────────────────────┐
  │  ...                             │
  │  CLI            ◄── sensitive!   │
  │  MOV CR3, eax   ◄── privileged  │
  │  POPF           ◄── sensitive!   │
  │  ADD ebx, ecx   ◄── safe        │
  │  ...                             │
  └──────────────────────────────────┘
          │
          ▼  VMM scans code blocks before execution
  ┌──────────────────────────────────┐
  │  Binary Translation Engine:      │
  │                                  │
  │  CLI  →  call vmm_handle_cli     │  (replace with trap to VMM)
  │  MOV CR3 → (already traps, ok)   │  (privileged: natural trap)
  │  POPF →  call vmm_handle_popf   │  (replace with trap to VMM)
  │  ADD  →  ADD ebx, ecx           │  (safe: pass through)
  └──────────────────────────────────┘
          │
          ▼  Translated code block
  ┌──────────────────────────────────┐
  │  call vmm_handle_cli             │
  │  MOV CR3, eax  (traps naturally) │
  │  call vmm_handle_popf            │
  │  ADD ebx, ecx                    │
  └──────────────────────────────────┘
  Executed in place of original code.
```

### Translation Cache

Binary translation is expensive if done for every instruction. VMware uses a **translation cache**: once a code block is translated, the translated version is cached and reused.

```
  Translation Cache:

  Guest Code Address → Translated Block
  ┌────────────────┬──────────────────────┐
  │ 0xC0001000     │ translated_block_17  │
  │ 0xC0001080     │ translated_block_18  │
  │ 0xC0002000     │ translated_block_42  │
  │ ...            │ ...                  │
  └────────────────┴──────────────────────┘

  First execution: translate + cache → slow
  Subsequent executions: cache hit → fast (near-native)
```

### Adaptive Translation

- **User-mode code** (Ring 3 in guest): Runs natively — no translation needed. The CPU is already in Ring 3, and user instructions work identically.
- **Kernel-mode code** (Ring 0 in guest, actually Ring 1 or 3): Binary translation applied — only kernel code contains sensitive instructions.

```
  Selective Translation:

  ┌────────────────────────────────────────┐
  │ Guest User-Mode Code (Ring 3)          │
  │ → Execute DIRECTLY on CPU              │  ◄── no overhead!
  │   (no sensitive instructions here)     │
  ├────────────────────────────────────────┤
  │ Guest Kernel-Mode Code (Ring 0 in VM)  │
  │ → Binary translation applied           │  ◄── overhead only here
  │   (sensitive instructions replaced)    │
  └────────────────────────────────────────┘

  Most application code runs natively at full speed.
  Only guest kernel code (small fraction of execution) is translated.
```

### Pros and Cons

| Pros | Cons |
|------|------|
| Runs unmodified guest OS (including Windows) | Higher overhead than paravirtualization |
| No guest OS modifications needed | Complex implementation (translation engine) |
| User-mode code runs at native speed | Kernel-heavy workloads suffer more overhead |
| Translation cache amortizes cost | Memory overhead for translation cache |

---

## 4. Hardware-Assisted Virtualization (Intel VT-x / AMD-V)

### The Definitive Solution

In 2005-2006, Intel (VT-x / VMX) and AMD (AMD-V / SVM) added CPU extensions that eliminate the fundamental x86 virtualization problem at the hardware level.

### VMX Modes

The CPU gains a **new mode of execution**: VMX (Virtual Machine Extensions). There are now two execution contexts:

- **VMX Root Mode**: The hypervisor runs here. Full privilege; can execute all instructions.
- **VMX Non-Root Mode**: Guest VMs run here. Has its own Ring 0-3, but certain operations cause automatic exit to the VMM.

```
  VMX Architecture:

  ┌─────────────────────────────────────────────────┐
  │              VMX Non-Root Mode                   │
  │           (Guest VM executes here)               │
  │                                                 │
  │  Ring 3: Guest applications                     │
  │  Ring 0: Guest OS kernel                        │
  │          ^^^ Guest thinks it's truly in Ring 0  │
  │          but CPU is in VMX non-root mode         │
  │                                                 │
  │  Guest can execute CLI, POPF, etc.              │
  │  CPU handles them correctly in non-root mode    │
  │  OR triggers a VM Exit if VMM needs to          │
  │  intervene.                                     │
  │                                                 │
  │         ┌──────────┐                            │
  │         │ VM Exit  │  (trap to VMM)             │
  │         └────┬─────┘                            │
  ├──────────────┼──────────────────────────────────┤
  │              ▼                                  │
  │              VMX Root Mode                       │
  │           (Hypervisor / VMM)                     │
  │                                                 │
  │  Ring 0: VMM kernel                             │
  │  Full control over hardware                     │
  │  Handles VM Exits                               │
  │  Configures VMCS (what triggers exits)          │
  │                                                 │
  │         ┌──────────┐                            │
  │         │ VM Entry │  (return to guest)         │
  │         └──────────┘                            │
  └─────────────────────────────────────────────────┘
```

### Key Concepts

**VMCS (Virtual Machine Control Structure)**: A hardware data structure that stores:
- Guest state (registers, CR3, EFLAGS, etc.)
- Host state (VMM registers)
- Control fields: which events cause a VM Exit (configurable!)
- Exit information: why the exit occurred

```
  VMCS:
  ┌─────────────────────────────────────────┐
  │ Guest State Area                        │
  │   CR0, CR3, CR4, EFLAGS, RIP, RSP     │
  │   CS, DS, SS, ES, FS, GS              │
  │   GDTR, IDTR, LDTR, TR                │
  │   MSRs, DR7                            │
  ├─────────────────────────────────────────┤
  │ Host State Area                         │
  │   CR0, CR3, CR4, RIP, RSP             │
  │   CS, DS, SS, ES, FS, GS              │
  ├─────────────────────────────────────────┤
  │ VM-Execution Control Fields             │
  │   Pin-based controls (ext. interrupts)  │
  │   Processor-based controls (HLT, I/O)  │
  │   Exception bitmap (which exceptions    │
  │     cause VM Exit)                      │
  │   I/O bitmap, MSR bitmap               │
  ├─────────────────────────────────────────┤
  │ VM-Exit Information Fields              │
  │   Exit reason, exit qualification       │
  │   Guest-linear/physical address         │
  └─────────────────────────────────────────┘
```

**VM Entry (`VMLAUNCH` / `VMRESUME`)**: Transition from VMX root → non-root. Loads guest state from VMCS. Guest resumes execution.

**VM Exit**: Transition from VMX non-root → root. Saves guest state to VMCS. VMM handles the exit reason.

### VM Exit / VM Entry Flow

```
  ┌────────────┐   VMLAUNCH/    ┌────────────┐
  │            │   VMRESUME     │            │
  │  VMM       │ ──────────►   │  Guest VM  │
  │  (VMX Root)│               │ (VMX       │
  │            │   VM Exit     │  Non-Root) │
  │            │ ◄──────────   │            │
  └────────────┘               └────────────┘

  VM Entry: VMM → Guest
    1. Load guest state from VMCS
    2. CPU enters VMX non-root mode
    3. Guest resumes at saved RIP

  VM Exit: Guest → VMM
    1. Save guest state to VMCS
    2. Load host state from VMCS
    3. CPU enters VMX root mode
    4. VMM reads exit reason, handles it
    5. Eventually does VM Entry to resume guest
```

### What Triggers a VM Exit?

The VMM configures (via VMCS) which events cause exits. Common triggers:

| Event | Why VMM Must Handle It |
|-------|----------------------|
| External interrupt | VMM manages real interrupts; decides which VM gets it |
| `HLT` instruction | VMM deschedules vCPU; runs another VM |
| I/O access (`IN`/`OUT`) | VMM emulates the device |
| `CPUID` instruction | VMM can present virtual CPU features |
| Access to control registers | VMM manages real page tables |
| EPT violation (page fault in nested PT) | VMM manages guest physical memory |
| `VMCALL` (explicit hypercall) | Guest-VMM communication (paravirt extensions) |

### KVM/QEMU — Linux's Hardware-Assisted Virtualization

**KVM** (Kernel-based Virtual Machine) is a Linux kernel module that uses VT-x/AMD-V to turn Linux into a Type 1 hypervisor. **QEMU** provides device emulation.

```
  KVM/QEMU Architecture:

  ┌──────────────────────────────────────────────────────┐
  │                    User Space                        │
  │                                                      │
  │  ┌──────────────────────────────┐  ┌──────────────┐  │
  │  │         QEMU Process         │  │ Other Linux  │  │
  │  │  (one per VM)                │  │ processes    │  │
  │  │                              │  │              │  │
  │  │  ┌────────┐ ┌────────────┐  │  └──────────────┘  │
  │  │  │ vCPU   │ │ Device     │  │                     │
  │  │  │ threads│ │ emulation  │  │                     │
  │  │  │        │ │ (disk,NIC, │  │                     │
  │  │  │        │ │  display)  │  │                     │
  │  │  └───┬────┘ └────────────┘  │                     │
  │  │      │ ioctl(KVM_RUN)       │                     │
  │  └──────┼───────────────────────┘                     │
  ├─────────┼────────────────────────────────────────────┤
  │         ▼     Kernel Space                           │
  │  ┌────────────────────────────────────────────────┐  │
  │  │                 KVM Module                      │  │
  │  │                                                │  │
  │  │  - Manages VMCS for each vCPU                  │  │
  │  │  - Handles VM Exits                            │  │
  │  │  - Manages EPT (nested page tables)            │  │
  │  │  - Issues VMLAUNCH / VMRESUME                  │  │
  │  │  - Exposes /dev/kvm to userspace               │  │
  │  └───────────────────┬────────────────────────────┘  │
  │                      │                               │
  │  Linux kernel: scheduling, memory management,        │
  │  networking — used for the host AND the VMs          │
  ├──────────────────────┼───────────────────────────────┤
  │                      ▼                               │
  │            Physical Hardware                          │
  │            (VT-x / AMD-V enabled CPU)                │
  └──────────────────────────────────────────────────────┘
```

**How KVM runs a vCPU:**

```c
// Simplified QEMU vCPU thread loop:
while (1) {
    ioctl(vcpu_fd, KVM_RUN, 0);  // Enter guest (VM Entry)
    // ... guest executes in VMX non-root mode ...
    // ... VM Exit occurs, returns here ...

    switch (run->exit_reason) {
    case KVM_EXIT_IO:
        handle_io(run);          // QEMU emulates device I/O
        break;
    case KVM_EXIT_MMIO:
        handle_mmio(run);        // QEMU emulates memory-mapped I/O
        break;
    case KVM_EXIT_HLT:
        // Guest halted; vCPU thread sleeps
        break;
    case KVM_EXIT_SHUTDOWN:
        return;
    }
}
```

### The `/dev/kvm` Interface

KVM exposes a character device. QEMU (or any VMM) interacts via `ioctl()`:

| ioctl | Purpose |
|-------|---------|
| `KVM_CREATE_VM` | Create a new VM (returns VM fd) |
| `KVM_CREATE_VCPU` | Create a virtual CPU for the VM |
| `KVM_SET_USER_MEMORY_REGION` | Map guest physical memory to host virtual memory |
| `KVM_RUN` | Enter the guest (VM Entry); blocks until VM Exit |
| `KVM_GET_REGS` / `KVM_SET_REGS` | Read/write vCPU registers |
| `KVM_SET_CPUID2` | Set CPUID values guest will see |

---

## 5. Comparison of x86 Virtualization Techniques

| | Paravirtualization | Binary Translation | Hardware-Assisted |
|-|-------------------|-------------------|-------------------|
| **Guest OS** | Modified source code | Unmodified | Unmodified |
| **Problem instructions** | Replaced with hypercalls | Dynamically translated | Handled by VMX non-root mode |
| **Performance** | Near-native | Good (user code native, kernel translated) | Near-native |
| **Complexity** | Guest OS porting | Translation engine in VMM | CPU hardware + VMM |
| **Windows support** | No (needs source) | Yes | Yes |
| **Example** | Xen (PV mode) | VMware Workstation (early) | KVM, Xen (HVM), VMware ESXi, Hyper-V |
| **Current status** | Mostly legacy | Largely replaced | **Dominant approach** |

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Paravirtualization | Modify guest OS to use hypercalls; avoids problem instructions (Xen) |
| Hypercall | Guest-to-VMM call, analogous to syscall; explicit trap to hypervisor |
| Binary translation | Scan guest code, replace sensitive instructions at runtime (VMware) |
| Translation cache | Cache translated code blocks; amortize translation cost |
| Hardware-assisted (VT-x/AMD-V) | CPU has VMX root/non-root modes; problem solved in hardware |
| VMX root mode | Hypervisor runs here; full privilege |
| VMX non-root mode | Guest runs here; Ring 0-3 with controlled exits to VMM |
| VMCS | Hardware struct storing guest/host state and exit configuration |
| VM Entry | VMM → Guest (VMLAUNCH/VMRESUME); load guest state |
| VM Exit | Guest → VMM; save guest state; VMM handles exit reason |
| KVM | Linux kernel module; uses VT-x; exposes `/dev/kvm` |
| QEMU | Userspace device emulation; paired with KVM for hardware acceleration |
| EPT (Extended Page Tables) | Hardware nested paging: GVA → GPA → HPA in hardware |
