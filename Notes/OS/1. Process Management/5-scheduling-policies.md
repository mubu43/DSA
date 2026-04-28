# Scheduling Policies

---

## 1. The OS Scheduler

The **CPU scheduler** is the kernel component that decides which process (or thread) runs on which CPU core at any given time. It is one of the most performance-critical parts of the OS — it runs thousands of times per second and its decisions directly affect system responsiveness, throughput, and fairness.

### Scheduling Decisions Happen At:

1. **Process blocks** (RUNNING → BLOCKED): e.g., calls `read()`, `wait()`, `sleep()`. The CPU is free; pick someone else.
2. **Process is preempted** (RUNNING → READY): timer interrupt fires, scheduler decides the current process has run long enough.
3. **Process wakes up** (BLOCKED → READY): e.g., I/O completes. The newly ready process might deserve the CPU more than the currently running one.
4. **Process exits** (RUNNING → TERMINATED): CPU is free; pick someone else.
5. **New process created** (NEW → READY): Another candidate for the CPU.

---

## 2. Preemptive vs. Non-Preemptive Scheduling

### Non-Preemptive (Cooperative) Scheduling

The running process keeps the CPU until it **voluntarily** gives it up — by making a blocking syscall or explicitly yielding. The OS cannot forcibly take the CPU away.

- **Problem**: A CPU-bound process (infinite loop, heavy computation) monopolizes the CPU. Other processes starve. A buggy process can hang the entire system.
- **Used in**: Early Mac OS (pre-X), Windows 3.x, some embedded RTOS.
- **Not suitable for general-purpose OSes**.

### Preemptive Scheduling

The OS can **forcibly** take the CPU from a running process at any time (via timer interrupt) and give it to another. This is the standard for all modern general-purpose OSes.

- **Mechanism**: A hardware **timer interrupt** fires periodically. The interrupt handler checks if the current process should be preempted. If so, it invokes the scheduler.
- **Advantage**: No process can monopolize the CPU. Ensures responsiveness and fairness.
- **Downside**: Added complexity (context switch overhead, need for synchronization of shared kernel data structures).

```
  Non-Preemptive:                    Preemptive:
  
  P1 runs until it blocks           P1 runs, timer fires, OS preempts
  ┌──────────────────────┐           ┌────────┐ ┌────────┐ ┌────────┐
  │       P1 runs        │           │P1 runs │ │P2 runs │ │P1 runs │
  │     (monopolizes)    │           │        │ │        │ │        │
  └──────────────────────┘           └────────┘ └────────┘ └────────┘
  If P1 is CPU-bound,                    ↑          ↑          ↑
  P2 never runs!                     timer int  timer int  timer int
```

---

## 3. Timer Interrupts

The **timer interrupt** (also called the **scheduler tick**) is the heartbeat that enables preemptive scheduling. A hardware timer (APIC timer, HPET, PIT) is programmed to fire an interrupt at a regular interval.

- When the timer interrupt fires, the CPU traps into the kernel.
- The timer interrupt handler updates accounting (runtime statistics) and checks if the current process needs to be preempted.
- If preemption is needed, the `TIF_NEED_RESCHED` flag is set, and `schedule()` is called on the return path from the interrupt.

### Linux Timer Configuration

- **`CONFIG_HZ`**: The tick rate. Common values:
  - `100` HZ — 10 ms tick (servers, lower overhead)
  - `250` HZ — 4 ms tick (default on many distros, good balance)
  - `1000` HZ — 1 ms tick (desktop/interactive, more responsive but more overhead)
  
