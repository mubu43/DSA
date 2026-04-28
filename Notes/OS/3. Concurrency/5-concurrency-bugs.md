# Concurrency Bugs

---

## 1. The Nature of Concurrency Bugs

Concurrent programs are among the hardest to debug because their bugs are **non-deterministic**:

- The same code may produce correct results 999 times and fail on the 1000th run.
- Bugs depend on **thread scheduling order**, which varies with CPU load, timing, number of cores, and OS decisions.
- Bugs may disappear when you add `printf` statements (timing changes mask the race).
- Bugs may only manifest under heavy load or on specific hardware.

### Two Categories of Concurrency Bugs

```
  Concurrency Bugs
  ├── Non-Deadlock Bugs
  │   ├── Atomicity Violations
  │   └── Order Violations
  └── Deadlock Bugs
      └── Circular wait, hold-and-wait, etc.
```

A landmark study (Lu et al., "Learning from Mistakes — A Comprehensive Study on Real World Concurrency Bug Characteristics", ASPLOS 2008) examined bugs in MySQL, Apache, Mozilla, and OpenOffice:
- **~97% of non-deadlock bugs** were atomicity violations or order violations.
- **~30% of all concurrency bugs** were deadlocks.

---

## 2. Atomicity Violation Bugs

An **atomicity violation** occurs when a code region that the programmer intended to be atomic (execute without interruption) is in fact interleaved with operations from another thread.

### Example

```c
// Thread 1:                         // Thread 2:
if (thd->proc_info) {                thd->proc_info = NULL;
    fputs(thd->proc_info, ...);
}
```

The programmer assumes: "if `proc_info` is not NULL, it stays not NULL while I use it." But Thread 2 can set it to NULL between Thread 1's check and use.

```
  Thread 1                         Thread 2
  ────────                         ────────
  if (thd->proc_info != NULL)
      // proc_info is valid here
                                   thd->proc_info = NULL;
      fputs(thd->proc_info, ...)
      // ^^^ NULL dereference! CRASH!
```

This is a classic **TOCTOU** (Time-of-Check-to-Time-of-Use) bug.

### Fix: Lock for Mutual Exclusion

```c
pthread_mutex_t proc_lock = PTHREAD_MUTEX_INITIALIZER;

// Thread 1:                          // Thread 2:
pthread_mutex_lock(&proc_lock);       pthread_mutex_lock(&proc_lock);
if (thd->proc_info) {                thd->proc_info = NULL;
    fputs(thd->proc_info, ...);       pthread_mutex_unlock(&proc_lock);
}
pthread_mutex_unlock(&proc_lock);
```

Now the check-and-use in Thread 1 and the modification in Thread 2 are mutually exclusive.

### Another Common Atomicity Violation: Check-Then-Act

```c
// BROKEN: check-then-act without lock
if (count > 0) {
    count--;
    // ... use resource ...
}
// Another thread may have decremented count between
// the check and the decrement!
```

---

## 3. Order Violation Bugs

An **order violation** occurs when the programmer assumes operation A in one thread happens before operation B in another thread, but there's no synchronization to guarantee this.

### Example

```c
// Thread 1 (initialization):          // Thread 2 (usage):
thd->mThread = CreateThread(...);      thd->mThread->State = RUNNING;
```

Thread 2 assumes `mThread` has been initialized by Thread 1. But Thread 2 might run first:

```
  Thread 2                          Thread 1
  ────────                          ────────
  thd->mThread->State = RUNNING;
  ^^^ mThread is NULL!
  NULL dereference! CRASH!
                                    thd->mThread = CreateThread(...);
                                    ^^^ Too late.
```

### Fix: Condition Variable for Ordering

```c
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
int initialized = 0;

// Thread 1 (initialization):
thd->mThread = CreateThread(...);
pthread_mutex_lock(&mtx);
initialized = 1;
pthread_cond_signal(&cv);
pthread_mutex_unlock(&mtx);

// Thread 2 (usage):
pthread_mutex_lock(&mtx);
while (!initialized)
    pthread_cond_wait(&cv, &mtx);
pthread_mutex_unlock(&mtx);
thd->mThread->State = RUNNING;  // guaranteed: mThread is valid
```

Alternatively, a semaphore initialized to 0 works cleanly for simple ordering:

