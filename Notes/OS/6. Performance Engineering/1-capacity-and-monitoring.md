# System Capacity, Monitoring, and Profiling

---

## 1. What Is System Capacity?

**Capacity** is the maximum throughput a system can sustain. When a system runs at capacity (saturation), some hardware resource at the **bottleneck component** is fully (100%) utilized — there are no spare cycles, bytes, or bandwidth to handle more work.

```
  System at Capacity — The Bottleneck:

  ┌─────────┐     ┌──────────┐     ┌─────────┐
  │ Network │ ──► │   CPU    │ ──► │  Disk   │
  │  (40%)  │     │  (100%)  │     │  (20%)  │
  └─────────┘     └──────────┘     └─────────┘
                   ^^^^^^^^^^^
                   BOTTLENECK
                   CPU is saturated.
                   Adding more disk or network
                   capacity won't help.
                   Must optimize CPU usage
                   or add more CPU cores.
```

### Types of Bottlenecks

**Hardware bottleneck**: A physical resource is fully utilized.
- All CPU cores at 100% utilization — no spare compute.
- RAM is full — excessive swapping/thrashing.
- Disk I/O bandwidth saturated — reads/writes queuing up.
- Network link at line rate — packets being dropped.

**Software bottleneck**: No hardware resource is fully utilized, but the system still can't handle more load. The limitation is in the software design or configuration.
- Maximum number of file descriptors reached — can't open more sockets.
- Threads wasting time on lock contention — CPU is idle but work isn't progressing.
- Thread pool too small — CPU cores are free but there are no threads to use them.
- Queue/buffer sizes too small — requests being dropped.

```
  Hardware vs. Software Bottleneck:

  Hardware:                         Software:
  ┌──────────────────────┐          ┌──────────────────────┐
  │ CPU: 100% ◄── bottleneck        │ CPU: 30%             │
  │ Disk: 40%           │          │ Disk: 15%            │
  │ RAM: 70%            │          │ RAM: 40%             │
  │ NIC: 20%            │          │ NIC: 10%             │
  │                     │          │                      │
  │ Throughput plateaus  │          │ Throughput plateaus   │
  │ because CPU is maxed │          │ but NO resource is   │
  │ out.                │          │ maxed out!            │
  │                     │          │ Something in software │
  │ Fix: optimize CPU   │          │ is the limit.        │
  │ usage or add cores  │          │ Fix: tune parameters │
  └──────────────────────┘          └──────────────────────┘
```

### How to Improve Capacity

1. **Optimize**: Make the bottleneck component use its hardware resources more efficiently (better algorithms, fewer syscalls, less lock contention).
2. **Scale vertically**: Add more of the bottleneck resource to the same machine (more cores, more RAM, faster disk).
3. **Scale horizontally**: Add more machines and distribute the load.
4. **Tune software parameters**: Increase thread pool sizes, buffer sizes, file descriptor limits, etc.

---

## 2. Monitoring Hardware Resource Usage

The first step in performance engineering: **measure**. Identify which resource is saturated.

### CPU Monitoring

```bash
# top: real-time process-level CPU usage
$ top
# Key columns: %CPU (per-process), us (user), sy (system), id (idle), wa (I/O wait)
# If 'id' (idle) is near 0% across all cores → CPU saturated

# htop: better interactive view (per-core bars)
$ htop

# mpstat: per-core CPU utilization
$ mpstat -P ALL 1
# Shows %usr, %sys, %iowait, %idle for EACH core every 1 second

# pidstat: CPU usage per process/thread
$ pidstat -t 1    # -t shows threads
```

```
  Reading 'top' Output:

  %Cpu(s):  82.3 us,  12.1 sy,   0.0 ni,   4.2 id,   1.0 wa,   0.4 si
             │          │                     │          │          │
             │          │                     │          │          └ software interrupts
             │          │                     │          └ I/O wait (CPU idle, waiting for disk)
             │          │                     └ idle (spare CPU)
             │          └ system (kernel) time
             └ user time

  4.2% idle → CPU is 95.8% utilized → near saturation!
  If 'wa' is high → disk I/O is the bottleneck, not CPU.
```

### Memory Monitoring

