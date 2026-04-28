# Memory and CPU Cache Optimization

---

## 1. Memory Optimization: The Big Picture

When a system's memory is under pressure — high utilization, frequent page faults, swapping to disk — performance degrades catastrophically. Disk is 100,000× slower than DRAM.

### Thrashing

When the working set of all processes exceeds available physical memory, the OS constantly pages data in and out of swap. This is **thrashing**: the system spends most of its time moving pages between RAM and disk rather than doing useful work.

```
  Thrashing:

  Throughput
  ^
  │        ╱╲
  │       ╱  ╲
  │      ╱    ╲
  │     ╱      ╲───────── throughput collapses
  │    ╱
  │   ╱
  │  ╱
  │ ╱
  ├──────────────────────► Memory pressure
       Normal    Thrashing
       operation zone

  In the thrashing zone:
  - CPU utilization drops (waiting for disk I/O)
  - Disk utilization spikes (constant page-in/page-out)
  - Response times explode (10× to 1000× slower)
```

### Strategies for Reducing Memory Footprint

| Strategy | Details |
|----------|---------|
| Use smaller data types | `int16_t` instead of `int64_t` when the range fits |
| Compress in-memory data | LZ4/Snappy for cold data (fast decompression) |
| Memory-mapped files | `mmap()` — let the kernel page in only what's accessed |
| Reduce per-object overhead | Avoid bloated class hierarchies, vtable pointers if unnecessary |
| Use memory pools / arenas | Pre-allocate a large block, sub-allocate from it (reduces fragmentation) |
| Free memory promptly | Don't hold onto allocations longer than needed |
| Shared memory | Multiple processes share the same pages (`mmap` with `MAP_SHARED`, `shmget`) |

### Huge Pages for TLB Efficiency

Recall from memory management: the TLB (Translation Lookaside Buffer) caches virtual-to-physical page mappings. With 4 KB pages and a large address space, TLB misses become frequent because the TLB can only hold a limited number of entries.

**Huge pages** (2 MB or 1 GB) mean each TLB entry covers a much larger range of memory, drastically reducing TLB misses.

```
  Regular pages:                     Huge pages:
  ┌────────┐                         ┌────────────────────────────────┐
  │  4 KB  │  TLB entry 0            │           2 MB                │ TLB entry 0
  ├────────┤                         │  (covers 512× more memory     │
  │  4 KB  │  TLB entry 1            │   than a 4 KB page)           │
  ├────────┤                         └────────────────────────────────┘
  │  4 KB  │  TLB entry 2
  ├────────┤                         For 1 GB of memory:
  │  ...   │  ...                    Regular: 262,144 pages → many TLB misses
  ├────────┤                         Huge:    512 pages → fits in TLB
  │  4 KB  │  TLB entry N
  └────────┘
```

```bash
# Check TLB miss rate:
$ perf stat -e dTLB-load-misses,dTLB-loads ./my_program
# dTLB-load-misses / dTLB-loads = TLB miss rate

# Using huge pages in Linux:
# Option 1: Transparent Huge Pages (THP) — automatic, enabled by default
$ cat /sys/kernel/mm/transparent_hugepage/enabled
[always] madvise never

# Option 2: Explicit huge pages via mmap
# In code:
void *p = mmap(NULL, size, PROT_READ|PROT_WRITE,
               MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0);

# Option 3: Reserve explicit huge pages at boot
$ echo 1024 > /proc/sys/vm/nr_hugepages   # reserve 1024 × 2MB = 2GB
# Application allocates from hugetlbfs mount
```

---

## 2. CPU Caches: The Memory Hierarchy

### Why Caches Matter

CPU clock speeds are vastly faster than DRAM access. Without caches, the CPU would stall for ~100ns on every memory access — wasting hundreds of cycles.

```
  Memory Hierarchy — Latency & Size:

  ┌─────────────┐
  │  Registers  │  ~0.3 ns   ~KB      ◄── fastest, smallest
  ├─────────────┤
  │  L1 Cache   │  ~1 ns     32-64 KB  (per core)
  ├─────────────┤
  │  L2 Cache   │  ~4 ns     256 KB-1 MB (per core)
  ├─────────────┤
  │  L3 Cache   │  ~10-20 ns  8-64 MB  (shared across cores)
  ├─────────────┤
  │  DRAM       │  ~80 ns    16-512 GB
  ├─────────────┤
  │  SSD        │  ~100 μs   TB range
  ├─────────────┤
  │  HDD        │  ~10 ms    TB range  ◄── slowest, largest
  └─────────────┘

  Each level is ~3-10× slower but ~10-100× larger.
  Caches exploit LOCALITY to keep frequently/recently used
  data close to the CPU.
```