```c
sem_t init_done;
sem_init(&init_done, 0, 0);

// Thread 1:
thd->mThread = CreateThread(...);
sem_post(&init_done);

// Thread 2:
sem_wait(&init_done);
thd->mThread->State = RUNNING;
```

---

## 4. Deadlock Bugs

A **deadlock** occurs when two or more threads are each waiting for a resource held by another, forming a cycle. None can make progress.

### Classic Example: Lock Ordering

```c
// Thread 1:                          // Thread 2:
pthread_mutex_lock(&lock_A);          pthread_mutex_lock(&lock_B);
pthread_mutex_lock(&lock_B);          pthread_mutex_lock(&lock_A);
// ... critical section ...           // ... critical section ...
pthread_mutex_unlock(&lock_B);        pthread_mutex_unlock(&lock_A);
pthread_mutex_unlock(&lock_A);        pthread_mutex_unlock(&lock_B);
```

```
  Deadlock Scenario:

  Thread 1                          Thread 2
  ────────                          ────────
  lock(A) → acquired
                                    lock(B) → acquired
  lock(B) → BLOCKED (T2 holds B)
                                    lock(A) → BLOCKED (T1 holds A)

  ┌──────┐  waits for B  ┌──────┐
  │  T1  │ ──────────────►│  T2  │
  │holds │                │holds │
  │  A   │◄────────────── │  B   │
  └──────┘  waits for A   └──────┘

  Circular wait → DEADLOCK!
  Neither thread can proceed.
```

### When Can Deadlock Occur?

Deadlock requires code paths where:
- Multiple threads acquire multiple locks.
- Different threads acquire the same set of locks in different orders.
- Complex lock dependencies arise from calling functions that acquire locks while already holding other locks.

In large codebases (OS kernels, databases), lock dependencies can be indirect and hard to trace.

---

## 5. Four Necessary Conditions for Deadlock

All four of the following conditions must hold simultaneously for a deadlock to occur (Coffman conditions, 1971):

### 5.1. Mutual Exclusion

At least one resource must be held in a non-sharable mode — only one thread can use it at a time.

```
  Lock A is held by T1 exclusively.
  T2 cannot acquire Lock A until T1 releases it.
```

### 5.2. Hold and Wait

A thread holding at least one resource is waiting to acquire additional resources held by other threads.

```
  T1 holds Lock A and is waiting for Lock B.
```

### 5.3. No Preemption

Resources cannot be forcibly taken from a thread — they can only be released voluntarily by the thread holding them.

```
  The OS cannot forcibly take Lock A away from T1.
  T1 must explicitly call unlock(A).
```

### 5.4. Circular Wait

A chain of threads exists where each thread holds a resource that the next thread in the chain is waiting for, and the chain forms a cycle.

```
  T1 holds A, waits for B.
  T2 holds B, waits for A.
  (Cycle: T1 → T2 → T1)

  Or with more threads:
  T1 holds A, waits for B.
  T2 holds B, waits for C.
  T3 holds C, waits for A.
  (Cycle: T1 → T2 → T3 → T1)
```

**All four conditions must hold simultaneously** for deadlock to occur. Breaking any one condition prevents deadlock.

```
  ┌──────────────────────────────────────────────────────┐
  │          Four Conditions for Deadlock                 │
  │                                                      │
  │  ┌─────────────────┐  ┌─────────────────┐           │
  │  │ Mutual Exclusion │  │  Hold and Wait  │           │
  │  │ (non-sharable    │  │ (hold one, want │           │
  │  │  resources)      │  │  another)       │           │
  │  └────────┬────────┘  └────────┬────────┘           │
  │           │                    │                     │
  │           ▼                    ▼                     │
  │       ┌───────────────────────────┐                  │
  │       │      ALL FOUR MUST HOLD   │                  │
  │       │      FOR DEADLOCK         │                  │
  │       └───────────────────────────┘                  │
  │           ▲                    ▲                     │
  │           │                    │                     │
  │  ┌────────┴────────┐  ┌───────┴─────────┐           │
  │  │  No Preemption  │  │  Circular Wait  │           │
  │  │ (can't force    │  │ (cycle in wait  │           │
  │  │  release)       │  │  graph)         │           │
  │  └─────────────────┘  └─────────────────┘           │
  └──────────────────────────────────────────────────────┘

  Break ANY ONE condition → no deadlock possible.
```

---

## 6. Deadlock Prevention

