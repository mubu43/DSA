# Demand Paging

---

## 1. Demand Paging Overview

In a naive system, the OS would load the **entire** program into physical memory before it starts running. This is wasteful — a program may have code paths that are rarely (or never) executed, data structures allocated but not yet touched, and libraries loaded but unused.

**Demand paging** is the strategy where the OS loads pages into physical memory **only when they are actually accessed** (on demand). If a page hasn't been touched, it doesn't occupy physical RAM.

```
  Without demand paging:               With demand paging:
  
  Load ENTIRE program into RAM          Load only what's needed
  before execution begins.              at the moment of access.
  
  ┌─────────┐                           ┌─────────┐
  │ code    │ → RAM ✓                   │ code p0 │ → RAM ✓ (accessed)
  │ code    │ → RAM ✓                   │ code p1 │ → NOT in RAM (not yet needed)
  │ data    │ → RAM ✓                   │ data p0 │ → RAM ✓ (accessed)
  │ data    │ → RAM ✓                   │ data p1 │ → NOT in RAM (not yet needed)
  │ heap    │ → RAM ✓                   │ heap    │ → NOT in RAM
  │ stack   │ → RAM ✓                   │ stack   │ → RAM ✓ (accessed)
  └─────────┘                           └─────────┘
  
  Uses 6 frames                          Uses 3 frames!
```

This means physical memory serves as a **cache** for the full virtual address space, which may be backed by files on disk or swap space.

### Benefits

- **Faster process startup**: No need to load the entire executable. Just set up page tables and start running.
- **More processes can coexist**: Physical memory is only consumed by pages actually in use.
- **Virtual address space can be larger than physical memory**: 100 processes each with 2 GB virtual space can coexist on a machine with 16 GB RAM, because most pages are never resident simultaneously.

---

## 2. Swap Space

**Swap space** is a designated area on disk (a swap partition or swap file) that acts as an extension of physical memory. When RAM is full and more pages are needed, the OS can **evict** (swap out) infrequently used pages from RAM to swap space, freeing frames for other pages.

```
  ┌────────────────────────┐        ┌─────────────────────────┐
  │   Physical Memory (RAM) │        │     Swap Space (Disk)    │
  │                        │        │                         │
  │  ┌──────┐ ┌──────┐    │        │  ┌──────┐  ┌──────┐    │
  │  │Page A│ │Page B│    │  swap  │  │Page C│  │Page D│    │
  │  │      │ │      │    │  out   │  │(was in│  │(was in│   │
  │  └──────┘ └──────┘    │ ────►  │  │ RAM)  │  │ RAM)  │   │
  │  ┌──────┐ ┌──────┐    │        │  └──────┘  └──────┘    │
  │  │Page E│ │(free)│    │  swap  │                         │
  │  │      │ │      │    │ ◄────  │  When Page C needed     │
  │  └──────┘ └──────┘    │  in    │  again, swap it back in │
  └────────────────────────┘        └─────────────────────────┘
```

### Tracking Pages in Swap Space

When a page is swapped out, its PTE is updated:
- **Present bit (P) = 0**: The page is not in RAM.
- The remaining PTE bits store a **swap entry** — an identifier that tells the OS where on the swap device the page's data resides.

```
  PTE when page is in memory:
  ┌─────────────────────────────────────────┐
  │ P=1 │ W │ U │ ... │    PFN (frame #)   │
  └─────────────────────────────────────────┘
  
  PTE when page is swapped out:
  ┌─────────────────────────────────────────┐
  │ P=0 │ swap_type │   swap_offset         │
  └─────────────────────────────────────────┘
  
  The OS encodes the swap device ID and the offset
  within that device into the PTE bits when P=0.
```

### Linux Swap Details

```bash
# View swap usage:
$ swapon --show
$ free -h
$ cat /proc/swaps

# Create and enable a swap file:
$ sudo fallocate -l 4G /swapfile
$ sudo chmod 600 /swapfile
$ sudo mkswap /swapfile
$ sudo swapon /swapfile

# Swappiness: how aggressively the kernel swaps (0-100)
$ cat /proc/sys/vm/swappiness     # default: 60
$ sudo sysctl vm.swappiness=10    # less aggressive swapping
```

The Linux kernel tracks swap entries using `swp_entry_t` and manages swap space via a bitmap in `mm/swap_state.c` and `mm/swapfile.c`.

---

