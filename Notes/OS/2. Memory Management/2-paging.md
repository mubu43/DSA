# Paging

---

## 1. Paging Overview

Paging is the memory management scheme used by all modern operating systems. The core idea:

- The **virtual address space** is divided into fixed-size blocks called **pages** (typically 4 KB).
- **Physical memory (RAM)** is divided into blocks of the same size called **frames** (or **page frames**).
- A **page table** maps virtual page numbers (VPNs) to physical frame numbers (PFNs).
- The mapping is completely flexible — any virtual page can map to any physical frame, in any order.

The CPU's **MMU (Memory Management Unit)** performs the translation on every memory access, consulting the page table stored in memory.

---

## 2. Page Table Concept and Structure

A page table is a per-process data structure that stores the VPN → PFN mappings. In its simplest form, it's an array indexed by VPN.

### Simple (Single-Level) Page Table

```
  Virtual Address (e.g., 32-bit, 4 KB pages):
  ┌────────────────────────┬──────────────┐
  │   VPN (20 bits)        │ Offset (12b) │
  │   (Virtual Page Number)│ (within page)│
  └────────────┬───────────┴──────┬───────┘
               │                  │
               ▼                  │
  ┌──────────────────────┐        │
  │     Page Table        │        │
  │  (array of PTEs)      │        │
  ├──────┬───────────────┤        │
  │ VPN 0│ PFN=5, flags  │        │
  ├──────┼───────────────┤        │
  │ VPN 1│ PFN=9, flags  │        │
  ├──────┼───────────────┤        │
  │ VPN 2│ INVALID       │        │
  ├──────┼───────────────┤        │
  │ VPN 3│ PFN=1, flags  │ ◄─────┘ (lookup by VPN)
  ├──────┼───────────────┤
  │ ...  │ ...           │
  └──────┴───────────────┘
               │
               ▼
  Physical Address = PFN × Page_Size + Offset
```

### Address Translation Using Paging (VPN → PFN)

Given a virtual address, the translation works as follows:

```
  Virtual Address (VA):
  ┌──────────────────┬─────────────┐
  │       VPN        │   Offset    │
  └────────┬─────────┴──────┬──────┘
           │                │
           │   ┌────────────┘
           │   │
           ▼   │
  Page Table[VPN] → PTE                    
           │                               
           ▼                               
  ┌────────────────┐                       
  │  PTE contains: │                       
  │  PFN + flags   │                       
  └───────┬────────┘                       
          │                                
          ▼                                
  Physical Address (PA):                   
  ┌──────────────────┬─────────────┐       
  │       PFN        │   Offset    │  ← Same offset, different frame
  └──────────────────┴─────────────┘       
```

**Key insight**: The **offset** is passed through unchanged — it identifies the byte within the page/frame. Only the page/frame number changes.

### Concrete Example

```
  Configuration: 32-bit virtual address, 4 KB (4096 = 2^12 byte) pages
  
  Page size = 4 KB = 2^12 bytes → Offset = 12 bits
  Virtual address = 32 bits → VPN = 32 - 12 = 20 bits
  Number of virtual pages = 2^20 = 1,048,576 pages
  
  Example VA: 0x00003A7C
  
  Binary: 0000 0000 0000 0000 0011 1010 0111 1100
          ├─────── VPN (20 bits) ─────┤├─ Offset ─┤
          0000 0000 0000 0000 0011 10   10 0111 1100
          VPN = 0x0000E = 3             Offset = 0xA7C
  
  Wait, let me redo this cleanly:
  
  VA = 0x00003A7C
  In binary (32 bits): 00000000 00000000 00111010 01111100
  
  Split at bit 12:
    VPN    = upper 20 bits = 0x00003  (decimal 3)
    Offset = lower 12 bits = 0xA7C   (decimal 2684)
  
  Look up Page Table[3]:
    PTE says PFN = 7 (for example)
  
  Physical Address:
    PA = PFN × 4096 + Offset
    PA = 7 × 4096 + 2684 = 28672 + 2684 = 31356
    PA = 0x7A7C
    
  Equivalently:
    PA = (PFN << 12) | Offset = (0x7 << 12) | 0xA7C = 0x7A7C
```