Prevention strategies eliminate one of the four necessary conditions, making deadlock **impossible by design**.

### 6.1. Preventing Circular Wait: Lock Ordering

**Strategy**: Define a **total ordering** (or at least a **partial ordering**) on all locks, and always acquire locks in ascending order.

```c
// Convention: always acquire locks in order of address
void transfer(account_t *from, account_t *to, int amount) {
    // Determine lock order by address
    pthread_mutex_t *first  = (from < to) ? &from->lock : &to->lock;
    pthread_mutex_t *second = (from < to) ? &to->lock : &from->lock;

    pthread_mutex_lock(first);
    pthread_mutex_lock(second);
    from->balance -= amount;
    to->balance += amount;
    pthread_mutex_unlock(second);
    pthread_mutex_unlock(first);
}
```

```
  Without ordering (deadlock possible):

  Thread 1: transfer(A → B)        Thread 2: transfer(B → A)
  lock(A)                           lock(B)
  lock(B) → BLOCKED                lock(A) → BLOCKED
  DEADLOCK!

  With address-based ordering (deadlock impossible):

  Assume &A < &B:
  Thread 1: transfer(A → B)        Thread 2: transfer(B → A)
  lock(A)  (first by address)      lock(A)  (first by address) → BLOCKED
  lock(B)
  // ... transfer ...
  unlock(B)
  unlock(A)
                                   lock(A) → acquired
                                   lock(B)
                                   // ... transfer ...
```

**Total ordering**: Assign a unique number to each lock. Always acquire lower-numbered locks first.

**Partial ordering**: For locks that are never held together, no ordering needed. Only related locks need ordering. This is more practical in large systems.

```
  Linux kernel example:

  Lock hierarchy (simplified):
  i_mutex (inode lock)
      └── page lock
              └── i_pages lock

  Always acquire in this order. Never hold i_pages
  and then try to acquire i_mutex.

  The kernel has lockdep (lock dependency validator) that
  dynamically checks lock ordering at runtime during development.
```

### 6.2. Preventing Hold and Wait: Acquire All Locks at Once

**Strategy**: Acquire all needed locks atomically before entering the critical section. If any lock is unavailable, release all and retry.

**Approach 1: Master lock (prevention lock)**

```c
pthread_mutex_t master = PTHREAD_MUTEX_INITIALIZER;

void transfer(account_t *from, account_t *to, int amount) {
    pthread_mutex_lock(&master);      // acquire master lock first
    pthread_mutex_lock(&from->lock);
    pthread_mutex_lock(&to->lock);
    pthread_mutex_unlock(&master);    // can release master now

    from->balance -= amount;
    to->balance += amount;

    pthread_mutex_unlock(&to->lock);
    pthread_mutex_unlock(&from->lock);
}
```

The master lock ensures that no thread can be in the middle of acquiring locks while another thread is also acquiring locks — eliminating the "hold and wait" window.

**Downside**: Reduced concurrency. Two completely independent transfers (different accounts) are serialized at the master lock. This can be a significant performance bottleneck.

**Approach 2: Try-lock with backoff**

```c
void transfer(account_t *from, account_t *to, int amount) {
    while (1) {
        pthread_mutex_lock(&from->lock);
        if (pthread_mutex_trylock(&to->lock) == 0)
            break;  // got both locks
        // Couldn't get 'to' lock — release 'from' and retry
        pthread_mutex_unlock(&from->lock);
        // Optional: backoff / yield to avoid livelock
    }
    from->balance -= amount;
    to->balance += amount;
    pthread_mutex_unlock(&to->lock);
    pthread_mutex_unlock(&from->lock);
}
```

**Risk**: Livelock — two threads repeatedly acquire one lock, fail to get the second, release, retry. Adding random backoff mitigates this.

### 6.3. Allowing Preemption (Breaking "No Preemption")

Not commonly used for locks because forcibly taking a lock from a thread can leave shared data in an inconsistent state. However, some approaches:
- `pthread_mutex_trylock` with timeout (`pthread_mutex_timedlock`) — if you can't get the lock in time, give up and release what you hold.
- Some database systems abort transactions (release all locks) when deadlock is detected.

### Summary of Prevention Strategies

