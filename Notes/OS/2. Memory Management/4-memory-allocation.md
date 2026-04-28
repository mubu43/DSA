# Memory Allocation and Free Space Management

---

## 1. Overview: How Does a Process Get More Memory?

When a user program calls `malloc(1024)`, a lot happens behind the scenes. Let's trace the full path from user space to the kernel and back.

```
  User Program
  ─────────────
  ptr = malloc(1024);
       │
       ▼
  C Library (glibc)
  ──────────────────
  malloc() checks its internal free list (user-space allocator).
  If it has a free chunk of 1024+ bytes → return it (no syscall needed).
  If not → request more memory from the kernel:
       │
       ├──► brk() / sbrk()        ← for small allocations (< 128 KB default)
       │    Moves the "program break" (end of heap) upward.
       │
       └──► mmap(MAP_ANONYMOUS)    ← for large allocations (≥ 128 KB default)
            Maps a new anonymous region into the address space.
       │
       ▼
  Kernel
  ──────
  Updates the process's VMA (Virtual Memory Area) list.
  Does NOT allocate physical pages yet! (Demand paging)
  Just creates a VMA entry saying "this range is valid."
       │
       ▼
  First Access (later)
  ────────────────────
  When the program actually reads/writes the allocated memory:
  → Page fault (P=0 in PTE, but valid VMA exists)
  → Kernel allocates a physical frame
  → Zeroes it out (security: can't leak previous contents)
  → Maps it in the page table (PTE: P=1, PFN=frame)
  → Returns to user mode
```

### Key Insight: Two Levels of Allocation

1. **Kernel allocator**: Manages physical frames and virtual address ranges. Syscalls `brk()` and `mmap()` request virtual address space from the kernel.
2. **User-space allocator (libc)**: Manages chunks within the virtual address space already obtained from the kernel. `malloc()`/`free()` operate at this level, subdividing large regions into smaller allocations.

This two-level design avoids the overhead of a syscall for every small allocation.

---

## 2. `mmap` and `munmap`

### `mmap` — Map Memory

`mmap` is the primary syscall for requesting virtual address space. It's more versatile than `brk` and is used for both file-backed and anonymous mappings.

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```

| Parameter | Description |
|-----------|-------------|
| `addr` | Hint for where to map (usually NULL — let kernel choose) |
| `length` | Size of mapping in bytes (rounded up to page boundary) |
| `prot` | Protection: `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`, `PROT_NONE` |
| `flags` | Behavior flags (see below) |
| `fd` | File descriptor (for file-backed mapping) or -1 (for anonymous) |
| `offset` | Offset into the file (for file-backed mapping) |

### Key Flags

| Flag | Meaning |
|------|---------|
| `MAP_ANONYMOUS` | No file backing — memory is zeroed (for heap, temp data) |
| `MAP_PRIVATE` | Copy-on-Write — changes are private to this process |
| `MAP_SHARED` | Changes are visible to other processes mapping the same file/region |
| `MAP_FIXED` | Map at exactly the specified address (dangerous if overlaps) |
| `MAP_HUGETLB` | Use huge pages (2 MB or 1 GB) |
| `MAP_POPULATE` | Pre-fault pages (don't wait for demand paging) |

### Common `mmap` Use Cases

```c
// 1. Anonymous private mapping (heap-like)
void *p = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
// Returns pointer to a new zeroed 4 KB page.
// Used by malloc() for large allocations.

// 2. File-backed private mapping (loading code/data)
int fd = open("data.bin", O_RDONLY);
void *p = mmap(NULL, file_size, PROT_READ,
               MAP_PRIVATE, fd, 0);
// Maps file into memory. Reads come from file via page cache.
// Writes (if allowed) are COW — don't modify the file.
// This is how the ELF loader maps .text and .data segments.

// 3. File-backed shared mapping (IPC / persistent data)
int fd = open("shared_data.bin", O_RDWR);
void *p = mmap(NULL, file_size, PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);
// Changes are written back to the file.
// Multiple processes can map the same file for IPC.