## 3. Valid and Present Bits in Page Table Entry

A PTE can represent several states:

```
  ┌──────────────────────────────────────────────────────────┐
  │  PTE State                    │ Present │ Meaning         │
  ├──────────────────────────────┼─────────┼─────────────────┤
  │ Page in RAM, accessible       │   P=1   │ Normal. MMU     │
  │                               │         │ translates to   │
  │                               │         │ physical frame. │
  ├──────────────────────────────┼─────────┼─────────────────┤
  │ Page swapped out to disk      │   P=0   │ PTE contains    │
  │                               │ (valid) │ swap entry.     │
  │                               │         │ PAGE FAULT →    │
  │                               │         │ OS reads from   │
  │                               │         │ swap.           │
  ├──────────────────────────────┼─────────┼─────────────────┤
  │ Page allocated but never      │   P=0   │ PTE is zero     │
  │ accessed (demand-zero)        │ (valid) │ or marked.      │
  │                               │         │ PAGE FAULT →    │
  │                               │         │ OS allocates    │
  │                               │         │ zeroed frame.   │
  ├──────────────────────────────┼─────────┼─────────────────┤
  │ Page file-backed but not      │   P=0   │ PTE marked.     │
  │ loaded (demand-load)          │ (valid) │ PAGE FAULT →    │
  │                               │         │ OS reads from   │
  │                               │         │ file on disk.   │
  ├──────────────────────────────┼─────────┼─────────────────┤
  │ Address not part of process   │   P=0   │ PTE not present │
  │ address space (invalid)       │(invalid)│ PAGE FAULT →    │
  │                               │         │ OS sends        │
  │                               │         │ SIGSEGV.        │
  └──────────────────────────────┴─────────┴─────────────────┘
```

The key insight: **P=0 does not always mean the page is invalid**. The OS must distinguish between "this is a valid page that just isn't in RAM right now" vs. "this address is truly unmapped." The OS checks the process's **VMA (Virtual Memory Area)** list to make this determination.

---

## 4. Page Fault

A **page fault** occurs when the MMU encounters a PTE with the Present bit = 0 (or a permission violation). The CPU raises an exception (interrupt vector 14 on x86), trapping into the kernel's page fault handler.

### Page Fault Handling Flow

```
  ┌───────────────────────────────────────────────────────────────────┐
  │                     PAGE FAULT HANDLER                             │
  │              (do_page_fault → handle_mm_fault on Linux)            │
  │                                                                   │
  │  1. Read faulting address from CR2 register                        │
  │  2. Read error code (pushed by CPU):                               │
  │     - bit 0: 0=non-present, 1=protection violation                │
  │     - bit 1: 0=read, 1=write                                      │
  │     - bit 2: 0=kernel mode, 1=user mode                           │
  │                                                                   │
  │  3. Check: Is this address in a valid VMA?                         │
  │     (Search the process's VMA tree: mm->mmap / mm->mm_rb)          │
  │                                                                   │
  │     ┌─────────────────────────┐                                   │
  │     │ No valid VMA found      │                                   │
  │     │ → Address is INVALID    │──► Send SIGSEGV to process         │
  │     │   (segmentation fault)  │    (segfault / crash)              │
  │     └─────────────────────────┘                                   │
  │                                                                   │
  │     ┌─────────────────────────┐                                   │
  │     │ Valid VMA found, but    │                                   │
  │     │ permission violation    │                                   │
  │     │ (e.g., write to R/O)   │                                   │
  │     └───────────┬─────────────┘                                   │
  │                 │                                                  │
  │         ┌───────┴──────────────────┐                              │
  │         │ Is this COW?             │                              │
  │         │ (read-only page after    │                              │
  │         │  fork, but VMA is R/W)   │                              │
  │         │ Yes → Copy page, make    │                              │
  │         │       writable.          │                              │
  │         │ No  → SIGSEGV            │                              │
  │         └──────────────────────────┘                              │
  │                                                                   │
  │     ┌─────────────────────────┐                                   │
  │     │ Valid VMA, page not     │                                   │
  │     │ present (P=0)           │                                   │
  │     └───────────┬─────────────┘                                   │
  │                 │                                                  │
  │     ┌───────────┴────────────────────────────┐                    │
  │     │                                        │                    │
  │     ▼                                        ▼                    │
  │  ┌───────────────────┐         ┌─────────────────────────┐       │
  │  │ Anonymous page     │         │ File-backed page         │       │
  │  │ (heap, stack, etc.)│         │ (.text, mmap'd file)     │       │
  │  │                   │         │                         │       │
  │  │ Is it in swap?    │         │ Read page from file      │       │
  │  │ Yes → Read from   │         │ on disk into a free      │       │
  │  │       swap space  │         │ frame.                   │       │
  │  │ No  → Allocate    │         │                         │       │
  │  │       zeroed frame│         │ (page cache may         │       │
  │  │       (demand-zero)│         │  already have it)       │       │
  │  └───────────────────┘         └─────────────────────────┘       │
  │                                                                   │
  │  4. Update PTE: set PFN, set P=1, set appropriate flags           │
  │  5. Add translation to TLB                                        │
  │  6. Return from fault — CPU retries the faulting instruction       │
  └───────────────────────────────────────────────────────────────────┘
```