---

## 3. Page Table Entry (PTE)

Each entry in the page table is a **Page Table Entry (PTE)**. It contains more than just the PFN — it also has control/flag bits:

```
  Page Table Entry (x86-64, 64 bits):
  ┌──────────────────────────────────────────────────────────────────┐
  │ 63│62:52│51:12 (40 bits)      │11:9│8│7│6│5│4│3│2│1│0│          │
  ├───┼─────┼─────────────────────┼────┼─┼─┼─┼─┼─┼─┼─┼─┼─┤          │
  │NX │Avail│  Physical Frame #   │Avl │G│S│D│A│C│T│U│W│P│          │
  │   │     │  (PFN, bits 51:12)  │   │ │ │ │ │D│W│/│/│ │          │
  │   │     │                     │   │ │ │ │ │ │ │S│R│ │          │
  └───┴─────┴─────────────────────┴────┴─┴─┴─┴─┴─┴─┴─┴─┴─┘          
```

### PTE Flag Bits

| Bit | Name | Meaning |
|-----|------|---------|
| **P** (bit 0) | **Present** | 1 = page is in physical memory. 0 = not in memory (unmapped or swapped out). Accessing a non-present page triggers a **page fault**. |
| **W/R** (bit 1) | **Read/Write** | 0 = read-only. 1 = read and write allowed. Writing to a read-only page → page fault (used for COW). |
| **U/S** (bit 2) | **User/Supervisor** | 0 = only kernel (Ring 0) can access. 1 = user mode (Ring 3) can also access. This is how kernel memory is protected from user processes. |
| **PWT** (bit 3) | **Page Write-Through** | Controls caching policy — write-through vs. write-back. |
| **PCD** (bit 4) | **Page Cache Disable** | If set, page is not cached. Used for memory-mapped I/O (device registers). |
| **A** (bit 5) | **Accessed** | Set by hardware when the page is read or written. Used by OS for page replacement decisions. |
| **D** (bit 6) | **Dirty** | Set by hardware when the page is written to. Tells OS the page has been modified and must be written back to disk before eviction. |
| **PS** (bit 7) | **Page Size** | In page directory entries: 0 = points to next-level table. 1 = this is a large page (2 MB or 1 GB). |
| **G** (bit 8) | **Global** | Page is not flushed from TLB on CR3 change. Used for kernel pages mapped in all processes. |
| **NX** (bit 63) | **No Execute** | If set, code cannot be executed from this page. Critical for security — prevents executing data as code (DEP/W^X). |

### Size of Page Tables

**Problem**: A single-level page table for a large address space is enormous.

```
  32-bit address space, 4 KB pages:
    VPN = 20 bits → 2^20 = 1 million entries
    PTE size = 4 bytes
    Page table size = 4 MB per process
    
  64-bit address space (48-bit used), 4 KB pages:
    VPN = 36 bits → 2^36 = 64 billion entries
    PTE size = 8 bytes
    Page table size = 512 GB per process ← ABSURD!
```

Even 4 MB per process (32-bit) is wasteful — most of those entries would be "not present" for a typical process that uses only a small fraction of the address space. And 512 GB per process is clearly impossible.

**Solution**: Multi-level page tables.

---

## 4. Address Translation in the MMU

The MMU is hardware integrated into the CPU. On every memory access:

1. CPU generates a virtual address.
2. MMU extracts the VPN.
3. MMU checks the **TLB (Translation Lookaside Buffer)** — a hardware cache of recent VPN→PFN translations.
   - **TLB hit**: PFN found instantly. Form the physical address. Done.
   - **TLB miss**: MMU must **walk the page table** in memory to find the PFN.
4. MMU reads the PTE from the page table in RAM.
5. Checks the Present bit and permission bits.
   - If valid: Cache the translation in the TLB. Form the physical address.
   - If not present or permission violation: Raise a **page fault** exception. CPU traps into kernel.
6. The physical address is sent to the memory controller to fetch the data.