// 4. Unmap when done
munmap(p, 4096);
```

### What Happens Inside the Kernel

When `mmap()` is called:

1. Kernel allocates a **VMA (Virtual Memory Area)** struct (`vm_area_struct`) describing the mapping:
   - Start and end virtual addresses
   - Protection flags
   - Backing (file or anonymous)
   - Pointer to `vm_operations_struct` (fault handler, etc.)
2. The VMA is inserted into the process's `mm_struct` (red-black tree + linked list of VMAs).
3. **No physical memory is allocated yet** — this is just bookkeeping.
4. On first access → page fault → kernel checks VMA → allocates frame → updates PTE.

```
  Process mm_struct:
  ┌──────────────────────────────────────────────────────┐
  │  VMA: 0x400000-0x401000  r-xp  (code, file-backed)   │
  │  VMA: 0x601000-0x602000  rw-p  (data, file-backed)   │
  │  VMA: 0x700000-0x800000  rw-p  (heap, anonymous)     │
  │  VMA: 0x7f0000-0x7f1000  rw-p  (mmap, anonymous)     │  ← new mmap
  │  VMA: 0x7fffe000-0x80000000 rw-p (stack, anonymous)  │
  └──────────────────────────────────────────────────────┘
```

```bash
# View VMAs:
$ cat /proc/self/maps

# View detailed VMA info:
$ cat /proc/<pid>/smaps    # includes RSS, PSS, shared/private, etc.
```

### `brk` / `sbrk` — Adjust the Program Break

The older, simpler mechanism for expanding the heap:

```c
#include <unistd.h>
int brk(void *addr);          // Set the program break to 'addr'
void *sbrk(intptr_t increment); // Move the break by 'increment' bytes
```

```
  Before brk/sbrk:                After sbrk(4096):
  
  ┌──────────────┐                ┌──────────────┐
  │    Stack      │                │    Stack      │
  │               │                │               │
  ├──────────────┤                ├──────────────┤
  │              │                │              │
  │              │                │              │
  │              │                │  ┌──────────┐ │
  │              │                │  │ new 4KB  │ │ ← newly usable
  ├──────────────┤ ← program     │  └──────────┘ │
  │    Heap      │    break      ├──────────────┤ ← new program break
  │              │                │    Heap      │
  └──────────────┘                └──────────────┘
```

`brk`/`sbrk` only grows/shrinks the heap contiguously. `mmap` can place mappings anywhere. glibc's `malloc` uses `brk` for small allocations (growing the heap) and `mmap` for large ones. The threshold is `M_MMAP_THRESHOLD` (default: 128 KB), tunable via `mallopt()`.

---

## 3. Memory Mapping Lifecycle

### Anonymous Page Lifecycle

```
  1. malloc(size) called (large allocation)
     └──► mmap(MAP_ANONYMOUS | MAP_PRIVATE)
          └──► Kernel creates VMA. No physical memory yet.
  
  2. First write to the page
     └──► Page fault (P=0, valid VMA)
          └──► Kernel allocates a zeroed physical frame
          └──► Updates PTE: P=1, W=1, PFN=frame
          └──► Page is now "anonymous, dirty"
  
  3. Memory pressure — page evicted
     └──► Kernel writes page to swap space
     └──► Updates PTE: P=0, swap_entry
     └──► Frame freed
  
  4. Process accesses page again
     └──► Page fault (P=0, swap entry in PTE)
     └──► Kernel reads from swap → allocates frame → fills it
     └──► Updates PTE: P=1, PFN=new_frame
  
  5. free(ptr) called
     └──► munmap()
          └──► Kernel removes VMA, frees frame (if resident),
               removes swap entry (if in swap)
```

### File-Backed Page Lifecycle

```
  1. mmap(fd, MAP_PRIVATE) called
     └──► Kernel creates VMA linked to the file's inode.
  
  2. First read from a page
     └──► Page fault
     └──► Kernel checks page cache: is this file page already cached?
          ├──► YES (page cache hit): Map existing page. Minor fault.
          └──► NO (page cache miss): Read from disk into page cache.
               Major fault. Map the page cache page into process.
  
  3. Process reads the page
     └──► TLB hit (fast). Data served from page cache in RAM.
  
  4. Memory pressure — page evicted from process
     └──► If MAP_PRIVATE and clean: just unmap (file is the backing).
     └──► If MAP_PRIVATE and dirty (COW copy): write to swap.
     └──► If MAP_SHARED and dirty: write back to file.