| Condition | Prevention Strategy | Tradeoff |
|-----------|-------------------|----------|
| Circular wait | Lock ordering (total or partial) | Must know all lock relationships; can be complex |
| Hold and wait | Master lock / acquire all at once | Reduced concurrency; performance hit |
| No preemption | trylock + backoff / timeouts | Livelock risk; complexity |
| Mutual exclusion | Use lock-free data structures | Hard to implement correctly; limited applicability |

---

## 7. Deadlock Avoidance

Prevention is **static** — it eliminates a condition by design, regardless of the actual thread behavior. **Avoidance** is **dynamic** — the system makes runtime decisions to avoid entering a deadlocked state.

### Banker's Algorithm (Dijkstra, 1965)

The classic deadlock avoidance algorithm. The OS (or resource manager) knows:
- Total resources available.
- Maximum demand of each process.
- Current allocation to each process.

Before granting a resource request, the algorithm checks if the resulting state is **safe** — meaning there exists some execution order in which all processes can complete.

```
  Safe State Check:

  Available resources: [3, 3, 2]  (3 types of resources)

  Process  Allocated   Max Need   Remaining Need
  P0       [0, 1, 0]  [7, 5, 3]  [7, 4, 3]
  P1       [2, 0, 0]  [3, 2, 2]  [1, 2, 2]
  P2       [3, 0, 2]  [9, 0, 2]  [6, 0, 0]
  P3       [2, 1, 1]  [2, 2, 2]  [0, 1, 1]
  P4       [0, 0, 2]  [4, 3, 3]  [4, 3, 1]

  Is this safe? Find a sequence where all can finish:
  P1 can finish (needs [1,2,2], have [3,3,2]) → release → [5,3,2]
  P3 can finish (needs [0,1,1], have [5,3,2]) → release → [7,4,3]
  P4 can finish (needs [4,3,1], have [7,4,3]) → release → [7,4,5]
  P0 can finish (needs [7,4,3], have [7,4,5]) → release → [7,5,5]
  P2 can finish (needs [6,0,0], have [7,5,5]) → release → [10,5,7]
  Safe sequence: P1 → P3 → P4 → P0 → P2 ✓
```

**Practical reality**: Banker's algorithm is **theoretically elegant but impractical**:
- Requires knowing maximum resource needs in advance — often unknown.
- O(n²) per request (n = number of processes).
- Too conservative — denies safe requests to maintain a safe state.
- Not used in general-purpose OS kernels.

---

## 8. Deadlock Detection and Recovery

Instead of preventing or avoiding deadlocks, some systems **allow** deadlocks to occur and then detect and recover from them.

### Detection: Wait-For Graph

Maintain a graph of dependencies: an edge from T1 → T2 means "T1 is waiting for a resource held by T2." Periodically check for **cycles** in this graph. A cycle = deadlock.

```
  Wait-For Graph:

  T1 ──► T2 ──► T3
                 │
                 ▼
  T4 ◄── T5 ◄── T6

  No cycle → no deadlock.

  T1 ──► T2 ──► T3
  ▲              │
  │              ▼
  └────── T4 ◄──┘

  Cycle: T1 → T2 → T3 → T4 → T1 → DEADLOCK!
```

### Recovery Strategies

Once a deadlock is detected:

1. **Kill one or more deadlocked processes/threads**: Release their resources. Simple but brutal — partial work is lost.
2. **Roll back to a checkpoint**: If the system supports checkpointing (databases do), roll back one participant to before it acquired the contested lock.
3. **Resource preemption**: Forcibly take a resource from one thread and give it to another. Difficult for in-memory locks.
4. **Reboot**: The "turn it off and on again" approach. Surprisingly common in practice for systems where deadlocks are rare.

### Practical Usage

| System | Approach |
|--------|----------|
| **Databases** (MySQL InnoDB, PostgreSQL) | Detect deadlocks via wait-for graphs. Abort one transaction (the "victim") and retry |
| **Linux kernel (lockdep)** | Detect potential deadlocks at **development time** by tracking lock ordering. Reports violations as warnings |
| **General-purpose OS** | Prevention via lock ordering discipline. No runtime detection for user-space locks |

---

## 9. Linux Kernel Lock Dependency Validator: lockdep

The Linux kernel includes **lockdep** — a runtime lock dependency checker that catches potential deadlock scenarios (not just actual deadlocks) during development.

### How It Works