### Linux Page Fault Handler

The page fault handler on x86-64 is:
- Entry: `exc_page_fault()` in `arch/x86/mm/fault.c`
- Core logic: `handle_mm_fault()` → `__handle_mm_fault()` in `mm/memory.c`
- This function walks the page table levels, allocating missing intermediate tables, and calls `handle_pte_fault()` for the final PTE.

---

## 5. Reclaiming Memory

When physical memory is running low, the OS must free up frames. This is **page reclamation** (or **page eviction**).

### What Can Be Evicted?

Not all pages can be evicted:
- **Kernel pages**: Generally not swappable (wired/pinned).
- **Pages locked with `mlock()`**: User has requested they stay in RAM.
- **DMA buffer pages**: Hardware is reading/writing them.

Pages that CAN be evicted:

| Page Type | Eviction Strategy |
|-----------|-------------------|
| **Clean file-backed pages** | Simply discard — can be re-read from the file |
| **Dirty file-backed pages** | Write back to the file, then discard |
| **Clean anonymous pages** | Already in swap (from previous swap-out) — just discard |
| **Dirty anonymous pages** | Write to swap space, then discard |

### Linux Page Reclamation

Linux uses a modified **LRU (Least Recently Used)** approach with two lists per memory zone:

```
  Active List                    Inactive List
  ┌──────────────────┐          ┌──────────────────┐
  │ Recently accessed │          │ Not recently      │
  │ pages             │ ──age──► │ accessed pages    │ ──evict──►  Free
  │                  │          │                  │
  │ Hot pages stay   │          │ Cold pages get   │
  │ here             │          │ reclaimed        │
  └──────────────────┘          └──────────────────┘
```

- Pages start on the **inactive list** when first loaded.
- If accessed again (Accessed bit set), they're promoted to the **active list**.
- When memory pressure occurs, pages at the tail of the active list are demoted to the inactive list.
- Pages at the tail of the inactive list are evicted.

The kernel daemon **`kswapd`** runs in the background, proactively reclaiming pages when free memory drops below a low watermark, keeping a pool of free frames ready.

```bash
# View memory zones and watermarks:
$ cat /proc/zoneinfo | grep -A 5 "Node 0, zone   Normal"

# View kswapd activity:
$ vmstat 1    # columns: si=swap-in, so=swap-out, free, buff, cache
```

---

## 6. File-Backed, Anonymous, and Dirty Pages

### File-Backed Pages

Pages whose content comes from a file on disk:
- Program code (`.text` segment) — backed by the executable file.
- Memory-mapped files (`mmap` of a file).
- Shared library code.

**Eviction**: If clean (unmodified), just discard — the file on disk is the canonical copy. If dirty (modified, e.g., `mmap` with `MAP_SHARED` + writes), write back to the file first.

### Anonymous Pages

Pages that have **no file backing** — they exist only in RAM (and potentially swap):
- Heap (allocated via `malloc` / `brk` / `mmap` with `MAP_ANONYMOUS`).
- Stack.
- Copy-on-Write pages after `fork()`.

**Eviction**: Must be written to **swap space** before the frame can be freed. If swap is disabled/full, anonymous pages cannot be evicted (OOM killer may be invoked).

### Dirty Pages

A page is **dirty** if it has been written to since it was loaded from disk (or since it was last written back). The hardware sets the **D (Dirty) bit** in the PTE when a write occurs.