```

### `malloc` and Standard Library Allocation

`malloc` is NOT a syscall — it's a user-space library function. glibc's `malloc` implementation (ptmalloc2) manages chunks of memory:

```
  malloc(32):
    1. Check thread-local free list (arena) for a 32-byte chunk.
    2. If found → return it. No syscall. Very fast (~20-50 ns).
    3. If not → check larger free chunks, split one.
    4. If all free lists empty → call sbrk() or mmap() to get more
       memory from the kernel. Then carve out a 32-byte chunk.
  
  free(ptr):
    1. Mark the chunk as free in the allocator's data structures.
    2. Potentially coalesce with adjacent free chunks.
    3. If a large region is now free and was mmap'd → munmap() it
       (returns memory to the kernel).
    4. If the top of the heap is free → may call brk() to shrink.
```

---

## 4. Types of Memory Allocators

### General-Purpose (Variable-Size) Allocators

Handle allocation requests of **arbitrary sizes**. The classic challenge: given a large block of memory, satisfy a stream of `malloc(n)` and `free(ptr)` calls of varying sizes.

**Examples**: glibc `malloc` (ptmalloc2), jemalloc, tcmalloc, dlmalloc.

**Challenges**:
- **External fragmentation**: Free memory exists but is split into small, non-contiguous chunks that can't satisfy a large request.
- **Internal fragmentation**: Allocated chunks are slightly larger than requested (due to alignment, headers).
- **Performance**: Must be fast — `malloc`/`free` are among the most frequently called functions.
- **Thread safety**: Multiple threads allocating concurrently.

### Fixed-Size (Pool) Allocators

Pre-allocate a pool of **identically-sized** objects. Very fast because there's no fragmentation concern — every slot is the same size.

**Examples**: Linux kernel's slab/slub allocator, custom object pools in game engines, network buffer pools.

```
  Fixed-size pool for 64-byte objects:
  
  ┌────────┬────────┬────────┬────────┬────────┬────────┐
  │ Obj 0  │ Obj 1  │ Obj 2  │ Obj 3  │ Obj 4  │ Obj 5  │
  │ (used) │ (free) │ (used) │ (free) │ (free) │ (used) │
  └────────┴────────┴────────┴────────┴────────┴────────┘
  
  Free list: Obj1 → Obj3 → Obj4 → NULL
  
  Allocate: Pop from free list → O(1)
  Free: Push onto free list → O(1)
  No fragmentation! Every slot is 64 bytes.
```

**Use cases**:
- Kernel: `task_struct` objects, `inode` objects, `sk_buff` (socket buffers) — all fixed size. Linux slab allocator pre-allocates caches of these.
- Networking: Packet buffer pools — all buffers are the same size (e.g., MTU-sized).
- Real-time systems: Deterministic O(1) allocation with no fragmentation risk.

### When to Use Which

| Aspect | General-Purpose | Fixed-Size Pool |
|--------|----------------|-----------------|
| Allocation sizes | Variable | Uniform |
| Fragmentation | Yes (external + internal) | None (all slots equal) |
| Speed | Good (but varies) | Excellent (O(1) always) |
| Determinism | Non-deterministic (may need to search) | Deterministic |
| Memory overhead | Headers per chunk, alignment padding | Minimal |
| Flexibility | Handle any size | Only one size |

---

## 5. General-Purpose Variable-Size Allocation — In Depth

This is the classic memory allocation problem: given a large contiguous block of memory, manage variable-sized allocations and frees. This is what `malloc`/`free` do internally.

### The Core Problem

```
  Initial state: one large free block
  ┌────────────────────────────────────────────────────────────┐
  │                    FREE (1 MB)                              │
  └────────────────────────────────────────────────────────────┘
  
  After several malloc() and free() calls:
  ┌───────┬──────────┬────┬─────────────┬──────┬───────────────┐
  │ USED  │   FREE   │USED│    FREE     │ USED │    FREE       │
  │ 100B  │   200B   │50B │    500B     │ 80B  │    ~999070B   │
  └───────┴──────────┴────┴─────────────┴──────┴───────────────┘
  
  Challenge: manage this efficiently!
  - Track which regions are free vs. allocated
  - Handle fragmentation
  - Be fast
