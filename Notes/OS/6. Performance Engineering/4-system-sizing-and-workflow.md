# System Sizing, Queuing Theory, and the Performance Workflow

---

## 1. Software Bottlenecks

When throughput plateaus but **no hardware resource is at 100%**, the bottleneck is in software. Common causes:

| Software Limit | Symptom | Diagnosis | Fix |
|---------------|---------|-----------|-----|
| File descriptor limit | `Too many open files` errors; accept() fails | `ulimit -n`, `cat /proc/sys/fs/file-max` | Increase `ulimit -n` or `fs.file-max` |
| Thread pool too small | CPU idle, but requests queueing | Thread pool threads all busy; CPU < 50% | Increase thread pool size (see formula below) |
| Queue/buffer too small | Requests dropped under burst | Buffer fills, producers blocked or items dropped | Size buffers with Little's Law |
| Connection pool exhausted | Threads waiting for DB connection | All connections in use; DB not saturated | Increase pool size or reduce hold time |
| Ephemeral port exhaustion | `Cannot assign requested address` | `ss -s` shows many TIME_WAIT | `SO_REUSEADDR`, tune `tcp_tw_reuse`, connection pooling |
| Lock contention | CPU idle, low throughput, high lock wait time | `perf lock`, `mutrace`, flame graph shows futex | Fine-grained locks, lock-free structures, reduce critical section |

```bash
# Check file descriptor limit:
$ ulimit -n          # per-process soft limit
1024
$ cat /proc/sys/fs/file-max    # system-wide limit
1048576

# Increase per-process limit:
$ ulimit -n 65536    # for current shell

# Persistent: /etc/security/limits.conf
# *  soft  nofile  65536
# *  hard  nofile  65536
```

---

## 2. Optimum Thread Pool Sizing

### The Problem

A thread pool handles incoming requests. If the pool is too small, requests queue up and CPU cores sit idle. If too large, excessive context switching and memory waste.

### The Key Insight: Service Demand vs. Turnaround Time

For each request, there are two times:
- **Service demand ($S$)**: The actual CPU time the request needs.
- **Turnaround time ($T$)**: The total wall-clock time from start to finish (includes CPU time + I/O wait time + any other waiting).

During I/O wait, the thread is blocked and its CPU core is free. Another thread can use it.

```
  Timeline of a Request (T = 1.01s, S = 0.01s):

  Thread: ──CPU──────WAIT (I/O: 1s)──────CPU──►
          0.005s          1.0s           0.005s

  Total turnaround T = 1.01s
  Service demand   S = 0.01s (CPU only)

  During the 1s I/O wait, this thread's core is FREE.
  Another thread could use it!
```

### The Formula

$$
\text{Threads} = \frac{T}{S} = \frac{\text{Turnaround time}}{\text{Service demand}}
$$

This gives the number of threads needed to keep **one core** fully utilized. For $C$ cores:

$$
\text{Threads}_{\text{total}} = C \times \frac{T}{S}
$$

### Example

A web server handles requests that:
- Take 10 ms of CPU time ($S = 0.01$s).
- Make a database call that takes 1 second ($T = 1.01$s total).
- Machine has 4 cores.

$$
\text{Threads per core} = \frac{1.01}{0.01} = 101
$$

$$
\text{Total threads} = 4 \times 101 = 404
$$

Need ~404 threads to keep all 4 cores busy. With fewer threads, cores would be idle during I/O waits.

```
  Why Many Threads Are Needed (I/O-heavy):

  Time ──────────────────────────────────────────►

  Thread 1:  [CPU]━━━━━━━━━━━━━━ I/O wait ━━━━━━━━━━━━━━[CPU]
  Thread 2:  [CPU]━━━━━━━━━━━━━━ I/O wait ━━━━━━━━━━━━━━[CPU]
  Thread 3:       [CPU]━━━━━━━━━━━━ I/O wait ━━━━━━━━━━━━━━
  Thread 4:            [CPU]━━━━━━━━━━━━ I/O wait ━━━━━━━━━━
  ...
  Thread 101:                      [CPU]━━━━━━━━━━━━ I/O wait

  Core 0:    [T1][T2][T3][T4]...[T101][T1][T2]...
             Core always busy! Always a ready thread when one blocks.

  If only 4 threads: core idle 99% of the time (each thread
  uses CPU for 1% of its turnaround time).
```