```
  Page States:
  
  File-backed, clean:  In RAM, matches file on disk.  → Discard freely.
  File-backed, dirty:  In RAM, modified.               → Write to file, then discard.
  Anonymous, clean:    In RAM, has a copy in swap.      → Discard (swap copy is valid).
  Anonymous, dirty:    In RAM, no valid swap copy.      → Write to swap, then discard.
```

### Linux Page Cache

Linux caches file-backed pages in the **page cache** (RAM). When a file is read, the data is stored in the page cache. Subsequent reads of the same file come from RAM (fast) instead of disk (slow).

```bash
# View page cache usage:
$ free -h     # "buff/cache" column
$ cat /proc/meminfo | grep -E "Cached|Buffers|Active|Inactive"

# Drop caches (for testing, don't do in production):
$ echo 3 | sudo tee /proc/sys/vm/drop_caches
# 1 = page cache, 2 = dentries/inodes, 3 = both
```

---

## 7. Disk Access During Page Fault

A page fault that requires reading from disk is called a **major page fault**. A page fault that can be resolved without disk I/O (e.g., demand-zero, COW) is a **minor page fault**.

```
  Major Page Fault (slow — involves disk I/O):
  
  Process               Kernel                    Disk
  ───────               ──────                    ────
  Access VA ──►  Page fault!
                 PTE: P=0
                 Page is in swap or file
                 ─────────────────────────► Read page from disk
                 
                 Process moves to
                 TASK_UNINTERRUPTIBLE
                 (blocked on I/O)
                 
                 schedule() → run
                 another process
                                            ◄─── DMA complete
                                                  Interrupt
                 
                 Interrupt handler:
                 Page data now in RAM
                 Update PTE: P=1, PFN=new
                 Wake up process
                 
  Retry the    ◄── Process resumes
  faulting
  instruction
  (succeeds)
  
  
  Minor Page Fault (fast — no disk I/O):
  
  Process               Kernel
  ───────               ──────
  Access VA ──►  Page fault!
                 PTE: P=0
                 But it's a demand-zero page
                 (or COW, or page already in
                  page cache)
                 
                 Allocate free frame
                 Zero it out (or copy COW page)
                 Update PTE: P=1, PFN=new
                 Return immediately
                 
  Retry instr ◄── (succeeds)
  (~microseconds, no disk wait)
```

```bash
# Count major/minor page faults for a command:
$ /usr/bin/time -v ls 2>&1 | grep -i fault
# Major (requiring I/O) page faults: 0
# Minor (reclaiming a frame) page faults: 123

# View per-process fault counts:
$ cat /proc/<pid>/stat    # fields 10 (minflt) and 12 (majflt)

# System-wide:
$ sar -B 1    # pgfault/s and pgmajfault/s columns
```

---

## 8. What Happens on Memory Access — Detailed Summary

This is the complete decision tree for a single memory access instruction (e.g., `mov rax, [addr]`):

### Case 1: TLB Hit, Page in Memory (fastest — common case)

```
  CPU generates VA
       │
       ▼
  TLB lookup ──► HIT! VPN→PFN found in TLB
       │
       ▼
  Check permissions (from TLB entry)
       │         │
       OK       FAIL → Exception (protection fault)
       │
       ▼
  Form PA = (PFN << 12) | offset
       │
       ▼
  Send PA to cache hierarchy
       │
       ▼
  L1 cache hit? → Return data (~1 ns)
  L1 miss, L2 hit? → Return data (~5 ns)
  L2 miss, L3 hit? → Return data (~20 ns)
  L3 miss → Read from DRAM (~100 ns)
  
  Total time: ~1-100 ns (no kernel involvement)
```

### Case 2: TLB Miss, Page in Memory

```
  CPU generates VA
       │
       ▼
  TLB lookup ──► MISS! VPN not in TLB
       │
       ▼
  MMU performs PAGE TABLE WALK
  (hardware on x86-64, software on some other architectures):
       │
       ▼
  Read PGD[...] → Read PUD[...] → Read PMD[...] → Read PTE[...]
  (4 memory accesses, but page table pages may be cached in L1/L2)
       │
       ▼
  PTE found with P=1 (page is in memory)
       │
       ▼
  Add VPN→PFN to TLB (evict old TLB entry if needed)
       │
       ▼
  Form PA, access cache hierarchy (same as Case 1)
  
  Total time: ~10-100 ns extra for page table walk
  (Often much less because page table pages are cached)
```

### Case 3: TLB Miss, Page Fault (slowest)