```

### Tracking Information About Chunks

The allocator needs metadata about each chunk: is it free or allocated? How big is it? The standard approach is to store a **header** immediately before each chunk:

```
  Memory layout with headers:
  
  ┌────────┬──────────────┬────────┬──────────────┬────────┬───────────┐
  │ Header │  User Data   │ Header │  User Data   │ Header │ User Data │
  │ (8-16B)│  (requested) │ (8-16B)│  (requested) │ (8-16B)│           │
  └────────┴──────────────┴────────┴──────────────┴────────┴───────────┘
  
  malloc(100) returns →  ┌──► pointer to User Data (after header)
                         │
  ┌───────────────┬──────┴────────────────┐
  │ Header        │ User Data (100 bytes) │
  │ size=100      │                       │
  │ allocated=1   │                       │
  └───────────────┴───────────────────────┘
  
  free(ptr):
  1. ptr - sizeof(header) → find the header
  2. Read size from header
  3. Mark as free
```

### The Header

A typical chunk header:

```c
typedef struct header {
    size_t size;        // size of the user data (or entire chunk)
    int    is_free;     // 1 = free, 0 = allocated
    struct header *next; // pointer to next free chunk (when in free list)
} header_t;
```

In practice, headers are more compact. glibc's malloc uses boundary tags where the size field includes a bit that indicates allocated/free status:

```
  glibc chunk layout:
  ┌────────────────────────────────┐
  │ prev_size (if previous is free)│  8 bytes (only used if prev chunk is free)
  ├────────────────────────────────┤
  │ size | A | M | P              │  8 bytes (size + 3 flag bits in low bits)
  ├────────────────────────────────┤  ← pointer returned by malloc()
  │                                │
  │ User data                      │
  │                                │
  ├────────────────────────────────┤
  │ (next_size for boundary tag)   │  (at end of chunk when free)
  └────────────────────────────────┘
  
  Flag bits (stored in low bits of size, since chunks are 16-byte aligned):
  P = previous chunk in use (PREV_INUSE)
  M = chunk obtained via mmap (IS_MMAPPED)
  A = chunk belongs to a non-main arena (NON_MAIN_ARENA)
```

---

## 6. Free List

The **free list** is the data structure that tracks all free chunks. The simplest implementation is a singly-linked list.

### Embedded Free List

The clever insight: free chunks aren't being used for user data, so we can **store the free list pointers inside the free chunks themselves**. No extra memory needed!

```
  Memory with embedded free list:
  
  ┌──────────────────────────────────────────────────────────────────┐
  │                                                                  │
  │  ┌────────┬──────────┐  ┌────────┬──────────┐  ┌────────┬─────┐ │
  │  │ Header │ ALLOCATED │  │ Header │  FREE    │  │ Header │FREE │ │
  │  │sz=100  │ user data │  │sz=200  │ next ────┼─►│sz=300  │next │ │
  │  │free=0  │          │  │free=1  │          │  │free=1  │=NULL│ │
  │  └────────┴──────────┘  └────────┴──────────┘  └────────┴─────┘ │
  │                          ▲                                       │
  │                          │                                       │
  │  free_list_head ─────────┘                                       │
  │                                                                  │
  └──────────────────────────────────────────────────────────────────┘
  
  The "next" pointer is stored in the free chunk's data area,
  which is unused since the chunk is free. Zero extra memory cost!