```bash
# free: memory usage overview
$ free -h
              total     used     free    shared  buff/cache   available
Mem:           16G      12G      500M     200M     3.5G        3.2G
Swap:          4G       2G       2G

# If swap is heavily used → memory pressure, possible thrashing
# 'available' is the key: memory the kernel can reclaim for applications

# vmstat: virtual memory statistics
$ vmstat 1
# si/so = swap in/out (pages swapped from/to disk per second)
# If si/so > 0 continuously → swapping → memory bottleneck

# /proc/meminfo: detailed breakdown
$ cat /proc/meminfo | head -20
```

### Disk I/O Monitoring

```bash
# iostat: disk utilization and throughput
$ iostat -xz 1
Device   r/s    w/s    rkB/s   wkB/s  await  %util
sda      150    80     4800    2400   12.5   92.3
#                                            ^^^^
# %util near 100% → disk is saturated!
# await: average time per I/O request (ms) — high = queuing

# iotop: per-process disk I/O
$ sudo iotop
```

### Network Monitoring

```bash
# sar -n DEV: network interface throughput
$ sar -n DEV 1
IFACE   rxpck/s  txpck/s  rxkB/s  txkB/s   %ifutil
eth0    150000   80000    980000  250000    78.4
#                                           ^^^^
# %ifutil approaching 100% → link saturated

# nstat / ss: socket statistics
$ ss -s          # socket summary (established, closed, etc.)
$ nstat          # kernel network counters

# ethtool -S: NIC-level statistics (drops, errors)
$ ethtool -S eth0 | grep -i drop
```

### Summary of Monitoring Tools

```
  ┌──────────────┬─────────────────────────────────────────────┐
  │  Resource    │  Tools                                      │
  ├──────────────┼─────────────────────────────────────────────┤
  │  CPU         │  top, htop, mpstat, pidstat, sar            │
  │  Memory      │  free, vmstat, /proc/meminfo, pmap          │
  │  Disk I/O    │  iostat, iotop, blktrace                    │
  │  Network     │  sar -n DEV, ss, nstat, ethtool -S          │
  │  General     │  dstat, glances, nmon (all-in-one)          │
  └──────────────┴─────────────────────────────────────────────┘
```

---

## 3. Profiling: Finding the Root Cause

Monitoring tells you **which resource** is saturated. **Profiling** tells you **why** — which parts of your code are responsible.

### What Profilers Do

Profilers monitor program execution and help:
- **Count hardware events**: CPU cycles, cache misses, branch mispredictions, TLB misses.
- **Count software events**: Page faults, context switches, syscalls.
- **Attribute events to code**: Which function is causing the most cache misses? Which call path consumes the most CPU time?
- **Understand CPU time distribution**: What percentage of time is spent in each user/kernel function?

### Linux `perf` — The Primary Profiling Tool

`perf` is the standard Linux profiling tool. It uses hardware performance counters (PMCs) built into the CPU and kernel tracepoints.

```bash
# Record CPU profile of a program (sampling):
$ perf record -g ./my_program     # -g captures call stacks
# This samples the program counter at regular intervals (e.g., every 1ms)
# and records which function was running.

# Analyze the recording:
$ perf report
# Shows a breakdown: which functions used the most CPU time.

# Example output:
#  Overhead   Command       Shared Object     Symbol
#  ........   .........     .............     ......
#    42.31%   my_program    my_program        [.] matrix_multiply
#    18.72%   my_program    libc.so           [.] malloc
#    12.05%   my_program    [kernel]          [k] copy_user_generic
#     8.41%   my_program    my_program        [.] parse_input
#     ...

# → 42% of CPU time in matrix_multiply — optimize this first!
# → 18% in malloc — consider a memory pool or arena allocator.
# → 12% in kernel copy — consider mmap to avoid copies.
```

### Counting Specific Events

```bash
# Count cache misses during execution:
$ perf stat -e cache-misses,cache-references,instructions,cycles ./my_program

 Performance counter stats for './my_program':
       12,345,678  cache-misses     # 8.2% of all cache references
      150,234,567  cache-references
    5,678,901,234  instructions     # 1.2 insn per cycle
    4,732,417,890  cycles

# 8.2% cache miss rate is high — investigate data access patterns.

# Count context switches and page faults:
$ perf stat -e context-switches,page-faults,cpu-migrations ./my_program
```

