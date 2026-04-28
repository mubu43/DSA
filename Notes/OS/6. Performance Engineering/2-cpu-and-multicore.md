# CPU and Multi-Core Optimization

---

## 1. Optimizing CPU Performance

When CPU cores are saturated (near 100% utilization), the goal is to **do more useful work per cycle** — or eliminate wasteful work entirely.

### Identifying What's Consuming CPU

Use profiling to determine whether CPU time is spent in:
- **User functions**: Your application code or libraries.
- **Kernel code**: Syscall handling, interrupt processing, context switching, networking stack, filesystem operations.

```bash
# Profile to see user vs. kernel breakdown:
$ perf top
# Shows live sampling: which functions (user + kernel) consume the most CPU.

# Or with perf stat:
$ perf stat ./my_program
# task-clock: total CPU time
# Breakdown: X% user, Y% kernel
```

### Optimizing User-Space CPU Usage

| Problem | Symptom | Fix |
|---------|---------|-----|
| Inefficient algorithm | One function dominates CPU time (flame graph) | Better algorithm / data structure (O(n²) → O(n log n)) |
| Hot loop | Tight loop consuming most cycles | Algorithmic improvement, SIMD, loop unrolling |
| Excessive allocations | `malloc`/`free` high in profile | Arena allocator, object pools, stack allocation |
| Slow libraries | Library function high in profile | Switch to high-performance alternative |
| Unnecessary work | Functions called more often than needed | Memoization, caching results, lazy evaluation |

### Optimizing Kernel CPU Usage

If `perf` shows significant time in kernel functions, identify the kernel subsystem:

| Kernel Overhead | Symptom in Profile | Fix |
|----------------|-------------------|-----|
| Interrupt handling | `do_IRQ`, `irq_handler`, NIC driver functions | Interrupt coalescing, multi-queue NIC, spread IRQs across cores |
| Filesystem overhead | `ext4_*`, `vfs_*`, `page_cache_*` | Faster filesystem, tune mount options, reduce I/O |
| Context switching | `schedule`, `__switch_to`, `finish_task_switch` | Fewer threads, reduce lock contention, CPU pinning |
| Syscall overhead | `entry_SYSCALL_64`, `sys_*` | Batch operations, `io_uring`, `mmap` instead of `read`/`write` |
| Networking stack | `tcp_*`, `ip_*`, `netif_*` | NAPI tuning, kernel bypass (DPDK/XDP), `SO_REUSEPORT` |

```bash
# Spread NIC interrupts across cores (instead of all going to core 0):
$ sudo service irqbalance start
# Or manually set IRQ affinity:
$ echo 2 > /proc/irq/<irq_number>/smp_affinity  # core 1

# Pin a process to specific cores (reduce migrations):
$ taskset -c 0,1 ./my_program    # run on cores 0 and 1 only
# Or use cgroups for finer control
```

---

## 2. Using Multiple Cores: Parallelism

Modern CPUs have many cores. A single-threaded program uses only one core — the rest sit idle. Using multiple threads allows work to be distributed across cores.

```
  Single-threaded on 8-core system:

  Core 0: ████████████████ (100% — running the program)
  Core 1: ░░░░░░░░░░░░░░░░ (idle)
  Core 2: ░░░░░░░░░░░░░░░░ (idle)
  ...
  Core 7: ░░░░░░░░░░░░░░░░ (idle)

  7/8 = 87.5% of CPU capacity is WASTED.

  Multi-threaded (8 threads):

  Core 0: ████████████████ (100%)
  Core 1: ████████████████ (100%)
  Core 2: ████████████████ (100%)
  ...
  Core 7: ████████████████ (100%)

  All cores utilized — up to 8× throughput!
```

### But Parallelism Has Limits

Not all code can run in parallel:
- **Critical sections**: Only one thread at a time (mutual exclusion).
- **Sequential dependencies**: Step B depends on the result of step A.
- **Synchronization overhead**: Lock acquisition, context switches, cache coherence traffic.

---

## 3. Amdahl's Law

Amdahl's Law gives the **theoretical maximum speedup** from parallelism, given that only a fraction of the work can be parallelized.

### Formula

$$
T_p = \alpha \cdot \frac{T_1}{p} + (1 - \alpha) \cdot T_1
$$

$$
\text{Speedup}(p) = \frac{T_1}{T_p} = \frac{1}{(1 - \alpha) + \frac{\alpha}{p}}
$$

Where:
- $T_1$ = execution time on 1 core.
- $T_p$ = execution time on $p$ cores.
- $\alpha$ = fraction of the task that can be parallelized (0 ≤ α ≤ 1).
- $p$ = number of cores.