```

When we `malloc(n)`:
1. Walk the free list to find a suitable chunk.
2. If the chunk is much larger than needed, **split** it.
3. Remove the (possibly split) chunk from the free list.
4. Return a pointer past the header.

When we `free(ptr)`:
1. Find the header: `header = ptr - sizeof(header_t)`.
2. Mark as free.
3. Insert into the free list.
4. Optionally **coalesce** with adjacent free chunks.

---

## 7. Splitting and Coalescing

### Splitting

When a free chunk is larger than the requested size, we split it into two parts: one for the allocation, one remains free.

```
  Before malloc(100):
  ┌────────┬──────────────────────────────────────────┐
  │ Header │                FREE (500 bytes)           │
  │ sz=500 │                                          │
  └────────┴──────────────────────────────────────────┘
  
  After malloc(100) with splitting:
  ┌────────┬────────────┐┌────────┬───────────────────────────┐
  │ Header │ ALLOCATED  ││ Header │        FREE (376 bytes)   │
  │ sz=100 │ (100 bytes)││ sz=376 │ (500 - 100 - 24 header)  │
  │ free=0 │            ││ free=1 │                           │
  └────────┴────────────┘└────────┴───────────────────────────┘
                          ▲
                          new header created here
  
  Note: 500 - 100 - sizeof(header) = remaining free space
```

### Coalescing

When a chunk is freed, if its **neighbors** are also free, they should be merged into one larger free chunk. This combats fragmentation.

```
  Before free(B):
  ┌────────┬──────┐┌────────┬──────┐┌────────┬──────┐
  │ hdr    │ FREE ││ hdr    │ USED ││ hdr    │ FREE │
  │ sz=200 │ (A)  ││ sz=100 │ (B)  ││ sz=300 │ (C)  │
  └────────┴──────┘└────────┴──────┘└────────┴──────┘
  
  After free(B) WITHOUT coalescing:
  ┌────────┬──────┐┌────────┬──────┐┌────────┬──────┐
  │ hdr    │ FREE ││ hdr    │ FREE ││ hdr    │ FREE │
  │ sz=200 │ (A)  ││ sz=100 │ (B)  ││ sz=300 │ (C)  │
  └────────┴──────┘└────────┴──────┘└────────┴──────┘
  Three small free chunks. Can't satisfy a 500-byte request!
  
  After free(B) WITH coalescing:
  ┌────────┬─────────────────────────────────────────┐
  │ hdr    │            FREE (A+B+C = 648 bytes)      │
  │ sz=648 │                                          │
  └────────┴─────────────────────────────────────────┘
  One large free chunk. Can satisfy a 500-byte request!
```

**Boundary tags** (storing size at both the beginning and end of each chunk) make coalescing efficient — you can check the previous chunk by looking at the boundary tag just before the current chunk's header.

---

## 8. Free Chunk Selection: First Fit, Best Fit, Worst Fit

When `malloc(n)` is called, which free chunk should we pick?

### First Fit

**Rule**: Walk the free list from the beginning. Use the **first** chunk that's large enough.

```
  Free list: [200B] → [50B] → [500B] → [100B] → NULL
  
  malloc(80):
  Walk: 200B ≥ 80? YES → use this one.
  
  Fast (stops early), but tends to fragment the beginning of memory.
```

**Pros**: Fast — often finds a chunk quickly.
**Cons**: Tends to leave small fragments at the beginning. Address-ordered first fit can mitigate this.

### Best Fit

**Rule**: Search the **entire** free list. Use the smallest chunk that's large enough (tightest fit).

```
  Free list: [200B] → [50B] → [500B] → [100B] → NULL
  
  malloc(80):
  Walk all: 200≥80, 50<80, 500≥80, 100≥80
  Best fit: 100B (smallest that fits)
  
  Minimizes wasted space in the chosen chunk.
```

**Pros**: Minimizes leftover space in the selected chunk.
**Cons**: Slow (must search entire list). Tends to create many tiny unusable fragments.

### Worst Fit

**Rule**: Use the **largest** free chunk.

```
  Free list: [200B] → [50B] → [500B] → [100B] → NULL
  
  malloc(80):
  Worst fit: 500B (largest)
  
  Leftover after split: 500-80-header = ~396B (still usable!)