```
  CPU generates VA
       │
       ▼
  TLB lookup ──► MISS!
       │
       ▼
  MMU page table walk
       │
       ▼
  PTE has P=0 (page not in memory)
       │
       ▼
  CPU raises PAGE FAULT exception (#PF, vector 14)
       │
       ▼
  ╔═══════════════════════════════════════════════════╗
  ║  TRAP INTO KERNEL (mode switch User → Kernel)     ║
  ║                                                   ║
  ║  Kernel page fault handler:                        ║
  ║  1. Read faulting address from CR2                 ║
  ║  2. Look up VMA for this address                   ║
  ║                                                   ║
  ║  ┌────────────────────────────────────────────┐   ║
  ║  │ Is address valid (in a VMA)?                │   ║
  ║  │ NO  → send SIGSEGV (segfault)              │   ║
  ║  │ YES → determine page type                  │   ║
  ║  └─────────────────┬──────────────────────────┘   ║
  ║                    │                              ║
  ║  ┌─────────────────┴──────────────────────┐       ║
  ║  │                                        │       ║
  ║  ▼                                        ▼       ║
  ║  Anonymous page:                File-backed page:  ║
  ║  - In swap? Read from swap      - In page cache?  ║
  ║    (major fault, ~ms)             If yes: minor    ║
  ║  - Demand-zero? Allocate          fault (~μs)      ║
  ║    zeroed frame (minor fault)   - Not cached:      ║
  ║  - COW? Copy page                read from disk    ║
  ║    (minor fault, ~μs)            (major fault,~ms) ║
  ║                                                   ║
  ║  3. Allocate a free physical frame                 ║
  ║     (may need to evict another page first)         ║
  ║  4. Fill frame with data (zero/copy/read from disk)║
  ║  5. Update PTE: P=1, PFN=new frame, set flags     ║
  ║  6. Flush stale TLB entry if needed                ║
  ║  7. Return from fault                              ║
  ╚═══════════════════════════════════════════════════╝
       │
       ▼
  CPU retries the faulting instruction (succeeds)
  
  Minor fault total: ~1-10 μs
  Major fault total: ~1-10 ms (dominated by disk I/O)
                     SSD: ~50-200 μs
                     HDD: ~3-10 ms
```

---

## 9. Virtual Memory and Caches

The CPU cache hierarchy can be addressed using either virtual or physical addresses:

### VIPT (Virtually Indexed, Physically Tagged) — Most Common

```
  CPU generates VA
       │
       ├──► Extract cache index from VA ──► Select cache set
       │                                        │
       ├──► TLB lookup (in parallel!) ──►  Get PA
       │                                        │
       │                                        ▼
       │                                   Compare PA tag
       │                                   with tags in set
       │                                        │
       │                                   Hit or Miss?
       ▼
  VIPT allows cache lookup and TLB lookup to happen IN PARALLEL
  because the cache index comes from the VA (offset bits, which
  are the same in VA and PA for small caches).
```

| Type | Index | Tag | Tradeoff |
|------|-------|-----|----------|
| PIPT (Physically Indexed, Physically Tagged) | PA | PA | Simple, no aliasing. But slow — must wait for TLB. |
| VIPT (Virtually Indexed, Physically Tagged) | VA | PA | Fast (parallel TLB+cache). Most L1 caches use this. |
| VIVT (Virtually Indexed, Virtually Tagged) | VA | VA | Fastest but aliasing problems. Rare in modern CPUs. |

Modern x86-64:
- L1 caches: **VIPT** (index bits come from the page offset, which is identical in VA and PA for 4 KB pages with 64-byte cache lines and 8-way associativity).
- L2, L3 caches: **PIPT** (physically addressed).

---

## 10. Thrashing

**Thrashing** occurs when the system spends more time swapping pages in and out of memory than doing useful work. This happens when the **working set** of active processes exceeds available physical memory.