### Key Insight: The Serial Fraction Dominates

As $p \to \infty$:

$$
\text{Speedup}_{\max} = \frac{1}{1 - \alpha}
$$

The serial (non-parallelizable) portion sets an **absolute upper bound** on speedup, regardless of how many cores you add.

```
  Amdahl's Law — Speedup vs. Cores:

  Speedup
  ^
  │                                        α = 1.0 (fully parallel)
  │                                    ──────────────────── (linear: p×)
  │                               ╱
  │                          ╱                α = 0.95
  │                     ╱          ───────────────────── (max 20×)
  │                ╱         ╱
  │           ╱         ╱              α = 0.75
  │      ╱         ╱        ──────────────────── (max 4×)
  │ ╱         ╱       ╱
  │      ╱       ╱                 α = 0.50
  │  ╱      ╱        ────────────────────── (max 2×)
  │╱   ╱  ╱
  ├───┬───┬───┬───┬───┬───┬───┬───► Cores (p)
  1   2   4   8   16  32  64  128

  Even with 128 cores, if α = 0.95, max speedup is only 20×.
  The 5% serial portion limits everything.
```

### Practical Examples

| Scenario | α | Max Speedup | Speedup with 8 cores |
|----------|---|-------------|---------------------|
| Embarrassingly parallel (image processing, batch rendering) | 0.99 | 100× | 7.5× |
| Matrix multiply with serial setup/reduce | 0.95 | 20× | 5.9× |
| Web server (some shared state) | 0.90 | 10× | 4.7× |
| Database with heavy locking | 0.50 | 2× | 1.8× |
| Mostly sequential algorithm | 0.10 | 1.1× | 1.1× |

### Implication for Optimization

**Minimize the serial fraction.** The biggest performance wins come from:
1. Reducing lock contention (make critical sections shorter or use lock-free structures).
2. Removing unnecessary serialization (avoid global locks, use per-thread data).
3. Using concurrent data structures (concurrent hash maps, lock-free queues).
4. Restructuring algorithms to expose more parallelism.

---

## 4. Reducing Serialization

### Fine-Grained Locking

Replace one big lock with many smaller locks to reduce contention:

```c
// Coarse-grained: one lock for entire hash table
pthread_mutex_t table_lock;

void insert(key, value) {
    pthread_mutex_lock(&table_lock);       // ALL threads contend on this
    // insert into hash table
    pthread_mutex_unlock(&table_lock);
}

// Fine-grained: one lock per bucket
pthread_mutex_t bucket_locks[NUM_BUCKETS];

void insert(key, value) {
    int bucket = hash(key) % NUM_BUCKETS;
    pthread_mutex_lock(&bucket_locks[bucket]);  // only threads hitting
    // insert into bucket                        // same bucket contend
    pthread_mutex_unlock(&bucket_locks[bucket]);
}
```

```
  Coarse-grained:                   Fine-grained:
  ┌──────────────────────┐          ┌──────────────────────┐
  │ ALL operations go    │          │ Operations on        │
  │ through ONE lock     │          │ different buckets    │
  │                      │          │ proceed in PARALLEL  │
  │ T1: ─lock─work─unlock│          │ T1: ─lock[0]─work─  │
  │ T2: ──────lock─work──│          │ T2: ─lock[7]─work─  │
  │ T3: ──────────lock───│          │ T3: ─lock[3]─work─  │
  │ (serialized)         │          │ (parallel!)          │
  └──────────────────────┘          └──────────────────────┘
```

### Read-Write Locks

For read-heavy workloads, allow concurrent readers:

```c
pthread_rwlock_t rwlock;
// Multiple readers in parallel. Only writers need exclusive access.
```

### Lock-Free and Wait-Free Data Structures

Eliminate locks entirely using atomic operations (CAS, fetch-and-add):

```c
// Lock-free stack push (using CAS):
void push(stack_t *s, node_t *node) {
    do {
        node->next = s->top;  // read current top
    } while (!CAS(&s->top, node->next, node));
    // Atomically: if top hasn't changed, set top = node
    // If another thread modified top, retry
}
```

Lock-free structures are complex to implement correctly but eliminate the serial fraction from locking entirely.

### CPU Affinity and NUMA Awareness

On multi-socket systems (NUMA — Non-Uniform Memory Access), memory is local to each CPU socket. Accessing remote memory is 2-3× slower.