```
  ┌──────┐  VA    ┌─────────────────────────────────────┐
  │ CPU  │──────►│                 MMU                   │
  └──────┘       │                                      │
                 │  ┌────────────┐                      │
                 │  │    TLB     │ ← small, fast cache  │
                 │  │  (64-1024  │   of recent VPN→PFN  │
                 │  │  entries)  │   translations        │
                 │  └─────┬──────┘                      │
                 │    hit │  miss                        │
                 │        │    │                         │
                 │   ┌────┘    └─────┐                  │
                 │   │               │                  │
                 │   ▼               ▼                  │  PA
                 │  Form PA     Page Table Walk ────────┼─────► RAM
                 │              (read PTE from RAM)     │
                 │                   │                  │
                 │              Fill TLB with new entry │
                 │              Form PA                 │
                 └─────────────────────────────────────┘
```

### TLB (Translation Lookaside Buffer)

The TLB is critical for performance. Without it, every memory access would require additional memory accesses just to read the page table (page table walk).

| Property | Typical Value |
|----------|--------------|
| Size | 64–1536 entries (L1 TLB: 64, L2 TLB: 512–1536) |
| Lookup time | 1 cycle (~0.3 ns) |
| Miss penalty | 10–100+ cycles (page table walk) |
| Associativity | Fully associative or 4-way/8-way set associative |
| Separate I/D? | Yes — L1 iTLB (instruction) and L1 dTLB (data), unified L2 TLB |

**TLB hit rate** is typically 99%+ for most workloads due to spatial and temporal locality.

**TLB management**:
- On **context switch**: The OS must ensure process A's TLB entries aren't used by process B (different page tables). Options:
  - **Flush the entire TLB** (expensive — all entries lost).
  - **Use ASIDs / PCIDs** (Process Context Identifiers): Tag TLB entries with a process ID. On switch, just change the PCID; old entries remain valid for when that process runs again.
- On x86-64: PCID support is available since Westmere. Linux uses it (when available) to avoid full TLB flushes, controlled by `CR4.PCIDE`.

```bash
# Check PCID support:
$ grep pcid /proc/cpuinfo
```

---

## 5. Multi-Level Page Tables

### The Problem with Single-Level Tables

A flat page table must have an entry for **every** possible virtual page, even if most of the address space is unmapped. For a 48-bit address space with 4 KB pages:
- 2^36 PTEs × 8 bytes = **512 GB** per process. Clearly impossible.
- Even for a 32-bit system: 2^20 PTEs × 4 bytes = **4 MB** per process, and most entries are "not present."

### The Solution: Hierarchical (Multi-Level) Page Tables

Instead of one giant flat array, use a **tree structure**. Only allocate page table pages for regions of the address space that are actually mapped.

### 2-Level Page Table (32-bit Example)

Split the VPN into two parts: an index into the **page directory** (level 1) and an index into a **page table** (level 2).

```
  32-bit VA, 4 KB pages, 2-level page table:
  
  ┌──────────────┬──────────────┬──────────────┐
  │ PD Index     │ PT Index     │   Offset     │
  │ (10 bits)    │ (10 bits)    │  (12 bits)   │
  └──────┬───────┴──────┬───────┴──────┬───────┘
         │              │              │
         ▼              │              │
  ┌──────────────┐      │              │
  │ Page Directory│      │              │
  │ (1024 entries)│      │              │
  │              │      │              │
  │ [PD Index]──┼──►   │              │
  │   points to  │  │   │              │
  │   a PT page  │  │   │              │
  └──────────────┘  │   │              │
                    ▼   ▼              │
            ┌──────────────┐           │
            │  Page Table   │           │
            │ (1024 entries)│           │
            │              │           │
            │ [PT Index]──┼──► PFN    │
            │              │    │      │
            └──────────────┘    │      │
                                ▼      ▼
                    Physical Address = (PFN << 12) | Offset
```

**Walk for VA = 0x00403004**:

```
  VA = 0x00403004 (binary: 0000 0000 0100 0000 0011 0000 0000 0100)
  
  PD Index  = bits 31:22 = 0000000001 = 1
  PT Index  = bits 21:12 = 0000000011 = 3
  Offset    = bits 11:0  = 000000000100 = 4
  
  Step 1: CR3 points to the Page Directory (physical address of PD).
  Step 2: Read PD[1] → get physical address of the Page Table.
  Step 3: Read PT[3] → get PFN (say, PFN = 0x8A).
  Step 4: PA = (0x8A << 12) | 0x004 = 0x8A004.
```

**Memory savings**: If a process only uses a small part of the address space, most Page Directory entries point to **nothing** (NULL / not present). The corresponding Page Table pages are never allocated. Only the PD (4 KB) plus a few PT pages need to exist.

```
  Address space mostly empty:
  
  Page Directory (always allocated, 4 KB):
  ┌──────┬──────────┐
  │  0   │  → PT_0  │  ← code/data region (allocated)
  ├──────┼──────────┤
  │  1   │  → PT_1  │  ← heap region (allocated)
  ├──────┼──────────┤
  │  2   │  NULL    │  ← unmapped (NOT allocated)
  ├──────┼──────────┤
  │ ...  │  NULL    │  ← unmapped (NOT allocated)
  ├──────┼──────────┤
  │ 1023 │  → PT_N  │  ← stack region (allocated)
  └──────┴──────────┘
  
  Total: 4 KB (PD) + 3 × 4 KB (PTs) = 16 KB
  vs. flat table: 4 MB
```

### 4-Level Page Table (x86-64, Linux)

64-bit systems use only 48 bits of the virtual address (bits 47:0) on current x86-64 hardware (with 57-bit support available via 5-level paging). The VPN is split into four 9-bit indices:

```
  48-bit Virtual Address (x86-64, 4 KB pages):
  
  ┌─────────┬─────────┬─────────┬─────────┬──────────────┐
  │ PGD     │ PUD     │ PMD     │ PTE     │   Offset     │
  │ (9 bits)│ (9 bits)│ (9 bits)│ (9 bits)│  (12 bits)   │
  │ bits    │ bits    │ bits    │ bits    │  bits        │
  │ 47:39   │ 38:30   │ 29:21   │ 20:12   │  11:0        │
  └────┬────┴────┬────┴────┬────┴────┬────┴──────┬───────┘
       │         │         │         │           │
       ▼         │         │         │           │
  ┌─────────┐    │         │         │           │
  │  PGD    │    │         │         │           │
  │ (Page   │    │         │         │           │
  │ Global  │    │         │         │           │
  │ Dir)    │    │         │         │           │
  │ 512     │    │         │         │           │
  │ entries │    │         │         │           │
  └────┬────┘    │         │         │           │
       │         ▼         │         │           │
       │    ┌─────────┐    │         │           │
       └───►│  PUD    │    │         │           │
            │ (Page   │    │         │           │
            │ Upper   │    │         │           │
            │ Dir)    │    │         │           │
            │ 512     │    │         │           │
            │ entries │    │         │           │
            └────┬────┘    │         │           │
                 │         ▼         │           │
                 │    ┌─────────┐    │           │
                 └───►│  PMD    │    │           │
                      │ (Page   │    │           │
                      │ Middle  │    │           │
                      │ Dir)    │    │           │
                      │ 512     │    │           │
                      │ entries │    │           │
                      └────┬────┘    │           │
                           │         ▼           │
                           │    ┌─────────┐      │
                           └───►│  PTE    │      │
                                │ (Page   │      │
                                │ Table)  │      │
                                │ 512     │      │
                                │ entries │      │
                                └────┬────┘      │
                                     │           │
                                     ▼           ▼
                           Physical Address = (PFN << 12) | Offset
  
  CR3 register points to the PGD (physical address).
```

**Key facts about x86-64 4-level paging**:

| Level | Linux Name | Entries | Bits Indexed | Each Entry Covers |
|-------|-----------|---------|-------------|-------------------|
| 4 (top) | PGD (Page Global Directory) | 512 | bits 47:39 | 512 GB |
| 3 | PUD (Page Upper Directory) | 512 | bits 38:30 | 1 GB |
| 2 | PMD (Page Middle Directory) | 512 | bits 29:21 | 2 MB |
| 1 (bottom) | PTE (Page Table Entry) | 512 | bits 20:12 | 4 KB |