### Cache Line: The Unit of Transfer

Data moves between cache and memory in fixed-size blocks called **cache lines** (typically **64 bytes** on x86).

When the CPU reads a single byte, the entire 64-byte cache line containing that byte is loaded from memory. This means:
- **Spatial locality**: Accessing nearby data is essentially free — it's already in the cache line.
- **False sharing**: Two unrelated variables in the same cache line can cause inter-core cache invalidation traffic (more on this later).

```
  Cache Line (64 bytes):

  Address: 0x1000  0x1004  0x1008  ...        0x103C
           ┌───────┬───────┬───────┬───────────┬───────┐
           │ int a │ int b │ int c │    ...    │ int p │
           └───────┴───────┴───────┴───────────┴───────┘
           ◄──────────── 64 bytes ──────────────────────►
           All 16 ints loaded together into the cache.

  If you access 'a', then 'b', 'c', ... are already cached.
  This is why iterating an array is fast — sequential access
  hits the cache line.
```

### Locality of Reference

Performance depends heavily on how your code accesses memory:

**Temporal locality**: Data accessed recently will likely be accessed again soon. Keep it in cache by reusing it quickly.

**Spatial locality**: Data near recently accessed data will likely be accessed next. Cache lines exploit this: load a block, expect nearby accesses.

```
  Good Locality (sequential array access):

  int arr[1000000];
  for (int i = 0; i < 1000000; i++)
      sum += arr[i];        // Sequential: each cache line used fully
                             // 1 miss per 16 ints (64B / 4B)

  Bad Locality (random access):

  for (int i = 0; i < 1000000; i++)
      sum += arr[rand() % 1000000];  // Random: each access likely a cache miss
                                      // ~1 miss per access!
```

---

## 3. Cache Organization

### Cache Hierarchy

```
  Per-Core vs. Shared Caches:

  ┌──────────────────────────────────────────────┐
  │ Core 0              Core 1                   │
  │ ┌──────────┐        ┌──────────┐             │
  │ │ L1i  L1d │        │ L1i  L1d │  ◄── per-core, private
  │ │ 32K  32K │        │ 32K  32K │      (fastest, smallest)
  │ ├──────────┤        ├──────────┤             │
  │ │   L2     │        │   L2     │  ◄── per-core, private
  │ │  256 KB  │        │  256 KB  │      (fast, medium)
  │ └────┬─────┘        └────┬─────┘             │
  │      └──────────┬────────┘                   │
  │           ┌─────┴─────┐                      │
  │           │    L3     │  ◄── shared across all cores
  │           │  8-64 MB  │      (slower, largest on-chip)
  │           └─────┬─────┘                      │
  └─────────────────┼────────────────────────────┘
                    │
              ┌─────┴─────┐
              │   DRAM    │
              │  (main    │
              │  memory)  │
              └───────────┘
```

- **L1i / L1d**: Separate caches for instructions and data. Fastest: ~1ns, ~32KB each.
- **L2**: Unified (instructions + data). ~4ns, ~256KB per core.
- **L3**: Shared across all cores. ~10-20ns, 8-64MB. Also called LLC (Last Level Cache).

### Cache Miss Types

| Type | Description | Example |
|------|------------|---------|
| **Cold (compulsory)** miss | First access to data — it has never been cached | Program startup, first iteration |
| **Capacity** miss | Cache is full; old data was evicted to make room | Working set exceeds cache size |
| **Conflict** miss | Multiple addresses map to the same cache set | Hash collisions in set-associative cache |
| **Coherence** miss | Another core invalidated this line (cache coherence) | Shared data modified by another core |

---

## 4. Optimizing Cache Usage

### Strategy 1: Keep Data Small

Smaller data structures → smaller working set → more likely to fit in cache.

```c
// Bloated: 24 bytes per entry
struct Entry_Bad {
    int64_t value;       // 8 bytes  (but value fits in 32 bits!)
    int64_t flags;       // 8 bytes  (but only 8 flags needed!)
    int64_t padding;     // 8 bytes  (unused)
};

// Compact: 8 bytes per entry
struct Entry_Good {
    int32_t value;       // 4 bytes
    uint8_t flags;       // 1 byte
    uint8_t pad[3];      // 3 bytes (alignment)
};
// 3× smaller → 3× more entries fit in cache
```

### Strategy 2: Store Related Data Together (Structure of Arrays)