```
  NUMA Architecture:

  ┌─────────────────────┐     ┌─────────────────────┐
  │  Socket 0           │     │  Socket 1           │
  │  ┌──────┐ ┌──────┐  │     │  ┌──────┐ ┌──────┐  │
  │  │Core 0│ │Core 1│  │     │  │Core 4│ │Core 5│  │
  │  └──────┘ └──────┘  │     │  └──────┘ └──────┘  │
  │  ┌──────┐ ┌──────┐  │     │  ┌──────┐ ┌──────┐  │
  │  │Core 2│ │Core 3│  │     │  │Core 6│ │Core 7│  │
  │  └──────┘ └──────┘  │     │  └──────┘ └──────┘  │
  │                     │     │                     │
  │  ┌────────────────┐ │     │ ┌────────────────┐  │
  │  │ Local Memory   │ │◄───►│ │ Local Memory   │  │
  │  │ (fast: ~80ns)  │ │ QPI │ │ (fast: ~80ns)  │  │
  │  └────────────────┘ │     │ └────────────────┘  │
  └─────────────────────┘     └─────────────────────┘

  Core 0 accessing Socket 1's memory: ~150ns (remote)
  Core 0 accessing Socket 0's memory: ~80ns (local)
```

```bash
# View NUMA topology:
$ numactl --hardware
$ lscpu | grep NUMA

# Pin process to a NUMA node:
$ numactl --cpunodebind=0 --membind=0 ./my_program
# Run on Socket 0's cores and allocate memory from Socket 0 only
```

---

## 5. Compiler Optimizations

The compiler can generate significantly faster machine code with optimization flags:

```bash
# GCC optimization levels:
$ gcc -O0 program.c    # no optimization (debug builds)
$ gcc -O1 program.c    # basic optimizations
$ gcc -O2 program.c    # standard optimizations (recommended for production)
$ gcc -O3 program.c    # aggressive (auto-vectorization, function inlining, etc.)
$ gcc -Os program.c    # optimize for size (good for cache utilization)
$ gcc -Ofast program.c # O3 + fast-math (may break IEEE float compliance)

# Architecture-specific tuning:
$ gcc -O2 -march=native program.c   # optimize for the current CPU
# Enables AVX, SSE4.2, etc. if available
```

Key optimizations the compiler performs:
- **Inlining**: Replace function call with function body (eliminates call overhead).
- **Loop unrolling**: Reduce loop control overhead by executing multiple iterations per loop step.
- **Vectorization (SIMD)**: Process multiple data elements per instruction using SSE/AVX.
- **Dead code elimination**: Remove code that can never execute.
- **Constant propagation**: Replace variables with their known constant values at compile time.
- **Register allocation**: Keep frequently used variables in CPU registers instead of memory.

```bash
# Profile-Guided Optimization (PGO):
# 1. Build with instrumentation:
$ gcc -fprofile-generate -O2 program.c -o program
# 2. Run with representative workload (generates profile data):
$ ./program < typical_input.txt
# 3. Rebuild using the profile:
$ gcc -fprofile-use -O2 program.c -o program_optimized
# Compiler uses actual runtime data to make better optimization decisions
# (branch prediction hints, hot/cold path splitting, etc.)
```

---

## 6. Hardware Accelerators

When CPU optimization isn't enough, offload compute-intensive work to specialized hardware:

| Accelerator | Best For | Interface |
|------------|---------|-----------|
| **GPU** (Graphics Processing Unit) | Massive parallelism: image/video processing, ML inference, scientific computing | CUDA (NVIDIA), OpenCL, Vulkan |
| **FPGA** (Field-Programmable Gate Array) | Custom logic at hardware speed: packet processing, cryptography | Verilog/VHDL, HLS |
| **NPU** (Neural Processing Unit) | Neural network inference | Vendor SDKs |
| **Crypto accelerator** | AES, SHA, RSA at hardware speed | `AF_ALG` socket, OpenSSL engine |

```bash
# Check if CPU has AES-NI (hardware crypto):
$ grep -o aes /proc/cpuinfo | head -1
aes
# OpenSSL automatically uses AES-NI when available.
```

---

## Summary

| Concept | Key Idea |
|---------|----------|
| CPU optimization | Profile → find hot functions → optimize algorithms, reduce syscalls, tune kernel |
| Kernel CPU overhead | Interrupt handling, context switching, syscall overhead; tune or bypass |
| Parallelism | Use multiple threads to utilize multiple cores |
| Amdahl's Law | Speedup = $\frac{1}{(1-\alpha) + \alpha/p}$; serial fraction limits scaling |
| Minimize serialization | Shorter critical sections, fine-grained locks, lock-free structures |
| NUMA awareness | Pin threads to local NUMA node; avoid remote memory access |
| CPU affinity | `taskset`, `numactl` to pin processes/threads to specific cores |
| Compiler optimization | `-O2`/`-O3`, `-march=native`, PGO for best codegen |
| Hardware accelerators | GPU, FPGA, crypto for specialized workloads |