**For CPU-bound workloads** ($T \approx S$): Threads ≈ number of cores. Adding more threads just adds context-switch overhead.

**For I/O-bound workloads** ($T \gg S$): Many more threads than cores needed to keep cores busy.

---

## 3. Buffer and Queue Sizing with Little's Law

### Little's Law

A fundamental result in queuing theory:

$$
N = R \times W
$$

Where:
- $N$ = average number of items in the system (queue + being processed).
- $R$ = arrival rate (items per second).
- $W$ = average time an item spends in the system (wait time + processing time).

This is universal — it applies to any stable queuing system regardless of arrival distribution or service time distribution.

### Applying Little's Law to Buffer Sizing

**Question**: How large should a request buffer be to avoid dropping requests during normal operation?

```
  ┌──────────┐     ┌─────────────────────┐     ┌──────────┐
  │ Requests │ ──► │   Buffer (size N)    │ ──► │ Workers  │
  │ R req/s  │     │                     │     │          │
  └──────────┘     └─────────────────────┘     └──────────┘

  If the buffer is too small, requests are DROPPED when it's full.
  Little's Law: N = R × W
```

### Example 1: Web Server Request Buffer

- Arrival rate: $R = 100$ requests/second.
- Average processing time: $W = 2$ seconds (includes queue wait).
- Buffer size needed: $N = 100 \times 2 = 200$ slots.

### Example 2: Message Queue

- Producer rate: $R = 10{,}000$ messages/second.
- Consumer processing time: Average $W = 0.5$ seconds in queue.
- Queue size: $N = 10{,}000 \times 0.5 = 5{,}000$ messages.

### Example 3: Network Interface Buffer

- Packet arrival rate: $R = 1{,}000{,}000$ packets/second.
- Processing time per packet: $W = 5\,\mu s = 0.000005$s.
- Ring buffer: $N = 1{,}000{,}000 \times 0.000005 = 5$ packets minimum.
- In practice, add headroom for bursts: use 256 or 512 entries.

```
  Little's Law — Intuition:

  If customers arrive at a restaurant at 60/hour,
  and each stays for 0.5 hours (30 min):

  N = 60 × 0.5 = 30 seats needed on average.

  Double the processing time (1 hour per customer):
  N = 60 × 1 = 60 seats needed.

  Double the arrival rate (120/hour):
  N = 120 × 0.5 = 60 seats needed.
```

---

## 4. Throughput vs. Response Time

### Definitions

- **Throughput ($X$)**: Number of requests completed per unit time (req/s).
- **Response time ($R$)**: Time from request submission to completion (includes queuing + service).
- **Service demand ($S$)**: Time a request actually uses a resource (CPU time, disk time, etc.).

### The Fundamental Relationship

For a system with a **bottleneck resource** at utilization $U$:

$$
R = S + \text{queueing delay}
$$

At low load, queueing delay ≈ 0, so $R \approx S$. As load increases toward the bottleneck capacity, queueing delay grows — eventually exponentially.

```
  Throughput vs. Response Time:

  Response
  Time (R)
  ^
  │                                ╱
  │                             ╱
  │                          ╱
  │                       ╱
  │                   ╱╱
  │              ╱╱╱
  │        ╱╱╱╱
  │   ╱╱╱╱
  │╱╱╱
  ├──────────────────────────────────► Load (req/s)
  │                        │
  │       ◄── linear ──►   │◄── hockey stick ──►
  │       response time    │  queueing delay
  │       ≈ service demand │  dominates
  │                        │
  │                    CAPACITY
  │                    (bottleneck
  │                     resource at
  │                     100%)
```

### Where Optimization Helps

| Scenario | Effect of Optimization |
|----------|----------------------|
| Optimize the **bottleneck** resource | Increases maximum throughput (capacity). Also improves response time. |
| Optimize a **non-bottleneck** resource | Does NOT increase throughput (bottleneck is elsewhere). DOES improve response time (less time at that resource). |