```c
// Array of Structures (AoS) — poor cache usage for column operations:
struct Particle { float x, y, z, vx, vy, vz, mass; };
struct Particle particles[N];
// Iterating over just 'x' loads y, z, vx, vy, vz, mass too → wastes cache

// Structure of Arrays (SoA) — great cache usage for column operations:
struct Particles {
    float x[N], y[N], z[N];
    float vx[N], vy[N], vz[N];
    float mass[N];
};
// Iterating over 'x' values: sequential, full cache line utilization
```

```
  AoS Memory Layout:
  [x0 y0 z0 vx0 vy0 vz0 m0] [x1 y1 z1 vx1 vy1 vz1 m1] [x2 ...]
  ◄── cache line ──────────► ◄── cache line ──────────────►
  Accessing only x: loads 7 fields per particle, uses 1.

  SoA Memory Layout:
  [x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 x10 x11 x12 x13 x14 x15] [x16 ...]
  ◄── cache line: 16 x-values ────────────────────────────────►
  Accessing only x: loads 16 useful values per cache line!
```

### Strategy 3: Row-Major Access for Matrices

In C/C++, 2D arrays are stored in **row-major** order: row 0, then row 1, etc.

```c
// GOOD: row-major access (follows memory layout)
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        sum += matrix[i][j];
// Accesses: [0][0] [0][1] [0][2] ... → sequential → cache-friendly

// BAD: column-major access (jumps across rows)
for (int j = 0; j < N; j++)
    for (int i = 0; i < N; i++)
        sum += matrix[i][j];
// Accesses: [0][0] [1][0] [2][0] ... → stride = N × sizeof(int)
// Each access likely misses the cache for large N
```

```
  Row-Major Memory Layout (C/C++):

  matrix[0][0] matrix[0][1] matrix[0][2] matrix[0][3]   ◄── Row 0
  matrix[1][0] matrix[1][1] matrix[1][2] matrix[1][3]   ◄── Row 1
  matrix[2][0] matrix[2][1] matrix[2][2] matrix[2][3]   ◄── Row 2

  In memory:
  [0,0] [0,1] [0,2] [0,3] [1,0] [1,1] [1,2] [1,3] [2,0] ...
        ◄── contiguous ──►

  Row access:    [0,0] → [0,1] → [0,2] → ...  stride = 4B → HITS cache
  Column access: [0,0] → [1,0] → [2,0] → ...  stride = N×4B → MISSES cache
```

### Strategy 4: Loop Fusion

Combine separate loops that access the same data:

```c
// Before: Two passes over the same array
for (int i = 0; i < N; i++)
    a[i] = b[i] + c[i];
for (int i = 0; i < N; i++)
    d[i] = a[i] * 2;

// After: Single pass — a[i] still in cache when used
for (int i = 0; i < N; i++) {
    a[i] = b[i] + c[i];
    d[i] = a[i] * 2;        // a[i] is still in L1 cache!
}
```

### Strategy 5: Cache-Oblivious and Tiled Algorithms

For operations on large matrices (e.g., matrix multiply), process data in **tiles** that fit in cache:

```c
// Naive matrix multiply: C = A × B
// For large N, columns of B jump across memory → terrible cache usage
for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        for (int k = 0; k < N; k++)
            C[i][j] += A[i][k] * B[k][j];

// Tiled (blocked) matrix multiply:
// Process TILE_SIZE × TILE_SIZE blocks that fit in L1 cache
#define TILE 64
for (int ii = 0; ii < N; ii += TILE)
  for (int jj = 0; jj < N; jj += TILE)
    for (int kk = 0; kk < N; kk += TILE)
      for (int i = ii; i < ii+TILE && i < N; i++)
        for (int j = jj; j < jj+TILE && j < N; j++)
          for (int k = kk; k < kk+TILE && k < N; k++)
            C[i][j] += A[i][k] * B[k][j];
// Each tile fits in cache → dramatically fewer cache misses
```

---

## 5. Cache Coherence and False Sharing

### Cache Coherence Protocol (MESI)

In multi-core systems, each core has its own L1/L2 cache. When multiple cores cache the same memory location, the hardware must keep them **coherent** — i.e., all cores must see a consistent view.

The **MESI** protocol (Modified, Exclusive, Shared, Invalid) is the most common:

| State | Meaning |
|-------|---------|
| **M** (Modified) | This core has the only valid copy, and it's been modified. Must write back before another core can read. |
| **E** (Exclusive) | This core has the only cached copy, and it matches memory. Can transition to M on write without bus traffic. |
| **S** (Shared) | Multiple cores have this line cached. All copies match memory. Must invalidate others before writing. |
| **I** (Invalid) | This cache line is not valid (evicted or invalidated). Must fetch from memory/other cache on next access. |