lockdep tracks:
- **Lock classes**: Each lock type (not instance) gets a class. E.g., all inode mutexes belong to the same class.
- **Acquisition order**: When thread T acquires lock B while holding lock A, lockdep records the dependency A → B.
- **Cycle detection**: If acquiring lock A while holding lock B would create a cycle in the dependency graph, lockdep reports a **potential deadlock** — even if the actual deadlock didn't happen this time.

```bash
# lockdep warning in dmesg:
[  123.456789] =============================================
[  123.456790] WARNING: possible circular locking dependency detected
[  123.456791] ---------------------------------------------
[  123.456792] Thread-A/1234 is trying to acquire lock:
[  123.456793]  (&inode->i_mutex){+.+.}-{3:3}
[  123.456794]
[  123.456795] but task is already holding lock:
[  123.456796]  (&sb->s_type->i_mutex_key){+.+.}-{3:3}
[  123.456797]
[  123.456798] which lock already depends on the new lock:
[  123.456799]  -> (&inode->i_mutex){+.+.}-{3:3}
```

```bash
# Enable lockdep (kernel config):
CONFIG_PROVE_LOCKING=y
CONFIG_LOCKDEP=y
CONFIG_DEBUG_LOCK_ALLOC=y

# View lock statistics:
cat /proc/lockdep           # lock dependency graph
cat /proc/lockdep_stats     # statistics
cat /proc/lock_stat         # per-lock contention stats (CONFIG_LOCK_STAT=y)
```

lockdep is invaluable for kernel development — it catches ordering violations that might only deadlock under rare timing conditions.

---

## 10. Tools for Detecting Concurrency Bugs

### Compile-Time / Static Analysis

- **Clang Thread Safety Analysis**: Annotations in code (`__attribute__((guarded_by(mutex)))`) that the compiler checks at compile time.

```c
// Clang Thread Safety Annotations:
int counter __attribute__((guarded_by(mutex)));

void increment() {
    counter++;  // WARNING: reading variable 'counter' requires
                // holding mutex 'mutex'
}
```

### Runtime / Dynamic Analysis

- **ThreadSanitizer (TSan)**: Compile with `-fsanitize=thread`. Detects data races at runtime.

```bash
$ gcc -fsanitize=thread -g -O1 program.c -o program -lpthread
$ ./program
==================
WARNING: ThreadSanitizer: data race (pid=12345)
  Write of size 4 at 0x... by thread T2:
    #0 increment program.c:10
  Previous write of size 4 at 0x... by thread T1:
    #0 increment program.c:10
==================
```

- **Helgrind** (Valgrind tool): Detects lock order violations, data races, misuses of POSIX threading API.

```bash
$ valgrind --tool=helgrind ./program
```

- **DRD** (Valgrind tool): Another data race detector, less memory overhead than Helgrind.

```bash
$ valgrind --tool=drd ./program
```

### Summary of Tools

| Tool | Type | Detects | Overhead |
|------|------|---------|----------|
| lockdep | Kernel runtime | Lock ordering violations, potential deadlocks | ~20% kernel overhead |
| TSan | User-space runtime | Data races | 5-15x slowdown, 5-10x memory |
| Helgrind | User-space runtime | Races, lock order violations, API misuse | 20-100x slowdown |
| Clang TSA | Compile-time | Missing lock acquisitions (with annotations) | Zero runtime overhead |

---

## Summary

| Bug Type | Description | Fix |
|----------|------------|-----|
| **Atomicity violation** | Intended-atomic region interrupted by another thread (TOCTOU) | Lock (mutex) to make region truly atomic |
| **Order violation** | Assumed execution order not guaranteed | Condition variable or semaphore for explicit ordering |
| **Deadlock** | Circular waiting — threads waiting on each other's locks | Prevention (lock ordering, master lock) or detection + recovery |

| Deadlock Condition | Prevention Strategy |
|-------------------|-------------------|
| Circular wait | Total/partial lock ordering |
| Hold and wait | Master lock / acquire all at once |
| No preemption | trylock + backoff / timeout |
| Mutual exclusion | Lock-free data structures |

| Approach | Description | Practical? |
|----------|------------|-----------|
| **Prevention** | Structurally eliminate one condition | Yes — lock ordering is standard practice |
| **Avoidance** | Dynamic safe-state analysis (Banker's) | No — requires future knowledge, too conservative |
| **Detection + Recovery** | Allow deadlocks, detect cycles, kill/abort | Yes — used in databases |