- Each table page is exactly **4 KB** (512 entries × 8 bytes per entry).
- 9 bits per index because 2^9 = 512 entries, and 512 × 8 bytes = 4096 bytes = one page. **Page tables themselves fit in one page** — this is elegant and deliberate.
- The page table walk requires **4 memory accesses** (one per level) in the worst case. With TLB caching, this rarely happens.

### 4-Level Walk Example (x86-64)

```
  VA = 0x00007F4A3B2C1D0E (48-bit canonical address)
  
  Decompose (each index is 9 bits):
    PGD index = bits 47:39 = 0x0FE = 254
    PUD index = bits 38:30 = 0x128 = 296  
    PMD index = bits 29:21 = 0x196 = 406
    PTE index = bits 20:12 = 0x0C1 = 193
    Offset    = bits 11:0  = 0xD0E = 3342

  Walk:
  1. Read CR3 → physical address of PGD (e.g., 0x1A3000)
  2. Read PGD[254] at PA 0x1A3000 + 254*8 → PUD base (e.g., PA 0x2B5000)
  3. Read PUD[296] at PA 0x2B5000 + 296*8 → PMD base (e.g., PA 0x3C7000)
  4. Read PMD[406] at PA 0x3C7000 + 406*8 → PTE base (e.g., PA 0x4D9000)
  5. Read PTE[193] at PA 0x4D9000 + 193*8 → PFN (e.g., PFN = 0x5EB)
  6. PA = (0x5EB << 12) | 0xD0E = 0x5EBD0E
```

### 5-Level Page Tables (LA57)

x86-64 with **5-level paging** (`CONFIG_X86_5LEVEL`) extends virtual addresses from 48 to 57 bits, adding a **PML5** level above PGD. This supports up to 128 PB of virtual address space. Used by large-memory servers. Most desktop systems don't enable it.

### Inner Page Tables on Demand

A crucial property: **inner-level page table pages are allocated only when needed**.

```
  Process with just code (low addresses) and stack (high addresses):
  
  PGD (always exists, 4 KB):
  ┌───────┬──────────┐
  │   0   │ → PUD_a  │  ← lower half of address space
  │   1   │  NULL    │  
  │  ...  │  NULL    │  ← vast middle region: NO page tables allocated
  │  510  │  NULL    │
  │  511  │ → PUD_b  │  ← upper half (kernel space / stack)
  └───────┴──────────┘
  
  Only PUD_a and PUD_b (and their children) are allocated.
  Hundreds of PUD/PMD/PTE pages are NEVER created.
  This saves enormous amounts of memory.
```

---

## 6. Process Address Space, OS Address Space, and Their Mapping

### Process (User) Address Space

On x86-64 Linux with 48-bit addressing, the virtual address space is split:

```
  64-bit Virtual Address Space (48-bit, canonical form):
  
  0xFFFFFFFFFFFFFFFF ┌──────────────────────────┐
                     │                          │
                     │    Kernel Space           │  128 TB
                     │    (same mapping in       │  (0xFFFF800000000000
                     │     every process)        │   to 0xFFFFFFFFFFFFFFFF)
                     │                          │
  0xFFFF800000000000 ├──────────────────────────┤
                     │                          │
                     │  Non-canonical hole       │  Not usable
                     │  (addresses with mixed    │  (CPU raises #GP
                     │   sign extension)         │   on access)
                     │                          │
  0x00007FFFFFFFFFFF ├──────────────────────────┤
                     │                          │
                     │    User Space             │  128 TB
                     │    (per-process,          │  (0x0000000000000000
                     │     unique mappings)      │   to 0x00007FFFFFFFFFFF)
                     │                          │
  0x0000000000000000 └──────────────────────────┘
```

### Why the Kernel Is Mapped into Every Process

When a process makes a system call or an interrupt occurs, the CPU transitions to kernel mode. The kernel code needs to execute and access its data structures. If the kernel had a separate address space, every syscall would require changing page tables (CR3) twice — extremely expensive.