```
  MESI State Transitions:

  Core 0 reads X (not cached anywhere):
    Core 0: I → E  (exclusive: sole owner)

  Core 1 reads X:
    Core 0: E → S  (now shared)
    Core 1: I → S  (shared copy)

  Core 0 writes X:
    Core 0: S → M  (modified: only valid copy)
    Core 1: S → I  (invalidated via bus snoop)
    ^^^ Core 1 must re-fetch X from Core 0's cache or memory next time!
```

### True Sharing

When multiple cores genuinely access the **same variable**, coherence traffic is unavoidable — this is **true sharing**. Minimize it by reducing shared mutable state.

### False Sharing

**False sharing** occurs when two cores access **different variables** that happen to reside on the **same cache line**. Each core's write invalidates the other core's cache line, even though they're working on independent data.

```c
// FALSE SHARING — two threads, two different counters
struct Counters {
    int counter_thread_0;   // 4 bytes
    int counter_thread_1;   // 4 bytes  (same cache line!)
};

// Thread 0 increments counter_thread_0
// Thread 1 increments counter_thread_1
// BOTH are in the same 64-byte cache line!
// Every increment invalidates the other core's cache → massive slowdown
```

```
  False Sharing:

  Cache line (64 bytes):
  ┌─────────────────┬─────────────────┬──────────────────────┐
  │ counter_thread_0│ counter_thread_1│      (unused)        │
  │   (Core 0)      │   (Core 1)      │                      │
  └─────────────────┴─────────────────┴──────────────────────┘

  Core 0 writes counter_thread_0:
    → Invalidates ENTIRE line in Core 1's cache
  Core 1 writes counter_thread_1:
    → Invalidates ENTIRE line in Core 0's cache

  This "ping-pong" can slow code by 10-50× vs. no false sharing!
```

### Fixing False Sharing: Padding / Alignment

```c
// Pad each counter to its own cache line:
struct Counters {
    alignas(64) int counter_thread_0;   // starts at cache line boundary
    alignas(64) int counter_thread_1;   // on a DIFFERENT cache line
};
// Now each counter is on its own 64-byte cache line — no false sharing.

// Alternative: manual padding
struct Counters {
    int counter_thread_0;
    char pad[60];             // fill rest of cache line
    int counter_thread_1;
};
```

### Lock Contention and Cache Line Bouncing

A spinlock or mutex variable is shared and modified by multiple cores. Each lock/unlock:
1. Core acquires lock → writes to lock variable → cache line moves to this core (MESI: M).
2. Other cores trying to acquire → their cached copy is invalidated → must fetch from owning core.

Heavy lock contention causes the lock's cache line to **bounce** between cores constantly — massive interconnect traffic and latency.

**Mitigation**:
- Reduce lock hold time (shorter critical sections).
- Per-core / per-thread data where possible.
- Use MCS or CLH queue-based locks (each thread spins on its own cache line).

---

## 6. Advanced Memory Optimization

### Sequential vs. Random Access

The CPU **hardware prefetcher** detects sequential access patterns and pre-loads the next few cache lines from memory before the program asks for them — effectively hiding memory latency.

```
  Sequential Access (prefetcher helps):
  ┌────┬────┬────┬────┬────┬────┬────┬────┐
  │ L1 │ L1 │ L1 │ L1 │PREF│PREF│    │    │
  │hit │hit │hit │hit │load│load│    │    │
  └────┴────┴────┴────┴────┴────┴────┴────┘
  Prefetcher detects stride, loads ahead → near-zero stalls.

  Random Access (prefetcher can't help):
  ┌────┐ ┌────┐   ┌────┐         ┌────┐
  │miss│ │miss│   │miss│         │miss│
  └────┘ └────┘   └────┘         └────┘
  No pattern → every access waits ~80ns for DRAM → very slow.
```

**Design principle**: Prefer arrays over linked lists, hash maps over tree maps when traversal order matters — arrays have sequential layout, linked structures do not.

### Pre-allocation vs. Dynamic Allocation

`malloc()` involves:
- Searching free lists, potentially calling `brk()`/`mmap()` for more memory.
- TLB misses for newly mapped pages.
- Page faults for first access to new pages (demand paging).

**Pre-allocation**: Allocate all needed memory upfront, before the critical path.

```c
// Bad: allocating in hot loop
for (int i = 0; i < N; i++) {
    struct Node *n = malloc(sizeof(struct Node));  // syscall, fragmentation
    process(n);
}

// Good: pre-allocate a pool
struct Node *pool = malloc(N * sizeof(struct Node));  // one allocation
for (int i = 0; i < N; i++) {
    process(&pool[i]);    // no allocation, sequential memory → cache-friendly
}
free(pool);
```