```

**Pros**: Leftover fragments tend to be large enough to be useful.
**Cons**: Slow (must search entire list). Quickly breaks down large free regions.

### Comparison

| Strategy | Speed | Fragmentation Pattern |
|----------|-------|----------------------|
| First Fit | Fast (early termination) | Fragments cluster at start |
| Best Fit | Slow (full scan) | Many tiny unusable fragments |
| Worst Fit | Slow (full scan) | Rapidly depletes large blocks |
| **Next Fit** | Fast (like FF, but resumes from last position) | Spreads fragmentation evenly |

In practice, **first fit** (or its variant, **next fit**) is most commonly used due to speed. The real-world allocators use more sophisticated approaches (segregated lists, size classes) that sidestep this problem entirely.

---

## 9. Segregated Free Lists and Slab Allocators

### Segregated Free Lists

Instead of one free list for all sizes, maintain **separate free lists for different size classes**:

```
  Segregated Free Lists:
  
  Size class 16:   [16B] → [16B] → [16B] → NULL
  Size class 32:   [32B] → [32B] → NULL
  Size class 64:   [64B] → [64B] → [64B] → [64B] → NULL
  Size class 128:  [128B] → NULL
  Size class 256:  [256B] → [256B] → NULL
  Size class 512:  (empty)
  Size class 1024: [1024B] → NULL
  Size class 2048: [2048B] → NULL
  ...
  Large:           sorted free list or tree for big chunks
  
  malloc(50):
    Round up to size class 64.
    Pop from the 64-byte free list. → O(1)!
  
  free(ptr):
    Read size from header.
    Push onto the appropriate free list. → O(1)!
```

**Advantages**:
- **O(1) allocation and deallocation** for common sizes.
- **Reduced fragmentation**: Chunks in the same list are the same size, so no splitting/coalescing needed for exact-fit requests.
- **Cache-friendly**: Same-sized objects tend to be allocated near each other.

This is the foundation of modern allocators like **jemalloc** (used by FreeBSD, Firefox, Rust), **tcmalloc** (Google), and glibc's **ptmalloc2** (which uses "bins" — small bins, large bins, unsorted bin, fast bins).

### The Slab Allocator (Linux Kernel)

The Linux kernel uses the **slab allocator** (or its successors, SLUB and SLOB) for kernel object allocation. It's a specialized fixed-size pool allocator with per-type caches.

```
  Kernel needs to allocate task_struct objects frequently.
  Each task_struct is ~6 KB.
  
  Slab cache for task_struct:
  ┌─────────────────────────────────────────────────────────┐
  │  Slab (one or more contiguous physical pages)            │
  │                                                         │
  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐   │
  │  │task_struct│ │task_struct│ │task_struct│ │task_struct│   │
  │  │ (used)   │ │ (free)   │ │ (used)   │ │ (free)   │   │
  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘   │
  │                                                         │
  │  Free list: slot1 → slot3 → NULL                        │
  └─────────────────────────────────────────────────────────┘
  
  kmem_cache_alloc(task_struct_cache):
    Pop a free slot from the slab. O(1).
    Object memory may still contain the previous task_struct's data
    → constructor can initialize it.
  
  kmem_cache_free(task_struct_cache, ptr):
    Push slot back onto free list. O(1).
```

**Key slab allocator features**:
- **Per-type caches**: Each kernel object type (task_struct, inode, dentry, sk_buff, etc.) has its own cache.
- **Object reuse**: Freed objects can be reused without re-initialization of constant fields (constructor/destructor callbacks).
- **Cache coloring**: Objects within a slab are offset slightly to reduce cache line conflicts.
- **Per-CPU caches**: Each CPU has a local pool to avoid lock contention (SLUB allocator).

```bash
# View slab caches:
$ cat /proc/slabinfo
$ sudo slabtop    # interactive view