```
  Normal operation:               Thrashing:
  
  CPU busy with                   CPU mostly idle, waiting
  useful computation              for disk I/O (page faults)
  
  ┌──────────────────┐            ┌──────────────────┐
  │ CPU: 90% busy    │            │ CPU: 10% busy    │
  │ Disk: 10% busy   │            │ Disk: 100% busy  │
  │ Throughput: HIGH  │            │ Throughput: NEAR  │
  │                  │            │              ZERO │
  └──────────────────┘            └──────────────────┘
  
  Adding more processes            
  ┌────────────────────────────────────────┐
  │                                        │
  │  Throughput                             │
  │  ▲                                     │
  │  │         ┌─────┐                     │
  │  │        /│     │\                    │
  │  │       / │     │ \                   │
  │  │      /  │     │  \                  │
  │  │     /   │     │   \                 │
  │  │    /    │     │    \───────          │
  │  │   /     │     │    thrashing         │
  │  │  /      │     │    begins           │
  │  │ /       │     │                     │
  │  └────────────────────► # of processes  │
  │            optimal                      │
  │            point                        │
  └────────────────────────────────────────┘
```

### Causes

- Too many processes competing for limited RAM.
- A single process with a working set larger than available memory.
- Poor page replacement policy.

### Detection and Mitigation in Linux

```bash
# Signs of thrashing:
$ vmstat 1
# Look for: high si/so (swap in/out), high bi/bo (block I/O),
# low free memory, high wa (I/O wait %)

$ sar -W 1     # swap activity
$ sar -B 1     # paging activity (pgmajfault/s)
```

**Linux OOM Killer**: When the system is critically low on memory and thrashing, the Out-Of-Memory killer (`mm/oom_kill.c`) selects a process to kill based on a heuristic score (memory usage, niceness, etc.). Each process has an OOM score visible at `/proc/<pid>/oom_score`. You can adjust it via `/proc/<pid>/oom_score_adj` (-1000 to +1000).

```bash
# View OOM scores:
$ cat /proc/<pid>/oom_score
$ cat /proc/<pid>/oom_score_adj

# Protect a critical process from OOM killer:
$ echo -1000 > /proc/<pid>/oom_score_adj
```

---

## 11. Page Replacement Policies

When a page fault occurs and there are no free frames, the OS must choose a **victim page** to evict. The choice of which page to evict is the **page replacement policy**.

### Setup for Examples

Consider 3 physical frames and the following page reference string:

```
  Reference string: 7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1
```

### 11.1 Optimal (OPT / Bélády's Algorithm)

**Rule**: Evict the page that will **not be used for the longest time in the future**.

This is provably optimal — it produces the fewest page faults. But it requires **future knowledge**, so it's unrealizable in practice. Used as a benchmark to evaluate other policies.

```
  Ref:   7  0  1  2  0  3  0  4  2  3  0  3  2  1  2  0  1  7  0  1
  
  Frame0: 7  7  7  2  2  2  2  2  2  2  2  2  2  2  2  2  2  7  7  7
  Frame1: -  0  0  0  0  0  0  4  4  4  0  0  0  1  1  1  1  1  1  1
  Frame2: -  -  1  1  1  3  3  3  3  3  3  3  3  3  3  0  0  0  0  0
  Fault?  F  F  F  F     F     F        F        F     F     F
  
  Total faults: 9
```

### 11.2 FIFO (First In, First Out)

**Rule**: Evict the page that has been in memory the **longest** (oldest arrival).

Simple to implement — just a queue.

```
  Ref:   7  0  1  2  0  3  0  4  2  3  0  3  2  1  2  0  1  7  0  1
  
  Frame0: 7  7  7  2  2  2  2  4  4  4  0  0  0  1  1  1  1  7  7  7
  Frame1: -  0  0  0  0  3  3  3  2  2  2  2  2  2  2  0  0  0  0  0
  Frame2: -  -  1  1  1  1  0  0  0  3  3  3  3  3  3  3  1  1  1  1
  Fault?  F  F  F  F     F  F  F  F  F  F        F        F  F
  
  Total faults: 15
```

**Pros**: Simple, O(1) per decision.

**Cons**: Doesn't consider how recently or frequently a page was used. Suffers from **Bélády's anomaly** — increasing the number of frames can *increase* the number of faults with FIFO (counterintuitive!).

### 11.3 LRU (Least Recently Used)

**Rule**: Evict the page that was accessed **least recently** — the one whose last access is farthest in the past.

LRU approximates OPT by using past behavior as a predictor of future behavior: a page not accessed for a long time is unlikely to be accessed soon.