### Custom Allocators and Slab Allocation

**Arena (bump) allocator**: Maintain a pointer into a large pre-allocated block. Allocation is just incrementing the pointer — O(1), no fragmentation, excellent cache locality.

```c
// Arena allocator:
char arena[1 << 20];  // 1 MB arena
size_t offset = 0;

void *arena_alloc(size_t size) {
    void *ptr = &arena[offset];
    offset += (size + 7) & ~7;  // align to 8 bytes
    return ptr;
}
// Deallocation: reset offset to 0 (free everything at once)
```

**Slab allocator** (used extensively in the Linux kernel): Pre-allocate pools of fixed-size objects (e.g., `struct task_struct`, `struct inode`). Each "slab" is a page-sized chunk of identical objects.

```
  Linux Slab Allocator:

  kmem_cache("task_struct"):
  ┌─────────────────────────────────────────────┐
  │ Slab 0 (4 KB page)                         │
  │ ┌──────────┬──────────┬──────────┬────────┐ │
  │ │task_struct│task_struct│task_struct│ free  │ │
  │ └──────────┴──────────┴──────────┴────────┘ │
  ├─────────────────────────────────────────────┤
  │ Slab 1 (4 KB page)                         │
  │ ┌──────────┬──────────┬──────────┬────────┐ │
  │ │task_struct│  free    │  free    │ free   │ │
  │ └──────────┴──────────┴──────────┴────────┘ │
  └─────────────────────────────────────────────┘

  Benefits:
  - No fragmentation (all objects same size)
  - Fast allocation (just pop from free list)
  - Objects stay warm in cache (reused immediately)
```

```bash
# View kernel slab caches:
$ cat /proc/slabinfo | head -20
$ slabtop            # interactive view
```

### Using `mmap` for Anonymous Memory

For large allocations, `mmap` with `MAP_ANONYMOUS` directly requests pages from the kernel — bypassing the `malloc` heap entirely.

```c
// Large allocation via mmap:
void *p = mmap(NULL, size, PROT_READ | PROT_WRITE,
               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
// Returns page-aligned memory, backed by demand paging.
// Free with munmap(p, size).
```

Advantages:
- Page-aligned (good for huge pages).
- No fragmentation from heap metadata.
- Pages returned to OS on `munmap` (unlike `free`, which may keep them in heap).

### Avoiding Unnecessary Copies

Every `memcpy` consumes memory bandwidth and cache space. Reduce copies by:

| Technique | Description |
|-----------|-------------|
| **Zero-copy I/O** | `sendfile()`, `splice()` — move data between fd's without copying to userspace |
| **Pass by reference** | Pass pointers/references instead of copying structs |
| **mmap for file I/O** | Access file data directly in memory instead of `read()` into a buffer |
| **Move semantics** (C++) | Transfer ownership of resources instead of deep-copying |
| **Scatter-gather I/O** | `readv()`/`writev()` — read/write multiple buffers in one syscall |

```bash
# sendfile: kernel copies directly from file to socket (zero-copy)
# Used by web servers (nginx, Apache) for static file serving:
sendfile(socket_fd, file_fd, &offset, count);
# No data ever enters userspace — kernel does it all.
```

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Thrashing | Working set > RAM → constant paging → throughput collapse |
| Huge pages | 2 MB/1 GB pages → fewer TLB misses for large memory use |
| Cache hierarchy | L1 (~1ns, 32KB) → L2 (~4ns, 256KB) → L3 (~15ns, shared) → DRAM (~80ns) |
| Cache line | 64 bytes — unit of transfer; basis for spatial locality |
| Spatial locality | Access nearby data → already in cache line → fast |
| Temporal locality | Reuse recently accessed data → still in cache → fast |
| Row-major access | Iterate arrays in memory order (row by row in C) |
| SoA vs AoS | Structure of Arrays better for column access patterns |
| Loop fusion | Combine loops over same data → reuse data while in cache |
| Tiling | Process data in cache-sized blocks for matrix operations |
| MESI protocol | Cache coherence: Modified, Exclusive, Shared, Invalid |
| False sharing | Different variables on same cache line → invalidation ping-pong |
| Fix false sharing | `alignas(64)` or padding to separate cache lines |
| Prefetcher | Hardware detects sequential patterns → loads ahead |
| Pre-allocation | Allocate memory upfront → avoid `malloc` on hot path |
| Slab allocator | Fixed-size object pools → fast, no fragmentation (Linux kernel) |
| Zero-copy | `sendfile()`, `mmap` → avoid copying data between kernel and userspace |
