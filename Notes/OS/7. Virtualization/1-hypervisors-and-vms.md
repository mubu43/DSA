# Hypervisors and Virtual Machines

---

## 1. What Is "The Cloud"?

"The cloud" is simply **other people's computers** — massive data centers operated by providers (AWS, Azure, GCP) where you rent compute, storage, and networking resources on demand.

Behind the scenes, the cloud runs on **virtualization**: a single physical machine is carved into multiple **virtual machines (VMs)**, each appearing to be a standalone computer with its own OS, CPU, memory, and network interfaces.

```
  Physical Machine in a Cloud Data Center:

  ┌───────────────────────────────────────────────────┐
  │                Physical Server                    │
  │  64 cores, 512 GB RAM, 4 NVMe SSDs, 100G NIC     │
  │                                                   │
  │  ┌──────────┐ ┌──────────┐ ┌──────────┐          │
  │  │  VM 1    │ │  VM 2    │ │  VM 3    │  ...     │
  │  │ 8 cores  │ │ 16 cores │ │ 4 cores  │          │
  │  │ 32GB RAM │ │ 128GB RAM│ │ 16GB RAM │          │
  │  │ Ubuntu   │ │ RHEL     │ │ Windows  │          │
  │  │ Customer │ │ Customer │ │ Customer │          │
  │  │    A     │ │    B     │ │    C     │          │
  │  └──────────┘ └──────────┘ └──────────┘          │
  │                                                   │
  │  ┌─────────────────────────────────────────────┐  │
  │  │        Hypervisor (Virtual Machine Monitor)  │  │
  │  └─────────────────────────────────────────────┘  │
  │                                                   │
  │  ┌─────────────────────────────────────────────┐  │
  │  │              Physical Hardware               │  │
  │  └─────────────────────────────────────────────┘  │
  └───────────────────────────────────────────────────┘
```

### Why Virtualization Instead of Bare Metal?

| Factor | Bare Metal | Virtual Machine |
|--------|-----------|----------------|
| **Resource utilization** | Often <20% utilized; rest wasted | Pack multiple VMs onto one server; >80% utilization |
| **Isolation** | One tenant per machine — maximum isolation | Strong isolation via hypervisor; multiple tenants share hardware |
| **Provisioning speed** | Days to weeks (order, ship, rack, configure) | Seconds to minutes (API call) |
| **Elasticity** | Buy for peak; waste during off-peak | Scale up/down with demand |
| **Cost** | Upfront capital expenditure | Pay-per-use (operational expenditure) |
| **Multi-tenancy** | Not possible | Essential for cloud economics |
| **Snapshotting / Migration** | Difficult | Snapshot VM state, live-migrate between hosts |

The core value proposition: **multiplexing**. One physical machine serves many customers. The hypervisor provides the illusion that each customer has their own dedicated hardware.

---

## 2. The Hypervisor (Virtual Machine Monitor)

A **hypervisor** (or **Virtual Machine Monitor — VMM**) is the software layer that creates and manages virtual machines. It virtualizes the underlying hardware so each VM believes it has exclusive access to a full computer.

### The Hypervisor Must Virtualize:

| Resource | How It's Virtualized |
|----------|---------------------|
| **CPU** | Time-share physical cores among VMs; trap privileged instructions |
| **Memory** | Provide each VM a virtual physical address space; nested page tables |
| **Disk** | Virtual disk images (files) backed by physical storage |
| **Network** | Virtual NICs, virtual switches; packets multiplexed onto physical NIC |
| **Devices** | Emulated devices or passthrough (direct hardware access) |

### Type 1 vs. Type 2 Hypervisors