Instead, the kernel is mapped into the **upper half** of every process's virtual address space. The page table entries for the kernel region have the **U/S (User/Supervisor) bit = 0**, so user-mode code can't access them even though they're "visible" in the address space.

```
  Process A's Page Tables:          Process B's Page Tables:
  
  ┌────────────────────┐            ┌────────────────────┐
  │ Kernel region      │            │ Kernel region      │
  │ (PGD[256..511])    │ ═══════    │ (PGD[256..511])    │
  │ SAME physical      │   SAME     │ SAME physical      │
  │ mappings!          │ ENTRIES    │ mappings!          │
  ├────────────────────┤            ├────────────────────┤
  │ User region        │            │ User region        │
  │ (PGD[0..255])      │ DIFFERENT  │ (PGD[0..255])      │
  │ A's code, data,    │            │ B's code, data,    │
  │ heap, stack, etc.  │            │ heap, stack, etc.  │
  └────────────────────┘            └────────────────────┘
```

### KPTI (Kernel Page-Table Isolation) — Meltdown Mitigation

After the **Meltdown vulnerability** (2018), Linux implemented **KPTI**: user-mode page tables have the kernel mappings **removed** (except for a minimal trampoline needed for syscall/interrupt entry). On transition to kernel mode, the CPU switches to a second set of page tables that include the kernel. This adds overhead (CR3 swap on every syscall) but prevents Meltdown-style speculative reads of kernel memory.

```bash
# Check if KPTI is active:
$ dmesg | grep "page tables isolation"
$ cat /sys/devices/system/cpu/vulnerabilities/meltdown
```

---

## 7. Page-Level Isolation and Security

Page tables provide fine-grained, per-page protection. The PTE flags enforce:

### Protection Examples

| Scenario | PTE Flags | Effect |
|----------|-----------|--------|
| User code (.text) | P=1, W=0, U=1, NX=0 | User can read and execute, but not write |
| User data (.data, heap) | P=1, W=1, U=1, NX=1 | User can read and write, but not execute |
| User read-only data (.rodata) | P=1, W=0, U=1, NX=1 | User can read only |
| Kernel memory | P=1, W=1, U=0, NX=0 | Only kernel can access; user access → fault |
| Unmapped region | P=0 | Any access → page fault (SIGSEGV) |
| Guard page (stack overflow) | P=0 | Detects stack overflow |
| Copy-on-Write page | P=1, W=0, U=1 | Read works; write → fault → kernel copies page, sets W=1 |

### Security Mechanisms Built on Paging

1. **W^X (Write XOR Execute)**: A page is either writable or executable, never both. Prevents code injection attacks. Enforced via W and NX bits.

2. **ASLR (Address Space Layout Randomization)**: Stack, heap, mmap region, and shared library bases are randomized on each exec. Makes exploitation harder because addresses are unpredictable.

3. **SMEP (Supervisor Mode Execution Prevention)**: CPU feature (x86 `CR4.SMEP`). Prevents the kernel from executing code in user-space pages. Stops ret2usr attacks.

4. **SMAP (Supervisor Mode Access Prevention)**: CPU feature (x86 `CR4.SMAP`). Prevents the kernel from reading/writing user-space pages unless explicitly enabled (via `stac`/`clac`). Forces the kernel to use `copy_from_user()` / `copy_to_user()`.

5. **Guard pages**: Unmapped pages placed between the stack and heap (or between thread stacks) to detect overflows.

```
  Stack Guard Page Example:
  
  ┌──────────────┐
  │    Stack      │  (grows downward)
  │    ...        │
  │    ▼          │
  ├──────────────┤
  │  GUARD PAGE   │  ← P=0 (unmapped)
  │  (4 KB)       │     Any access → SIGSEGV
  ├──────────────┤     Catches stack overflow
  │              │     before it corrupts heap
  │    Heap       │
  └──────────────┘
```

### Linux Specifics