# Output shows:
# task_struct        1234   1500   6784   ...
# inode_cache        5678   6000    800   ...
# dentry             9012  10000    192   ...
```

The current default kernel allocator is **SLUB** (`CONFIG_SLUB`), which is simpler and more scalable than the original SLAB allocator. SLOB is a minimal allocator for memory-constrained embedded systems.

| Allocator | Strengths | Used For |
|-----------|-----------|----------|
| SLAB | Original, feature-rich, cache coloring | Legacy |
| **SLUB** | Simpler, better scalability, less memory overhead | **Default since 2.6.23** |
| SLOB | Minimal, tiny footprint | Embedded systems |

### `kmalloc` and `kfree`

For general-purpose kernel allocations (not tied to a specific object type), Linux provides `kmalloc()`/`kfree()`:

```c
void *kmalloc(size_t size, gfp_t flags);
void kfree(const void *ptr);
```

`kmalloc` internally uses slab caches for common sizes (8, 16, 32, 64, 128, ..., 8192 bytes). For larger allocations, it falls back to the page allocator.

GFP flags control allocation behavior:
- `GFP_KERNEL`: Normal kernel allocation (may sleep/block).
- `GFP_ATOMIC`: Cannot sleep (used in interrupt context).
- `GFP_DMA`: Allocate from DMA-capable memory zone.

---

## 10. Buddy Allocator

The **buddy allocator** is the Linux kernel's **physical page frame allocator** — it manages the pool of free physical pages. When the slab allocator (or `mmap`) needs physical pages, it requests them from the buddy allocator.

### How It Works

The buddy allocator manages free pages in **power-of-two sized blocks**: 1, 2, 4, 8, 16, 32, ..., 1024 contiguous pages (i.e., 4 KB, 8 KB, ..., 4 MB).

```
  Free lists by order:
  
  Order 0 (1 page, 4 KB):    [pg] → [pg] → [pg] → NULL
  Order 1 (2 pages, 8 KB):   [pg pg] → [pg pg] → NULL
  Order 2 (4 pages, 16 KB):  [pg pg pg pg] → NULL
  Order 3 (8 pages, 32 KB):  [pg pg pg pg pg pg pg pg] → NULL
  ...
  Order 10 (1024 pages, 4 MB): [large block] → NULL
```

### Allocation

Request for N pages: round up N to the nearest power of two (say, order k).

1. Check the order-k free list. If a block is available → use it.
2. If not → go to order k+1 free list. Split the block into two **buddies** of order k. Use one, put the other on the order-k free list.
3. If order k+1 is also empty → go to k+2, split recursively.

```
  Example: allocate 1 page (order 0), but only order-2 block available:
  
  Order 2 free list:
  ┌───────────────────────┐
  │ A  │ B  │ C  │ D      │   (4 contiguous pages)
  └───────────────────────┘
  
  Split into two order-1 buddies:
  ┌───────────┐  ┌───────────┐
  │ A  │ B    │  │ C  │ D    │
  └───────────┘  └───────────┘
  Put [C,D] on order-1 free list.
  
  Split [A,B] into two order-0 buddies:
  ┌─────┐  ┌─────┐
  │  A  │  │  B  │
  └─────┘  └─────┘
  Put [B] on order-0 free list.
  
  Return page A to the caller.
```

### Deallocation (Easy Coalescing!)

The key advantage of the buddy system: **coalescing is trivial and efficient**.

When a block of order k is freed:
1. Find its **buddy** (the block it was split from). The buddy's address is computed by XOR-ing the block's address with `(1 << k) × PAGE_SIZE`.
2. If the buddy is also free → merge them into a single order-(k+1) block.
3. Repeat recursively: check the merged block's buddy at order k+1, and so on.

```
  Free page A (order 0):
  
  A's buddy at order 0 = B.
  Is B free? YES → merge into [A,B] (order 1).
  
  [A,B]'s buddy at order 1 = [C,D].
  Is [C,D] free? YES → merge into [A,B,C,D] (order 2).
  
  [A,B,C,D]'s buddy at order 2 = ?
  Not free → stop. Put [A,B,C,D] on order-2 free list.
  
  Buddy address formula:
  buddy_addr = block_addr XOR (1 << (order + PAGE_SHIFT))
  
  Because blocks are power-of-2 aligned, XOR-ing the appropriate
  bit gives the buddy's address. No searching needed!