```
  Type 1 (Bare-Metal):              Type 2 (Hosted):

  ┌──────┐ ┌──────┐ ┌──────┐       ┌──────┐ ┌──────┐
  │ VM 1 │ │ VM 2 │ │ VM 3 │       │ VM 1 │ │ VM 2 │
  │ Guest│ │ Guest│ │ Guest│       │ Guest│ │ Guest│
  │  OS  │ │  OS  │ │  OS  │       │  OS  │ │  OS  │
  └──┬───┘ └──┬───┘ └──┬───┘       └──┬───┘ └──┬───┘
     │        │        │               │        │
  ┌──┴────────┴────────┴──┐        ┌───┴────────┴───┐
  │      HYPERVISOR       │        │   HYPERVISOR    │
  │  (runs directly on    │        │ (runs as app    │
  │   hardware)           │        │  on host OS)    │
  └───────────┬───────────┘        └───────┬────────┘
              │                    ┌───────┴────────┐
              │                    │    Host OS      │
              │                    │ (Linux, Windows)│
              │                    └───────┬────────┘
  ┌───────────┴───────────┐        ┌───────┴────────┐
  │    Physical Hardware   │        │Physical Hardware│
  └───────────────────────┘        └────────────────┘

  Examples:                        Examples:
  - KVM (Linux kernel module)      - VirtualBox
  - Xen                            - VMware Workstation
  - VMware ESXi                    - QEMU (pure emulation)
  - Microsoft Hyper-V              - Parallels Desktop
```

**Type 1** runs directly on the hardware — the hypervisor IS the OS (or is part of it, as with KVM which is a Linux kernel module). Better performance; used in production/cloud.

**Type 2** runs on top of a host OS as a regular application. The host OS manages hardware; the hypervisor runs VMs as processes. Easier to use on desktops; more overhead.

**Note on KVM**: KVM is technically a Type 1 hypervisor — it's a kernel module that turns the Linux kernel itself into the hypervisor. But since Linux is the host OS that also runs normal applications, it has characteristics of both types.

---

## 3. Trap-and-Emulate Virtualization

The classical approach to virtualization, described in IBM's early VM systems (1970s).

### CPU Privilege Levels (Rings)

Recall that x86 CPUs have privilege rings:
- **Ring 0**: Kernel mode — can execute ALL instructions, access all hardware.
- **Ring 3**: User mode — restricted; privileged instructions cause a trap (exception).

Normally, the OS runs in Ring 0 and applications in Ring 3.

### The Basic Idea

Run the **guest OS in Ring 3** (user mode) instead of Ring 0. When the guest OS attempts a privileged instruction (e.g., modifying page tables, disabling interrupts), the CPU traps — control transfers to the hypervisor in Ring 0. The hypervisor **emulates** the effect of that instruction for the guest, then returns control.

```
  Trap and Emulate:

  ┌─────────────────────────────────────────────────────┐
  │  Ring 3 (User Mode)                                 │
  │                                                     │
  │  ┌──────────────┐    ┌──────────────────────────┐   │
  │  │ Guest App    │    │ Guest OS Kernel           │   │
  │  │ (thinks it's │    │ (thinks it's in Ring 0,   │   │
  │  │  in Ring 3)  │    │  but actually in Ring 3!) │   │
  │  └──────────────┘    └────────────┬─────────────┘   │
  │                                   │                 │
  │                                   │ Executes        │
  │                                   │ privileged      │
  │                                   │ instruction     │
  │                                   │ (e.g., MOV CR3) │
  │                                   ▼                 │
  │                              ╔═══════════╗          │
  │                              ║   TRAP!   ║          │
  │                              ╚═════╤═════╝          │
  ├────────────────────────────────────┼────────────────┤
  │  Ring 0 (Kernel Mode)             │                 │
  │                                   ▼                 │
  │  ┌─────────────────────────────────────────────┐    │
  │  │              HYPERVISOR (VMM)                │    │
  │  │                                             │    │
  │  │  1. Examine trapped instruction             │    │
  │  │  2. Emulate its effect for the guest        │    │
  │  │     (e.g., update shadow page table)        │    │
  │  │  3. Return to guest OS in Ring 3            │    │
  │  └─────────────────────────────────────────────┘    │
  └─────────────────────────────────────────────────────┘
```

### Why It Works

Every time the guest OS tries to do something that only a real kernel should do, the hardware traps to the hypervisor. The hypervisor acts as an intermediary, maintaining the illusion for the guest while staying in control.