```
  Example: 3-stage pipeline

  ┌─────────┐     ┌─────────┐     ┌─────────┐
  │ Stage A │ ──► │ Stage B │ ──► │ Stage C │
  │  2 ms   │     │  8 ms   │     │  3 ms   │
  └─────────┘     └─────────┘     └─────────┘
                   ^^^^^^^^
                   BOTTLENECK (slowest stage)

  Max throughput = 1 / 8ms = 125 req/s (limited by Stage B)
  Response time  = 2 + 8 + 3 = 13 ms (at low load)

  Optimize Stage A from 2ms to 1ms:
  - Throughput: still 125 req/s (bottleneck unchanged)
  - Response time: 1 + 8 + 3 = 12 ms (improved by 1ms)

  Optimize Stage B from 8ms to 4ms:
  - Throughput: 1 / 4ms = 250 req/s (doubled!)
  - Response time: 2 + 4 + 3 = 9 ms (improved by 4ms)
  - But now Stage C may become the new bottleneck...
```

---

## 5. The Performance Engineering Workflow

Performance engineering is an **iterative cycle**. You never "finish" — you optimize until the system meets its requirements.

```
  ┌─────────────────────────────────────────────────┐
  │                                                 │
  │  ┌──────────┐                                   │
  │  │  BUILD   │  Implement the system / feature   │
  │  └────┬─────┘                                   │
  │       │                                         │
  │       ▼                                         │
  │  ┌──────────┐                                   │
  │  │LOAD TEST │  Apply realistic workload         │
  │  │          │  (wrk, hey, k6, JMeter, locust)   │
  │  └────┬─────┘                                   │
  │       │                                         │
  │       ▼                                         │
  │  ┌──────────┐                                   │
  │  │ IDENTIFY │  Monitor resources (top, iostat)  │
  │  │BOTTLENECK│  Profile code (perf, flame graph) │
  │  └────┬─────┘                                   │
  │       │                                         │
  │       ▼                                         │
  │  ┌──────────┐                                   │
  │  │ OPTIMIZE │  Fix the bottleneck:              │
  │  │          │  - Better algorithm               │
  │  │          │  - Tune parameters                │
  │  │          │  - Cache results                  │
  │  │          │  - Reduce I/O or syscalls          │
  │  └────┬─────┘                                   │
  │       │                                         │
  │       ▼                                         │
  │  ┌──────────┐                                   │
  │  │ MEASURE  │  Did it actually improve?          │
  │  │  AGAIN   │  By how much?                     │
  │  └────┬─────┘                                   │
  │       │                                         │
  │       ▼                                         │
  │  ┌──────────┐     ┌─────┐                       │
  │  │  MEETS   │────►│DONE │                       │
  │  │  TARGET? │ Yes └─────┘                       │
  │  └────┬─────┘                                   │
  │       │ No                                      │
  │       └──── Bottleneck may have SHIFTED ────────┤
  │             to a different component.            │
  │             Go back to IDENTIFY.                │
  └─────────────────────────────────────────────────┘
```

### Step 1: Load Testing

Generate realistic workload and measure throughput + response time under various concurrency levels.

```bash
# wrk: HTTP benchmarking tool
$ wrk -t12 -c400 -d30s http://localhost:8080/api
# 12 threads, 400 connections, 30 seconds
# Output: Requests/sec, Avg/Max latency, transfer/sec

# hey: simpler alternative
$ hey -n 10000 -c 100 http://localhost:8080/api
# 10000 requests, 100 concurrent

# Gradually increase concurrency to find the saturation point:
$ for c in 1 10 50 100 200 500 1000; do
    echo "=== Concurrency: $c ==="
    hey -n 5000 -c $c http://localhost:8080/api 2>&1 | grep -E 'Requests|Average|Fastest|Slowest'
  done
```

### Step 2: Identify the Bottleneck

While load testing, monitor all resources simultaneously:

```bash
# Terminal 1: CPU
$ mpstat -P ALL 1

# Terminal 2: Memory
$ vmstat 1

# Terminal 3: Disk
$ iostat -xz 1

# Terminal 4: Network
$ sar -n DEV 1

# Terminal 5: Application-level
$ perf top          # which functions are hot?
$ strace -c -p PID  # which syscalls dominate?
```

### Step 3: Optimize

Based on what the bottleneck is:

| Bottleneck | Optimization Strategies |
|-----------|------------------------|
| **CPU** | Better algorithms, compiler optimization, SIMD, reduce syscalls, cache-friendly data structures |
| **Memory** | Reduce footprint, huge pages, pre-allocate, avoid fragmentation |
| **Disk I/O** | Caching (Redis, memcached), async I/O, batch writes, faster storage (SSD → NVMe) |
| **Network** | Connection pooling, compression, batching, kernel bypass |
| **Software** | Tune thread pools, buffer sizes, connection pools, fd limits |
| **Lock contention** | Fine-grained locks, lock-free structures, reduce critical section size |

### Step 4: Scale

When single-machine optimization is exhausted:

```
  Vertical Scaling:                  Horizontal Scaling:
  ┌──────────────────┐              ┌──────────┐
  │  Bigger Machine  │              │  Load    │
  │  More cores      │              │ Balancer │
  │  More RAM        │              └────┬─────┘
  │  Faster disk     │                ╱  │  ╲
  │  Faster NIC      │              ╱   │   ╲
  └──────────────────┘          ┌──┴─┐┌─┴──┐┌┴───┐
                                │ M1 ││ M2 ││ M3 │
  Pros: Simple, no code         │    ││    ││    │
        changes needed.         └────┘└────┘└────┘
  Cons: Expensive,
        diminishing returns,    Pros: Linear scaling,
        single point of               fault tolerance.
        failure.                Cons: Complexity (state,
                                      consistency, networking).
```

### Caching: The Cross-Cutting Optimization

Caching reduces load on the bottleneck by serving repeated requests from fast storage.

```
  Caching Hierarchy:

  ┌─────────────────────────────────────────┐
  │ Client-side cache (browser, CDN)        │  ◄── fastest, farthest
  ├─────────────────────────────────────────┤
  │ Reverse proxy cache (Nginx, Varnish)    │
  ├─────────────────────────────────────────┤
  │ Application cache (in-process HashMap)  │
  ├─────────────────────────────────────────┤
  │ Distributed cache (Redis, Memcached)    │
  ├─────────────────────────────────────────┤
  │ Database query cache                    │
  ├─────────────────────────────────────────┤
  │ OS page cache (kernel)                  │  ◄── always present
  └─────────────────────────────────────────┘

  Each layer avoids going to the slower layer below.
  The best request is one that never reaches the server.
```

---

## Summary

| Concept | Key Idea | Formula/Tool |
|---------|----------|-------------|
| Software bottleneck | Throughput limited by config/design, not hardware | Check fd limits, thread counts, queue sizes |
| Thread pool sizing | Threads = Turnaround / Service demand (per core) | $\text{Threads} = C \times T/S$ |
| CPU-bound sizing | Threads ≈ number of cores | $T \approx S$ |
| I/O-bound sizing | Threads ≫ cores (threads idle during I/O) | $T \gg S$ |
| Little's Law | Items in system = arrival rate × time in system | $N = R \times W$ |
| Buffer sizing | Size buffers to hold in-flight items | $N = R \times W$ + headroom |
| Throughput | Limited by bottleneck resource | Optimize bottleneck to increase |
| Response time | Service demand + queueing delay | Optimize any resource to improve |
| Bottleneck shift | Fixing one bottleneck reveals the next | Re-measure after every optimization |
| Workflow | Build → Load test → Identify → Optimize → Measure → Repeat | `wrk`/`hey`, `top`/`perf`, iterate |
| Vertical scaling | Bigger machine (more cores, RAM, faster disk) | Simple but limited |
| Horizontal scaling | More machines behind a load balancer | Complex but linearly scalable |
| Caching | Serve repeated requests from fast storage | CDN, Redis, app cache, page cache |