- **Tickless (NO_HZ) modes**:
  - `CONFIG_NO_HZ_IDLE`: Stop ticks when CPU is idle (saves power). Ticks resume when the CPU has work.
  - `CONFIG_NO_HZ_FULL`: Stop ticks even when a single task is running (useful for latency-sensitive workloads that don't want to be interrupted). Only generate ticks when there are multiple runnable tasks.

```bash
# Check current HZ setting:
$ grep CONFIG_HZ /boot/config-$(uname -r)

# Check tickless mode:
$ grep NO_HZ /boot/config-$(uname -r)

# Check timer interrupt frequency at runtime:
$ cat /proc/interrupts | grep -i timer
```

---

## 4. Scheduling Policy Goals

Different workloads have different needs. Scheduling policies try to optimize for one or more of these (often competing) goals:

### 4.1 Maximize CPU Utilization

Keep the CPU busy as much as possible. Idle CPU = wasted resource.
- Utilization = (time CPU is busy) / (total time)
- Ideal: 100% utilization. Achieved by always having a READY process to run when the current one blocks.

### 4.2 Minimize Turnaround Time

**Turnaround time** = (time of completion) − (time of arrival)

This is the total time from when a job is submitted until it finishes, including any waiting time. Important for **batch workloads** (compilation, data processing).

### 4.3 Minimize Response Time

**Response time** = (time of first run) − (time of arrival)

How quickly a process gets its first time slice after becoming READY. Critical for **interactive workloads** (GUI, terminals, games) — users perceive sluggishness if response time is high.

### 4.4 Fairness

Every process should get a "fair" share of the CPU. No process should starve (wait indefinitely while others run). But "fair" is nuanced:
- **Equal time**: Every process gets the same CPU time.
- **Proportional**: Processes with higher priority/weight get more CPU time.
- **Max-min fairness**: Maximize the minimum allocation.

### 4.5 Low Scheduling Overhead

The scheduler itself consumes CPU time. If the scheduler is too complex (e.g., O(n) scan of all processes), it becomes a bottleneck on systems with many processes.

### The Fundamental Tension

These goals conflict:

```
  Turnaround Time ◄──── TENSION ────► Response Time
  
  • Short jobs first (SJF) minimizes    • Round-robin with small time
    turnaround time, but long jobs        slices minimizes response time,
    may starve → unfair.                  but context switch overhead
                                          hurts turnaround time.
  
  Throughput ◄──── TENSION ────► Responsiveness
  
  • Long time slices reduce context     • Short time slices improve
    switch overhead → better throughput    responsiveness but increase
                                          context switch overhead.
```

---

## 5. Scheduling Algorithms

To build intuition, we'll analyze each policy with a consistent example:

**Setup**: Four jobs arrive at a single-CPU system.

| Job | Arrival Time | Run Time (burst) |
|-----|-------------|-------------------|
| A | 0 | 8 |
| B | 1 | 4 |
| C | 2 | 9 |
| D | 3 | 5 |

We'll compute **turnaround time** and **response time** for each.

### 5.1 FIFO (First In, First Out) / FCFS (First Come, First Served)

**Rule**: Run jobs in the order they arrive. Each job runs to completion (non-preemptive).

```
  Time: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  CPU:  ├──────────A──────────┤├────────B────────┤├──────────────C──────────────┤├──────────D──────────┤
        │     A (8 units)     │  B (4 units)      │     C (9 units)             │  D (5 units)        │
        0                     8                  12                            21                    26
```

| Job | Arrival | Completion | Turnaround (Compl - Arrival) | Response (First Run - Arrival) |
|-----|---------|------------|------------------------------|-------------------------------|
| A | 0 | 8 | 8 | 0 |
| B | 1 | 12 | 11 | 7 |
| C | 2 | 21 | 19 | 6 |
| D | 3 | 26 | 23 | 18 |
| **Avg** | | | **15.25** | **7.75** |

**Pros**:
- Simple to implement — just a FIFO queue.
- Low overhead — no preemption, minimal scheduling decisions.

**Cons**:
- **Convoy effect**: Short jobs stuck behind long jobs. If A takes 100 time units, B, C, D all wait even if they're tiny.
- Poor response time for interactive workloads.
- Non-preemptive — one CPU-bound job blocks everything.

### 5.2 SJF (Shortest Job First)

**Rule**: Run the job with the shortest total run time first. Non-preemptive — once a job starts, it runs to completion.

Since SJF is non-preemptive, at time 0 only A has arrived, so A starts. When A finishes at t=8, we pick the shortest among B(4), C(9), D(5) → B.

```
  Time: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  CPU:  ├──────────A──────────┤├──B──B──B──B┤├──D──D──D──D──D┤├──C──C──C──C──C──C──C──C──C┤
        0                     8            12                17                           26
```

| Job | Arrival | Completion | Turnaround | Response |
|-----|---------|------------|------------|----------|
| A | 0 | 8 | 8 | 0 |
| B | 1 | 12 | 11 | 7 |
| D | 3 | 17 | 14 | 14 |
| C | 2 | 26 | 24 | 15 |
| **Avg** | | | **14.25** | **9.0** |

**Pros**:
- Provably **optimal** for minimizing average turnaround time (among non-preemptive policies), when all jobs arrive at the same time.

**Cons**:
- **Starvation**: Long jobs may never run if short jobs keep arriving.
- Non-preemptive — still has convoy effect if a long job arrives first and starts before short jobs arrive.
- **Requires knowing job lengths in advance** — often impossible in practice.

### 5.3 SRTF (Shortest Remaining Time First) / STCF (Shortest Time to Completion First)

**Rule**: Preemptive version of SJF. At every scheduling point (new arrival, job completion), run the job with the **shortest remaining** run time. If a new job arrives with a shorter remaining time than the current one, preempt.

```
  Time: 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  
  t=0: Only A arrived (remaining=8). Run A.
  t=1: B arrives (remaining=4). A has remaining=7. B < A → preempt A, run B.
  t=2: C arrives (remaining=9). B has remaining=3. B < C → keep B.
  t=3: D arrives (remaining=5). B has remaining=2. B < D → keep B.
  t=5: B finishes. Remaining: A=7, C=9, D=5 → run D.
  t=10: D finishes. Remaining: A=7, C=9 → run A.
  t=17: A finishes. Only C left → run C.
  t=26: C finishes.

  CPU:  ├A┤├──B──B──B──B┤├────D────D────D────D────D┤├──────A──────A──────A──────A──────A──────A──────A┤├──────────C─────────┤
        0  1            5                          10                                                17                   26

  More precisely:
  t=0-1:  A runs (1 unit)
  t=1-5:  B runs (4 units, completes)
  t=5-10: D runs (5 units, completes)
  t=10-17: A runs (7 remaining units, completes)
  t=17-26: C runs (9 units, completes)
```

| Job | Arrival | Completion | Turnaround | Response |
|-----|---------|------------|------------|----------|
| A | 0 | 17 | 17 | 0 |
| B | 1 | 5 | 4 | 0 |
| C | 2 | 26 | 24 | 15 |
| D | 3 | 10 | 7 | 2 |
| **Avg** | | | **13.0** | **4.25** |

**Pros**:
- **Optimal** for minimizing average turnaround time (provably, among all policies).
- Short jobs get through quickly.

**Cons**:
- **Starvation**: Long jobs can starve indefinitely if short jobs keep arriving.
- Requires knowing remaining execution time — usually unknown.
- **Poor response time for long jobs** — they keep getting preempted.

### 5.4 RR (Round Robin)

**Rule**: Each job gets a fixed **time quantum** (time slice). Jobs run in FIFO order, but each runs for at most one quantum before being preempted and placed at the end of the ready queue.

Let's use a time quantum of **2 units**:

```
  Time:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26
  
  Queue at t=0: [A]
  t=0-2:  A runs (remaining=6). At t=1, B arrives. At t=2, A preempted. Queue: [B,C,A] (C arrived at t=2)
  t=2-4:  B runs (remaining=2). At t=3, D arrives. Queue: [C,A,D,B]
  t=4-6:  C runs (remaining=7). Queue: [A,D,B,C]
  t=6-8:  A runs (remaining=4). Queue: [D,B,C,A]
  t=8-10: D runs (remaining=3). Queue: [B,C,A,D]
  t=10-12: B runs (remaining=0, completes!). Queue: [C,A,D]
  t=12-14: C runs (remaining=5). Queue: [A,D,C]
  t=14-16: A runs (remaining=2). Queue: [D,C,A]
  t=16-18: D runs (remaining=1). Queue: [C,A,D]
  t=18-20: C runs (remaining=3). Queue: [A,D,C]
  t=20-22: A runs (remaining=0, completes!). Queue: [D,C]
  t=22-23: D runs (remaining=0, completes!). Queue: [C]
  t=23-26: C runs (remaining=0, completes!).

  CPU:  ├A─A┤├B─B┤├C─C┤├A─A┤├D─D┤├B─B┤├C─C┤├A─A┤├D─D┤├C─C┤├A─A┤├D┤├C─C─C┤
        0   2    4    6    8   10   12   14   16   18   20  22 23   26
```

| Job | Arrival | Completion | Turnaround | Response |
|-----|---------|------------|------------|----------|
| A | 0 | 22 | 22 | 0 |
| B | 1 | 12 | 11 | 1 |
| C | 2 | 26 | 24 | 2 |
| D | 3 | 23 | 20 | 5 |
| **Avg** | | | **19.25** | **2.0** |

**Pros**:
- **Excellent response time** — every job gets a CPU slice quickly.
- **Fair** — all processes get equal CPU time in proportion.
- **No starvation** — every process runs within at most (N-1) × quantum time.

**Cons**:
- **Poor turnaround time** — jobs are stretched out over time. Compare RR's avg turnaround (19.25) with SRTF's (13.0).
- **Context switch overhead** — if the quantum is too small, the CPU spends a significant fraction of time switching instead of doing useful work.
- All jobs take roughly the same time to complete (they all finish near the end).

### Time Quantum Trade-off

```
  Small quantum (e.g., 1ms)          Large quantum (e.g., 100ms)
  ─────────────────────────          ──────────────────────────
  + Better response time              + Less context switch overhead
  + More "fair" feeling               + Better turnaround time
  - High context switch overhead      - Worse response time
  - Throughput drops                  - Approaches FCFS behavior
  
  Typical practical quantum: 1-10 ms
```

### 5.5 WFQ (Weighted Fair Queuing) / Weighted Round Robin

**Rule**: Each process has a **weight** (proportional to its priority). The CPU time is divided in proportion to the weights. A process with weight 2 gets twice as much CPU time as a process with weight 1.

```
  Example: P1 weight=2, P2 weight=1, P3 weight=1
  Total weight = 4
  
  P1 gets 2/4 = 50% of CPU
  P2 gets 1/4 = 25% of CPU
  P3 gets 1/4 = 25% of CPU
  
  With quantum=10ms:
  ┌──P1──P1──┐┌──P2──┐┌──P3──┐┌──P1──P1──┐┌──P2──┐ ...
  │  20ms    ││ 10ms ││ 10ms ││  20ms    ││ 10ms │
  └──────────┘└──────┘└──────┘└──────────┘└──────┘
```

**This is the foundation of Linux CFS** (Completely Fair Scheduler), which we'll discuss in the MLFQ section.

### 5.6 MLFQ (Multi-Level Feedback Queue)

MLFQ is the most practical and widely-used scheduling approach. It tries to achieve the best of all worlds without requiring advance knowledge of job lengths.

**Key idea**: Use **past behavior** to predict future behavior.
- A process that has been CPU-bound will likely continue to be CPU-bound.
- A process that has been doing I/O will likely continue to do I/O.

#### Structure

MLFQ has multiple priority queues. Each queue has a different priority level and potentially a different time quantum.

```
  ┌─────────────────────────────────────────────┐
  │  Priority Queue 7 (Highest)                  │  ← Short quantum (e.g., 8ms)
  │  [interactive/I/O-bound processes]           │     Run these first
  ├─────────────────────────────────────────────┤
  │  Priority Queue 6                            │  ← 16ms quantum
  │                                             │
  ├─────────────────────────────────────────────┤
  │  Priority Queue 5                            │  ← 32ms quantum
  │                                             │
  ├─────────────────────────────────────────────┤
  │  ...                                         │
  ├─────────────────────────────────────────────┤
  │  Priority Queue 1                            │  ← 256ms quantum
  │                                             │
  ├─────────────────────────────────────────────┤
  │  Priority Queue 0 (Lowest)                   │  ← Longest quantum / FCFS
  │  [long-running CPU-bound processes]          │     Run only when higher
  │                                             │     queues are empty
  └─────────────────────────────────────────────┘
```

#### MLFQ Rules

1. **If Priority(A) > Priority(B)**: A runs (B doesn't).
2. **If Priority(A) == Priority(B)**: A and B run in Round Robin within that queue.
3. **New jobs start at the highest priority queue** (optimistic assumption: it might be short/interactive).
4. **If a job uses up its entire time quantum**: Its priority is reduced (moved down one queue). It's behaving CPU-bound.
5. **If a job voluntarily gives up the CPU** (e.g., I/O): It stays at the same priority. It's behaving I/O-bound/interactive.

```
  How MLFQ adapts:

  New job P arrives → placed in Queue 7 (highest priority)
  
  Case 1: P does a small burst then I/O
    → P stays in Queue 7 (short, interactive pattern)
    → Gets fast response time ✓
  
  Case 2: P uses full quantum in Queue 7
    → Demoted to Queue 6, gets longer quantum
    → Uses full quantum again → demoted to Queue 5
    → ... eventually ends up in Queue 0
    → Treated as CPU-bound, gets large time slices ✓
    → Doesn't hurt interactive processes ✓
```

#### Problems and Solutions

**Problem 1: Starvation**
If many interactive jobs keep arriving, CPU-bound jobs in low-priority queues never run.

**Solution: Priority Boost (aging)**
Periodically (e.g., every S seconds), move ALL jobs to the highest-priority queue. This prevents starvation and also handles jobs whose behavior changes (e.g., a CPU-bound job that starts doing I/O).

```
  Before boost:                After boost:
  Q7: [I1, I2, I3]            Q7: [I1, I2, I3, C1, C2]  ← everyone boosted
  Q6: []                       Q6: []
  Q5: []                       Q5: []
  ...                          ...
  Q0: [C1, C2] (starving)     Q0: []
```

**Problem 2: Gaming the scheduler**
A clever process can issue a tiny I/O operation right before its quantum expires, keeping itself at high priority while being essentially CPU-bound.

**Solution: Total CPU accounting**
Instead of resetting the quantum on each entry to a queue, track the **total** CPU time used at each priority level. Once a process has used the full allotment for a level, it's demoted regardless of how many times it voluntarily yielded.

#### MLFQ Summary

| Aspect | MLFQ Approach |
|--------|---------------|
| Short jobs | Fast turnaround (start at high priority) |
| Interactive jobs | Good response time (stay at high priority) |
| CPU-bound jobs | Get large time slices (at low priority) |
| Unknown job lengths | No need to know — behavior-based adaptation |
| Starvation prevention | Priority boost |
| Gaming prevention | Total CPU time accounting per level |

---

## 6. Multi-Core Scheduling Considerations

Modern systems have multiple CPU cores. Scheduling becomes significantly more complex:

### Per-CPU Run Queues

Each CPU core has its own run queue. This avoids a global lock on a single shared queue (which would be a bottleneck with many cores).

```
  ┌────────────┐  ┌────────────┐  ┌────────────┐  ┌────────────┐
  │   CPU 0    │  │   CPU 1    │  │   CPU 2    │  │   CPU 3    │
  │            │  │            │  │            │  │            │
  │ Run Queue: │  │ Run Queue: │  │ Run Queue: │  │ Run Queue: │
  │ [P1, P5]  │  │ [P2, P6]  │  │ [P3]       │  │ [P4, P7]  │
  │            │  │            │  │            │  │            │
  └────────────┘  └────────────┘  └────────────┘  └────────────┘
```

### Load Balancing

If one CPU is overloaded and another is idle, processes should be **migrated** (load balancing):
- **Pull migration**: Idle CPU steals work from a busy CPU's run queue.
- **Push migration**: A periodic kernel thread checks for imbalance and pushes tasks from busy to idle CPUs.

### Cache Affinity

Moving a process from one CPU to another is expensive — the new CPU's caches are cold (don't have the process's data). The scheduler tries to keep a process on the same CPU (**cache affinity**) unless load imbalance is significant.

### NUMA Awareness

On **NUMA (Non-Uniform Memory Access)** systems, each CPU socket has its own local memory. Accessing remote memory (attached to another socket) is slower (~2x latency). The scheduler prefers to run a process on a CPU near its memory allocation.

```
  NUMA Architecture:
  
  ┌──────────────────────┐     ┌──────────────────────┐
  │    NUMA Node 0        │     │    NUMA Node 1        │
  │  ┌──────┐  ┌──────┐  │     │  ┌──────┐  ┌──────┐  │
  │  │CPU 0 │  │CPU 1 │  │     │  │CPU 2 │  │CPU 3 │  │
  │  └──────┘  └──────┘  │     │  └──────┘  └──────┘  │
  │       │                │     │       │                │
  │  ┌──────────────┐     │     │  ┌──────────────┐     │
  │  │ Local Memory  │     │     │  │ Local Memory  │     │
  │  │ (fast access) │     │◄───►│  │ (fast access) │     │
  │  └──────────────┘     │ QPI │  └──────────────┘     │
  │                       │     │                       │
  └──────────────────────┘     └──────────────────────┘
                          slow interconnect
```

### Linux Multi-Core Scheduling

Linux organizes CPUs into **scheduling domains** — hierarchical groupings based on the hardware topology:

```
  Scheduling Domain Hierarchy (example):
  
  Level 3: System-wide domain (all CPUs)
    └─ Level 2: NUMA node domain
         ├─ Level 1: Physical package (socket)
         │    ├─ Level 0: SMT/HT siblings (logical cores sharing a physical core)
         │    │    ├─ CPU 0
         │    │    └─ CPU 1  (hyperthreading sibling)
         │    ├─ Level 0: SMT siblings
         │    │    ├─ CPU 2
         │    │    └─ CPU 3
         ...
```

Load balancing is attempted first within the closest domain (SMT siblings, same socket) and only escalates to cross-NUMA migration when the imbalance is severe enough to justify the cache and memory locality penalty.

Key syscalls and commands:
```bash
# Pin a process to specific CPUs
$ taskset -c 0,1 ./my_program        # run on CPUs 0 and 1
$ taskset -p 0x3 <pid>               # set affinity mask of running process

# View CPU topology
$ lscpu
$ numactl --hardware                 # NUMA topology
$ cat /proc/cpuinfo

# View scheduling domains
$ cat /proc/sys/kernel/sched_domain/cpu0/domain0/name

# Set CPU affinity from code
sched_setaffinity(pid, sizeof(cpu_set_t), &mask);
```

---

## 7. Linux CFS (Completely Fair Scheduler) — In Depth

CFS is the default scheduler for normal (non-real-time) processes in Linux (kernel 2.6.23 through 6.5). Understanding CFS is important because:
- It's been the production scheduler for ~15 years.
- Its design principles (weighted fair queuing, virtual runtime) appear in interview questions.
- EEVDF (its successor) builds on the same concepts.

### Core Idea: Virtual Runtime (vruntime)

CFS models an **ideal multitasking CPU** where N processes each get exactly 1/N of the CPU simultaneously. Since real CPUs can't do this, CFS approximates it by tracking how much CPU time each process has received and always running the process that has received the **least**.

**vruntime** = the weighted amount of CPU time a process has consumed.

```
  For equal-weight processes:
    vruntime increases at the same rate as wall-clock time.
  
  For a process with higher weight (lower nice value):
    vruntime increases SLOWER → it gets more CPU time before being preempted.
  
  For a process with lower weight (higher nice value):
    vruntime increases FASTER → it gets less CPU time.
  
  Formula:
    vruntime += (actual_runtime) × (NICE_0_LOAD / process_weight)
  
  Where process_weight depends on the nice value:
    nice 0  → weight 1024 (baseline)
    nice -20 → weight 88761 (~87x baseline)
    nice +19 → weight 15 (~0.015x baseline)
```

### Data Structure: Red-Black Tree

CFS organizes all runnable processes in a **red-black tree** (self-balancing BST), keyed by `vruntime`.

```
  Red-Black Tree (per-CPU run queue):
  
             ┌──────┐
             │ P3   │
             │vr=50 │
             └──┬───┘
           ┌────┴────┐
       ┌───┴──┐  ┌───┴──┐
       │ P1   │  │ P5   │
       │vr=30 │  │vr=70 │
       └──┬───┘  └──┬───┘
        ┌─┴─┐     ┌─┴─┐
       P0  P2   P4   P6
      vr=20 vr=40 vr=60 vr=80
  
  ← leftmost node                rightmost node →
     (lowest vruntime)           (highest vruntime)
     RUNS NEXT                   has had most CPU time
```

- **Scheduling decision**: O(1) — pick the leftmost node (cached by the kernel).
- **Insertion/removal**: O(log N).
- **CFS scheduling entity**: `struct sched_entity` (embedded in `task_struct`), contains `vruntime`, `load.weight`, and the rb-tree node.

### Time Slice in CFS

CFS doesn't use a fixed time quantum like traditional RR. Instead, it computes a **target latency** and divides it proportionally:

```
  target_latency = sysctl_sched_latency (default: 6ms for ≤8 processes)
  
  For each process:
    time_slice = target_latency × (process_weight / total_weight_of_all_runnable)
  
  Example: 3 processes with weights 1024, 1024, 1024
    Each gets: 6ms × (1024/3072) = 2ms
  
  Example: 2 processes, P1 weight=2048 (nice -5), P2 weight=1024 (nice 0)
    P1: 6ms × (2048/3072) = 4ms
    P2: 6ms × (1024/3072) = 2ms
```

Minimum granularity (`sysctl_sched_min_granularity`, default: 0.75ms) ensures no time slice is too small (avoids excessive context switches).

### CFS Parameters

```bash
# View/tune CFS parameters:
$ cat /proc/sys/kernel/sched_latency_ns           # target latency (6000000 = 6ms)
$ cat /proc/sys/kernel/sched_min_granularity_ns    # min time slice (750000 = 0.75ms)
$ cat /proc/sys/kernel/sched_wakeup_granularity_ns # wakeup preemption threshold
```

### CFS vs. MLFQ

| Aspect | CFS | MLFQ |
|--------|-----|------|
| Priority queues | Single RB tree (no queues) | Multiple priority queues |
| Time tracking | vruntime (continuous) | Discrete quantum per level |
| Fairness model | Weighted fair queuing | Heuristic (feedback-based) |
| Job length prediction | Not needed | Uses past behavior |
| Starvation | Impossible (lowest vruntime always runs) | Possible without boosting |
| Complexity | O(log N) per operation | O(1) per operation |

---

## 8. EEVDF (Earliest Eligible Virtual Deadline First)

As of Linux kernel 6.6, **EEVDF** replaces CFS as the default scheduler for `SCHED_NORMAL` tasks.

### Motivation

CFS has a problem with **latency**: a process that has been sleeping for a long time wakes up with a very low `vruntime`, which gives it a large "credit" to run for a long time. This can cause latency spikes for other processes. CFS's `sched_wakeup_granularity` tunable tried to address this but was a blunt instrument.

### How EEVDF Works

EEVDF extends the virtual runtime concept with a **virtual deadline**:

```
  For each process:
    - vruntime: same as CFS (tracks how much CPU time consumed)
    - virtual deadline: vruntime + (time_slice / weight_factor)
    
  Scheduling rule:
    Among all ELIGIBLE processes (vruntime ≤ ideal fair share),
    pick the one with the EARLIEST VIRTUAL DEADLINE.
```

This provides:
- **Fairness**: Same as CFS (based on vruntime).
- **Bounded latency**: The virtual deadline ensures that no process has to wait too long for its next slice, regardless of its sleep history.
- **Simpler tuning**: Removes the need for `sched_wakeup_granularity` and the sleeper bonus heuristics.

### EEVDF vs. CFS

| Aspect | CFS | EEVDF |
|--------|-----|-------|
| Data structure | Red-black tree (vruntime) | Red-black tree (vruntime + deadline) |
| Pick next task | Leftmost in RB tree | Earliest eligible deadline |
| Sleeper fairness | Heuristic (wakeup granularity) | Structural (eligibility + deadline) |
| Latency bounds | Approximate | Formally bounded |
| Tunables | Many (`sched_latency_ns`, etc.) | Fewer needed |

---

## 9. Comparison Table: All Policies

| Policy | Type | Optimal For | Starvation? | Knows Job Length? | Used In Practice? |
|--------|------|-------------|-------------|-------------------|-------------------|
| **FIFO/FCFS** | Non-preemptive | Simple batch | No (but convoy) | No | Rarely alone |
| **SJF** | Non-preemptive | Avg. turnaround | Yes (long jobs) | **Yes** | Impractical (theory) |
| **SRTF/STCF** | Preemptive | Avg. turnaround | Yes (long jobs) | **Yes** | Impractical (theory) |
| **RR** | Preemptive | Response time | No | No | Building block |
| **MLFQ** | Preemptive | General purpose | With boost: No | No (adapts) | Windows, macOS, older Linux |
| **CFS** | Preemptive | Fairness | No | No | Linux 2.6.23–6.5 |
| **EEVDF** | Preemptive | Fairness + latency | No | No | Linux 6.6+ |

---

## Summary

| Concept | Key Takeaway |
|---------|-------------|
| Preemptive scheduling | OS can forcibly take CPU from a process (via timer interrupt). Essential for general-purpose OS. |
| Turnaround time vs. response time | Fundamental trade-off. SJF/SRTF optimize turnaround; RR optimizes response. |
| MLFQ | Best practical approach: multiple priority queues, adapt based on behavior, boost to prevent starvation. |
| CFS / EEVDF | Linux's approach: weighted fair queuing using virtual runtime, red-black tree, O(log N). EEVDF adds deadline-based latency guarantees. |
| Multi-core scheduling | Per-CPU run queues, load balancing, cache affinity, NUMA awareness. |
| Timer interrupt | The heartbeat of preemptive scheduling. Configurable via CONFIG_HZ; tickless modes reduce overhead. |