| Guest OS Action | What Happens |
|----------------|-------------|
| Write to page table register (CR3) | Trap → VMM updates shadow page table |
| Disable interrupts (CLI) | Trap → VMM records that guest wants interrupts disabled |
| Access I/O port (IN/OUT) | Trap → VMM emulates the device interaction |
| Execute HLT (halt CPU) | Trap → VMM deschedules the vCPU |

---

## 4. Problems with Trap-and-Emulate on x86

The trap-and-emulate model relies on a critical assumption: **every privileged instruction must cause a trap when executed in user mode (Ring 3)**. This property does not hold on classic x86.

### Sensitive vs. Privileged Instructions

- **Privileged instruction**: Causes a trap (exception) when executed in Ring 3. Example: `MOV CR3` (write to page table base register).
- **Sensitive instruction**: An instruction that behaves differently depending on the privilege level, or that could affect the VMM's control. Example: `POPF` (pop flags — silently ignores the IF bit when in Ring 3 instead of trapping).

```
  The Problem:

  ┌─────────────────────────────────────────────────────┐
  │ Instruction Classification:                         │
  │                                                     │
  │  ┌───────────────────────┐                          │
  │  │    SENSITIVE           │                          │
  │  │  (affect VM state or   │                          │
  │  │   behave differently   │                          │
  │  │   per privilege level) │                          │
  │  │                        │                          │
  │  │  ┌─────────────────┐   │   ┌──────────────────┐  │
  │  │  │  PRIVILEGED     │   │   │ SENSITIVE but    │  │
  │  │  │  (trap in       │   │   │ NOT PRIVILEGED   │  │
  │  │  │   Ring 3)       │   │   │ (silently fail   │  │
  │  │  │                 │   │   │  or behave       │  │
  │  │  │  MOV CR3 ✓      │   │   │  differently)    │  │
  │  │  │  LGDT ✓         │   │   │                  │  │
  │  │  │  HLT ✓          │   │   │  POPF ✗          │  │
  │  │  └─────────────────┘   │   │  SGDT ✗          │  │
  │  │                        │   │  SMSW ✗          │  │
  │  │                        │   │  PUSHF ✗         │  │
  │  └───────────────────────-┘   └──────────────────┘  │
  │                                                     │
  │  The ✗ instructions are the PROBLEM:                │
  │  They're sensitive but DON'T trap.                  │
  │  VMM never gets control → can't emulate correctly.  │
  └─────────────────────────────────────────────────────┘
```

### Specific x86 Problem Instructions

| Instruction | Problem |
|-------------|---------|
| `POPF` | Pops flags register. In Ring 3, silently ignores changes to IF (interrupt flag) — no trap. Guest OS thinks it disabled interrupts, but it didn't. |
| `PUSHF` | Pushes flags register. In Ring 3, returns the actual Ring 3 flags — guest sees it's NOT in Ring 0. Breaks the illusion. |
| `SGDT` / `SIDT` / `SMSW` | Store system registers. Execute without trapping in Ring 3. Guest can detect it's not in Ring 0. |
| `LAR` / `LSL` / `VERR` / `VERW` | Segment descriptor access — reveal actual privilege level. |

These **17 problematic x86 instructions** are why pure trap-and-emulate doesn't work on classic x86 hardware.

---

## 5. The Popek-Goldberg Theorem (1974)

Gerald Popek and Robert Goldberg formalized the requirements for a virtualizable architecture:

> **Theorem**: A computer architecture is **efficiently virtualizable** if the set of **sensitive instructions** is a subset of the set of **privileged instructions**.

$$
\text{Sensitive} \subseteq \text{Privileged} \implies \text{Efficiently virtualizable}
$$

If every sensitive instruction traps in user mode, the VMM can intercept and emulate all of them — trap-and-emulate works perfectly.