```bash
# View a process's memory mappings with permissions:
$ cat /proc/self/maps
# Output format: start-end perms offset dev inode pathname
# 55a3c4000000-55a3c4001000 r--p 00000000 08:01 ...  /usr/bin/cat
# 55a3c4001000-55a3c4005000 r-xp ...                  (executable)
# 55a3c4009000-55a3c400a000 rw-p ...                  (writable)
# 7ffc12300000-7ffc12321000 rw-p ... [stack]          (writable, not executable)

# Check SMEP/SMAP:
$ grep -E "smep|smap" /proc/cpuinfo

# View ASLR setting:
$ cat /proc/sys/kernel/randomize_va_space
# 0 = off, 1 = conservative (stack, mmap, VDSO), 2 = full (+ brk/heap)
```

---

## 8. Linux Paging Implementation Details

### Linux Page Table API

Linux abstracts the page table levels through a set of macros and functions in `arch/x86/include/asm/pgtable.h` and `include/linux/pgtable.h`:

| Function/Macro | Purpose |
|---------------|---------|
| `pgd_offset(mm, addr)` | Get PGD entry for a virtual address |
| `pud_offset(pgd, addr)` | Get PUD entry |
| `pmd_offset(pud, addr)` | Get PMD entry |
| `pte_offset_map(pmd, addr)` | Get PTE entry |
| `pte_present(pte)` | Check if page is present in memory |
| `pte_write(pte)` | Check if page is writable |
| `pte_dirty(pte)` | Check if page has been written to |
| `pte_young(pte)` | Check if page has been accessed recently |
| `set_pte(ptep, pte)` | Set a PTE |

### CR3 and Page Table Base

On x86-64, the `CR3` register holds the **physical address** of the current process's PGD. On context switch, the kernel loads the new process's PGD address into CR3:

```c
// In switch_mm():
load_cr3(next->pgd);   // Changes the entire virtual address space!
```

### Huge Pages in Linux

Linux supports huge pages to reduce TLB pressure:

| Page Size | Page Table Level | PMD/PUD PS bit | Covers |
|-----------|-----------------|----------------|--------|
| 4 KB | PTE (level 1) | N/A | Standard page |
| 2 MB | PMD (level 2) | PMD entry PS=1 | Skips PTE level |
| 1 GB | PUD (level 3) | PUD entry PS=1 | Skips PMD+PTE levels |

```
  4 KB page walk:  CR3 → PGD → PUD → PMD → PTE → Page (4 levels)
  2 MB huge page:  CR3 → PGD → PUD → PMD → Page       (3 levels, PMD.PS=1)
  1 GB huge page:  CR3 → PGD → PUD → Page              (2 levels, PUD.PS=1)
```

Two mechanisms:
- **HugeTLB pages**: Pre-allocated, reserved huge pages. Used via `mmap` with `MAP_HUGETLB` or `hugetlbfs`. Configure via `/proc/sys/vm/nr_hugepages`.
- **Transparent Huge Pages (THP)**: The kernel automatically promotes/demotes pages between 4 KB and 2 MB. No application changes needed. Controlled via `/sys/kernel/mm/transparent_hugepage/enabled` (values: `always`, `madvise`, `never`).

```bash
# Use huge pages in a program:
void *p = mmap(NULL, 2*1024*1024, PROT_READ|PROT_WRITE,
               MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0);

# Or advise the kernel:
madvise(addr, len, MADV_HUGEPAGE);  // encourage THP
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| Page table | Maps VPN → PFN, one per process | 4-level radix tree (PGD→PUD→PMD→PTE) |
| PTE | Contains PFN + flag bits (P, W, U, NX, A, D, etc.) | 8 bytes on x86-64, 40-bit PFN |
| Address translation | VPN indexes into page table levels; offset passed through | MMU hardware walks tables from CR3 |
| TLB | Hardware cache for VPN→PFN translations | L1 iTLB + dTLB, L2 unified; PCID avoids flush on switch |
| Multi-level tables | Only allocate inner tables for mapped regions | Saves enormous memory for sparse address spaces |
| Kernel mapping | Kernel mapped in upper half of every process | PGD[256..511] shared; KPTI separates user/kernel tables |
| Page-level protection | Per-page read/write/execute/user bits | W^X, SMEP, SMAP, guard pages, ASLR |
| Huge pages | 2 MB / 1 GB pages to reduce TLB misses | HugeTLB (static) and THP (transparent, automatic) |