```

### Why "Buddy" Allocator?

Two blocks are "buddies" if:
1. They are the same size (same order).
2. They are adjacent in memory.
3. They were created by splitting a single larger block.
4. Their combined address is aligned to the next higher order.

The XOR trick works because the addresses of buddies differ in exactly one bit.

### Linux Buddy Allocator

- Implemented in `mm/page_alloc.c`.
- Per-NUMA-node, per-zone (DMA, Normal, HighMem) free lists.
- Functions: `alloc_pages(gfp_mask, order)`, `__free_pages(page, order)`.
- View fragmentation: `cat /proc/buddyinfo`.

```bash
$ cat /proc/buddyinfo
# Node 0, zone   Normal  1234  567  234  123  56  23  12  5  2  1  0
# (columns show number of free blocks at each order: 0, 1, 2, ..., 10)

$ cat /proc/pagetypeinfo    # detailed breakdown by migration type
```

**Memory compaction**: When a large contiguous allocation is needed but memory is fragmented, the kernel can compact memory by migrating movable pages to create larger contiguous blocks. This is done by `kcompactd` or on-demand.

---

## 11. Linux Memory Allocation Stack — Full Picture

```
  User Space:
  ┌──────────────────────────────────────────────────────────────┐
  │  Application: malloc(100)                                     │
  │       │                                                      │
  │       ▼                                                      │
  │  glibc malloc (ptmalloc2):                                    │
  │  - Check thread-local arena (per-thread free lists/bins)      │
  │  - Segregated free lists by size class (fast bins, small bins, │
  │    large bins, unsorted bin)                                  │
  │  - If no suitable chunk:                                      │
  │    - Small: sbrk() to extend heap                             │
  │    - Large (≥128KB): mmap(MAP_ANONYMOUS)                      │
  └──────────────────────────┬───────────────────────────────────┘
                             │ syscall
  ═══════════════════════════╪════════════════════════════════════
  Kernel Space:              │
  ┌──────────────────────────▼───────────────────────────────────┐
  │  VMA management (mm/mmap.c):                                  │
  │  - Create/extend VMAs in the process's mm_struct              │
  │  - On first access → page fault                               │
  │       │                                                      │
  │       ▼                                                      │
  │  Page Fault Handler (mm/memory.c):                            │
  │  - Allocate physical page from:                               │
  │       │                                                      │
  │       ▼                                                      │
  │  SLAB/SLUB Allocator (mm/slub.c):                             │
  │  - For kernel objects (task_struct, inode, dentry, etc.)       │
  │  - Segregated caches per object type                          │
  │  - Per-CPU freelists for speed                                │
  │  - When slab needs more pages:                                │
  │       │                                                      │
  │       ▼                                                      │
  │  Buddy Allocator (mm/page_alloc.c):                           │
  │  - Manages ALL physical page frames                           │
  │  - Power-of-2 block sizes (order 0-10)                        │
  │  - Per-zone, per-node free lists                              │
  │  - Buddy coalescing on free                                   │
  │       │                                                      │
  │       ▼                                                      │
  │  Physical Page Frames (Hardware RAM)                           │
  └──────────────────────────────────────────────────────────────┘
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| `mmap` / `munmap` | Map virtual address ranges (anonymous or file-backed) | Creates VMAs in `mm_struct`; demand-paged |
| `brk` / `sbrk` | Extend heap contiguously | Used by malloc for small allocs |
| `malloc` / `free` | User-space allocator managing chunks within mmap'd/brk'd regions | ptmalloc2 (glibc), jemalloc, tcmalloc |
| General-purpose allocators | Variable-size chunks, free list, split/coalesce | Segregated bins, boundary tags |
| Free chunk selection | First fit (fast), Best fit (tight), Worst fit (large leftovers) | Real allocators use size classes to avoid searching |
| Segregated free lists | Separate lists per size class → O(1) alloc/free | Used by all modern allocators |
| Slab / SLUB allocator | Fixed-size per-type object caches in the kernel | `kmem_cache_create()`, `/proc/slabinfo` |
| Buddy allocator | Power-of-2 physical page blocks, easy coalescing via XOR | `mm/page_alloc.c`, `/proc/buddyinfo` |
| Two-level allocation | User-space allocator + kernel page allocator | Avoids syscall overhead per malloc |