```
  Virtualizable Architecture          Non-Virtualizable (x86 pre-VT-x):
  (e.g., IBM System/370):

  ┌────────────────────┐              ┌────────────────────┐
  │   SENSITIVE        │              │   SENSITIVE        │
  │ ┌────────────────┐ │              │ ┌───────────┐      │
  │ │  PRIVILEGED    │ │              │ │ PRIVILEGED│  ┌──┐│
  │ │  (all sensitive│ │              │ │           │  │??││
  │ │   instructions │ │              │ │           │  │  ││
  │ │   trap)        │ │              │ │           │  └──┘│
  │ └────────────────┘ │              │ └───────────┘  ^^^  │
  └────────────────────┘              └───────── problem ──┘

  Sensitive ⊆ Privileged ✓           Sensitive ⊄ Privileged ✗
  → Trap-and-emulate works           → Trap-and-emulate BROKEN
```

**x86 violates this theorem**: It has sensitive instructions that are NOT privileged (they don't trap). This is why x86 virtualization required creative workarounds (paravirtualization, binary translation) until Intel and AMD added hardware extensions (VT-x, AMD-V) in 2005-2006.

---

## 6. Memory Virtualization

Each guest OS manages its own virtual address space, expecting to control "physical" memory via page tables. But the guest's "physical" memory is actually the hypervisor's virtual memory. This creates two levels of translation.

```
  Two Levels of Address Translation:

  Guest Virtual Address (GVA)
        │
        │  Guest page tables (managed by guest OS)
        ▼
  Guest Physical Address (GPA)
        │
        │  Hypervisor mapping (managed by VMM)
        ▼
  Host Physical Address (HPA)
        │
        │  Actual DRAM
        ▼
  ┌──────────────┐
  │ Physical RAM │
  └──────────────┘
```

### Shadow Page Tables

The VMM maintains **shadow page tables** that directly map GVA → HPA (combining both levels). The hardware MMU uses the shadow tables for actual translation.

```
  Shadow Page Tables:

  Guest Page Table (GVA → GPA):      VMM Mapping (GPA → HPA):
  ┌──────────┬────────┐              ┌──────────┬────────┐
  │ GVA 0x1000 → GPA 0x5000 │       │ GPA 0x5000 → HPA 0xA000 │
  │ GVA 0x2000 → GPA 0x8000 │       │ GPA 0x8000 → HPA 0xD000 │
  └──────────┴────────┘              └──────────┴────────┘

  Shadow Page Table (GVA → HPA):
  ┌──────────┬────────┐
  │ GVA 0x1000 → HPA 0xA000 │  ◄── combines both levels
  │ GVA 0x2000 → HPA 0xD000 │      loaded into hardware CR3
  └──────────┴────────┘

  When guest updates its page table, VMM must trap and
  update the shadow table accordingly → overhead.
```

### Extended Page Tables (EPT) / Nested Paging

Modern hardware (Intel EPT, AMD NPT) adds a **second hardware page table walk** so both translations happen in hardware:

```
  Extended Page Tables (EPT):

  GVA ──► [Guest Page Table] ──► GPA ──► [EPT / NPT] ──► HPA
           (guest-managed)                (VMM-managed)
           in guest CR3                   in EPTP register

  Both walks done by hardware MMU — no VMM intervention
  on every page table update. Much faster than shadow tables.
```

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Cloud | Virtualized data centers; rent compute on demand |
| Hypervisor (VMM) | Software that creates/manages VMs; virtualizes CPU, memory, I/O |
| Type 1 hypervisor | Runs on bare metal (KVM, Xen, ESXi, Hyper-V) |
| Type 2 hypervisor | Runs on host OS (VirtualBox, VMware Workstation) |
| Trap-and-emulate | Run guest in Ring 3; trap privileged instructions → emulate in VMM |
| Sensitive instruction | Behaves differently per privilege level or affects VMM control |
| Privileged instruction | Traps when executed outside Ring 0 |
| x86 problem | 17 sensitive-but-not-privileged instructions → trap-and-emulate fails |
| Popek-Goldberg theorem | Architecture virtualizable iff Sensitive ⊆ Privileged |
| Shadow page tables | VMM maintains GVA → HPA tables; overhead on guest PT updates |
| Extended Page Tables | Hardware nested paging (Intel EPT / AMD NPT); eliminates shadow PT overhead |