### Flame Graphs

**Flame graphs** are a visualization of profiling data (popularized by Brendan Gregg). They show the call stack hierarchy and where CPU time is spent.

```
  Flame Graph (ASCII approximation):

  ┌────────────────────────────────────────────────────┐
  │                    main()                          │
  ├──────────────────────────┬─────────────────────────┤
  │     process_request()    │    handle_connection()  │
  ├───────────┬──────────────┤                         │
  │ parse()   │ compute()    │                         │
  │           ├──────┬───────┤                         │
  │           │sort()│hash() │                         │
  └───────────┴──────┴───────┴─────────────────────────┘

  Width = proportion of CPU time.
  compute() → sort() is the widest = most CPU time.
  Optimize sort() for the biggest impact.
```

```bash
# Generate a flame graph:
$ perf record -g ./my_program
$ perf script | stackcollapse-perf.pl | flamegraph.pl > flame.svg
# Open flame.svg in a browser — interactive!
```

### Other Profiling Tools

| Tool | Purpose |
|------|---------|
| `perf` | General-purpose: CPU sampling, hardware counters, tracepoints |
| `perf top` | Live, top-like view of functions using the most CPU |
| `strace` | Trace system calls (shows which syscalls and their latency) |
| `ltrace` | Trace library calls |
| `bpftrace` | eBPF-based dynamic tracing (custom probes) |
| `Valgrind` (cachegrind) | Cache simulation — detailed cache miss analysis |
| Intel VTune | Advanced CPU profiling (Intel CPUs) |

```bash
# strace: see what syscalls a program makes and how long they take
$ strace -c ./my_program          # summary: count and time per syscall
$ strace -T ./my_program          # per-call: show time for each syscall
```

---

## 4. The Profiling Workflow

```
  ┌──────────────────────────────────────────────────────────┐
  │                                                          │
  │  1. MONITOR: Which resource is saturated?                │
  │     top / iostat / free / sar                            │
  │     → CPU? Memory? Disk? Network? None (software)?       │
  │                                                          │
  │  2. PROFILE: Why is that resource saturated?             │
  │     perf record / perf stat / strace                     │
  │     → Which functions? Which events?                     │
  │                                                          │
  │  3. ANALYZE: What is the root cause?                     │
  │     perf report / flame graphs                           │
  │     → Inefficient algorithm? Too many cache misses?      │
  │       Too many syscalls? Lock contention?                │
  │                                                          │
  │  4. OPTIMIZE: Fix the root cause.                        │
  │     Rewrite code / change data structure / tune params   │
  │                                                          │
  │  5. MEASURE AGAIN: Did it help?                          │
  │     Re-run monitoring + profiling                        │
  │     → Bottleneck may shift to a different component!     │
  │                                                          │
  │  6. REPEAT until satisfied with performance.             │
  │                                                          │
  └──────────────────────────────────────────────────────────┘
```

---

## Summary

| Concept | Key Idea | Linux Tool |
|---------|----------|-----------|
| Capacity | Max throughput; limited by bottleneck resource | - |
| Hardware bottleneck | A physical resource is 100% utilized | `top`, `iostat`, `free`, `sar` |
| Software bottleneck | No resource saturated, but throughput plateaus | Check fd limits, thread counts, queue sizes |
| CPU monitoring | %user, %system, %idle, %iowait per core | `top`, `mpstat`, `pidstat` |
| Memory monitoring | Used/free/available, swap in/out | `free`, `vmstat`, `/proc/meminfo` |
| Disk monitoring | %utilization, IOPS, await time | `iostat`, `iotop` |
| Network monitoring | Throughput, drops, errors per interface | `sar -n DEV`, `ethtool -S` |
| Profiling | Find which code/events cause the bottleneck | `perf record`, `perf stat`, `perf report` |
| Flame graphs | Visualize CPU time across call stacks | `perf script` + flamegraph.pl |
| Hardware counters | CPU cycles, cache misses, branch mispredictions | `perf stat -e` |