```
  Ref:   7  0  1  2  0  3  0  4  2  3  0  3  2  1  2  0  1  7  0  1
  
  Frame0: 7  7  7  2  2  2  2  4  4  4  0  0  0  1  1  1  1  1  1  1
  Frame1: -  0  0  0  0  0  0  0  0  3  3  3  3  3  3  0  0  0  0  0
  Frame2: -  -  1  1  1  3  3  3  2  2  2  2  2  2  2  2  2  7  7  7
  Fault?  F  F  F  F     F     F  F  F  F        F     F     F
  
  Total faults: 12
```

**Pros**: Good approximation of OPT. No Bélády's anomaly.

**Cons**: Expensive to implement exactly — need to track the access time of every page on every access.

### LRU Implementation Approaches

**Exact LRU** (impractical for OS):
1. **Timestamp**: Record a counter/timestamp on every page access. Evict the page with the smallest timestamp. Requires updating a counter on *every* memory access — far too expensive.
2. **Doubly-linked list (stack)**: Move the accessed page to the head of a linked list on every access. The tail is the LRU page. Again, updating the list on every access is too expensive for hardware to do.

**Approximate LRU** (what real systems use):

**Clock Algorithm (Second-Chance)**:

The clock algorithm uses the hardware **Accessed (A) bit** in the PTE:

```
  Physical frames arranged in a circular buffer with a "clock hand":
  
          ┌───────────┐
       ┌──┤ Frame 0   │
       │  │ A=1       │
       │  ├───────────┤
       │  │ Frame 1   │
       │  │ A=0       │◄── Clock hand
       │  ├───────────┤
       │  │ Frame 2   │
       │  │ A=1       │
       │  ├───────────┤
       │  │ Frame 3   │
       │  │ A=0       │
       │  └───────────┘
       └───────────────┘
  
  When evicting:
  1. Look at page under the clock hand.
  2. If A=1: Give it a "second chance" — clear A to 0, advance hand.
  3. If A=0: Evict this page. Done.
  4. Advance hand and repeat.
  
  Effect: Pages that have been accessed recently (A=1) survive.
          Pages not accessed since the last sweep (A=0) get evicted.
          Approximates LRU — recently accessed pages tend to have A=1.
```

### Linux's Approach: Active/Inactive LRU Lists

As described in Section 5 (Reclaiming Memory), Linux maintains **active** and **inactive** lists. The Accessed bit is checked periodically:

- Pages with A=1 get promoted from inactive → active (or stay in active).
- Pages with A=0 in active get demoted to inactive.
- Pages at the tail of inactive get evicted.

This is a **multi-generational LRU**. Linux kernel 6.1+ introduced **MGLRU (Multi-Generational LRU)** as an optional improvement, using multiple generations to better classify page hotness.

```bash
# Check MGLRU status:
$ cat /sys/kernel/mm/lru_gen/enabled

# Enable MGLRU:
$ echo 1 > /sys/kernel/mm/lru_gen/enabled
```

### Policy Comparison

| Policy | Faults (example) | Optimal? | Implementable? | Complexity |
|--------|------------------|----------|----------------|------------|
| **OPT** | 9 | Yes | **No** (needs future knowledge) | — |
| **FIFO** | 15 | No | Yes | O(1) |
| **LRU** | 12 | No | Exact: too expensive. Approx: Yes. | Approx: O(1) |
| **Clock** | ~LRU | No | Yes (uses A bit) | O(1) amortized |

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| Demand paging | Load pages into RAM only when accessed | Default behavior; lazy allocation |
| Swap space | Disk area for evicted anonymous pages | `swapon`/`swapoff`, `/proc/swaps`, `vm.swappiness` |
| PTE states | P=1 (in RAM), P=0 (swapped, demand-zero, or invalid) | VMA lookup distinguishes valid vs invalid |
| Page fault | Exception when accessing non-present page | `do_page_fault()` → `handle_mm_fault()` in `mm/memory.c` |
| Major/minor fault | Major = disk I/O needed; Minor = no disk | `/usr/bin/time -v`, `/proc/<pid>/stat` fields 10,12 |
| Page types | File-backed (from file), anonymous (heap/stack), dirty (modified) | Page cache for file-backed; swap for anonymous |
| Thrashing | System spends more time paging than computing | OOM killer (`mm/oom_kill.c`), `oom_score_adj` |
| Page replacement | OPT (theoretical), FIFO (simple), LRU (good, approximate) | Active/inactive LRU lists, MGLRU (kernel 6.1+), clock algorithm |
| TLB interaction | TLB hit = fast, TLB miss = page table walk, page fault = kernel trap | PCID avoids full TLB flush on context switch |
