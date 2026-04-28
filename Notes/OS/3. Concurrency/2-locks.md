# Locks

---

## 1. What Is a Lock?

A **lock** (or **mutex** — mutual exclusion object) is the most basic synchronization primitive. It ensures that only one thread at a time can execute the critical section.

### API (Conceptual)

```c
lock_t mutex;

lock(&mutex);
// ─── critical section ───
// only one thread at a time can be here
counter++;
// ─── end critical section ───
unlock(&mutex);
```

The lock has two states:
- **Available** (unlocked/free): No thread holds it.
- **Acquired** (locked/held): Exactly one thread holds it; other threads that try to `lock()` must wait.

```
  Lock State Machine:

  ┌───────────┐   lock() by T1    ┌───────────┐
  │ AVAILABLE │ ─────────────────► │  ACQUIRED  │
  │           │                    │  (by T1)   │
  └───────────┘ ◄───────────────── └───────────┘
                  unlock() by T1

  While ACQUIRED by T1:
   - T2 calls lock() → T2 BLOCKS (waits)
   - T1 calls unlock() → lock becomes AVAILABLE → T2 wakes up and acquires
```

---

## 2. Why Naive Software-Only Approaches Fail

Before looking at real solutions, let's understand **why implementing locks is hard** without hardware support.

### Attempt 1: Simple Flag

```c
typedef struct { int flag; } lock_t;

void lock(lock_t *l) {
    while (l->flag == 1)
        ;  // spin-wait (busy loop)
    l->flag = 1;  // acquire
}

void unlock(lock_t *l) {
    l->flag = 0;
}
```

**This is BROKEN.** The check-then-set (`while (flag == 1); flag = 1;`) is itself a race condition — two threads can both see `flag == 0`, then both set it to `1`, both believing they hold the lock.

```
  Thread 1                     Thread 2
  ───────────                  ───────────
  read flag → 0 (not held)
                               read flag → 0 (not held)
  set flag = 1 (acquired!)
                               set flag = 1 (acquired!)

  BOTH threads think they hold the lock.
  Critical section violated!
```

The fundamental problem: **the test and set must happen atomically** — as a single indivisible operation. Software alone can't guarantee this on modern hardware without help.

### Attempt 2: Taking Turns (Strict Alternation)

```c
int turn = 0;  // shared

// Thread 0:                       // Thread 1:
while (turn != 0) ;               while (turn != 1) ;
// critical section                // critical section
turn = 1;                         turn = 0;
```

This achieves mutual exclusion but fails the **progress** requirement — threads must strictly alternate. If Thread 0 wants to enter twice in a row but Thread 1 hasn't run yet, Thread 0 is stuck.

### Attempt 3: Peterson's Algorithm (for 2 threads)

```c
int flag[2] = {0, 0};
int turn;

void lock(int self) {
    int other = 1 - self;
    flag[self] = 1;       // I want to enter
    turn = other;          // But I yield to the other
    while (flag[other] == 1 && turn == other)
        ;  // spin
}

void unlock(int self) {
    flag[self] = 0;
}
```

Peterson's algorithm is **correct for 2 threads** under the sequential consistency memory model. However, it **does not work on modern hardware** without memory barriers because:
- Modern CPUs reorder memory operations (store buffers, write combining).
- The compiler can reorder instructions.
- Without explicit memory fences, the visibility guarantees break.

This is why we need **hardware atomic instructions**.

---

## 3. Hardware Atomic Instructions

Modern CPUs provide atomic **read-modify-write** instructions that execute indivisibly — no other thread can interleave between the read and the write.

### Test-and-Set (TAS)

Atomically: read old value, write new value, return old value.

```c
// Hardware provides this as a SINGLE atomic instruction:
int test_and_set(int *ptr, int new_val) {
    int old = *ptr;
    *ptr = new_val;
    return old;
}
// On x86: XCHG instruction (lock prefix implicit)
```

**Spin lock using TAS:**

```c
typedef struct { int flag; } spinlock_t;

void spin_lock(spinlock_t *l) {
    while (test_and_set(&l->flag, 1) == 1)
        ;  // spin until we successfully set flag from 0 → 1
    // If test_and_set returned 0, the old value was 0 (unlocked),
    // and we atomically set it to 1 (locked). We hold the lock.
}

void spin_unlock(spinlock_t *l) {
    l->flag = 0;  // Simple store is sufficient
                   // (with a memory barrier on weakly-ordered architectures)
}
```

Why this works:

```
  Thread 1                          Thread 2
  ────────                          ────────
  TAS(&flag, 1) → returns 0        
  (atomically read 0, wrote 1)     
  Lock acquired!                    TAS(&flag, 1) → returns 1
                                    (flag was already 1)
                                    Keeps spinning...
  
  unlock: flag = 0
                                    TAS(&flag, 1) → returns 0
                                    Lock acquired!
```

**x86 assembly for a spin lock:**

```asm
spin_lock:
    mov eax, 1
.retry:
    xchg eax, [lock]    ; atomic exchange: eax ↔ [lock]
    test eax, eax        ; was old value 0?
    jnz .retry           ; if not, lock was held → retry
    ret                  ; lock acquired

spin_unlock:
    mov dword [lock], 0  ; release
    ret
```

### Compare-and-Swap (CAS)

Atomically: if `*ptr == expected`, set `*ptr = new_val`, return old value.

```c
// Hardware provides this atomically:
int compare_and_swap(int *ptr, int expected, int new_val) {
    int old = *ptr;
    if (old == expected)
        *ptr = new_val;
    return old;
}
// On x86: CMPXCHG instruction (with LOCK prefix)
```

**Spin lock using CAS:**

```c
void spin_lock(spinlock_t *l) {
    while (compare_and_swap(&l->flag, 0, 1) != 0)
        ;  // spin until CAS succeeds (flag was 0, now 1)
}
```

CAS is **more powerful** than TAS — it only modifies the value if it matches the expected value. This is the basis for **lock-free** data structures and algorithms.

### Fetch-and-Add

Atomically: increment `*ptr` by 1, return old value.

```c
int fetch_and_add(int *ptr) {
    int old = *ptr;
    *ptr = old + 1;
    return old;
}
// On x86: LOCK XADD instruction
```

**Ticket lock using fetch-and-add:**

```c
typedef struct {
    int ticket;     // next ticket to hand out
    int turn;       // which ticket is currently being served
} ticket_lock_t;

void ticket_lock(ticket_lock_t *l) {
    int my_ticket = fetch_and_add(&l->ticket);
    while (l->turn != my_ticket)
        ;  // spin until it's my turn
}

void ticket_unlock(ticket_lock_t *l) {
    l->turn++;  // serve the next ticket
}
```

Ticket locks guarantee **FIFO ordering** — threads acquire the lock in the order they requested it. This solves the **starvation** problem of basic TAS spin locks.

```
  Ticket Lock (FIFO fairness):

  T1 arrives → ticket=0, turn=0 → enters immediately
  T2 arrives → ticket=1, turn=0 → spins
  T3 arrives → ticket=2, turn=0 → spins

  T1 finishes → turn=1 → T2 enters
  T2 finishes → turn=2 → T3 enters

  Strict FIFO: no starvation!
```

---

## 4. Evaluating Lock Implementations

Three criteria for judging a lock implementation:

### 4.1. Correctness (Mutual Exclusion)

Does the lock actually provide mutual exclusion? At most one thread in the critical section at any time.

- **Naive flag**: FAILS (race in test-and-set).
- **TAS spin lock**: PASSES (atomic instruction guarantees it).
- **CAS spin lock**: PASSES.
- **Ticket lock**: PASSES.

### 4.2. Fairness (No Starvation)

Does every thread that requests the lock eventually get it?

- **TAS spin lock**: NO guarantee. A thread can be **unlucky** and keep losing the race to another thread. On highly contended locks, starvation is possible.
- **Ticket lock**: YES — strict FIFO ordering. Each thread gets a ticket number and waits for its turn. No starvation.

### 4.3. Performance

How much overhead does the lock add?

**Spin locks on single core:**
- **Terrible.** If the lock holder is preempted, all other threads spin uselessly for the entire time slice, wasting CPU. The lock holder can't make progress to release the lock because it's not running.
- N threads contending → (N-1) * time_slice of wasted CPU per scheduling round.

**Spin locks on multi-core (low contention):**
- Acceptable. If the lock is held for a very short time and the holder is running on another core, spinning wastes fewer cycles than the overhead of a context switch.

**Spin locks on multi-core (high contention):**
- Bad. Many threads spinning → cache line bouncing → memory bus saturation. Each TAS/XCHG invalidates the cache line on all other cores, generating heavy interconnect traffic.

```
  TAS Spin Lock — Cache Line Bouncing:

  Core 0         Core 1         Core 2         Core 3
  XCHG lock      XCHG lock      XCHG lock      XCHG lock
      │              │              │              │
      ▼              ▼              ▼              ▼
  ┌────────────────────────────────────────────────────┐
  │  Coherence Interconnect (MESI protocol)            │
  │  Each XCHG invalidates the cache line on all       │
  │  other cores → "cache line ping-pong"              │
  └────────────────────────────────────────────────────┘
```

---

## 5. Spin Lock vs. Sleeping Mutex (Blocking Lock)

The key distinction in lock implementation strategy:

### Spin Lock

The thread **busy-waits** in a loop until the lock is free.

```c
while (test_and_set(&lock, 1) == 1)
    ;  // spin — burns CPU cycles
```

- **Pros**: No context switch overhead; very fast when lock is held briefly.
- **Cons**: Wastes CPU while spinning; terrible on single-core; cache line contention.
- **Use when**: Lock is held for a very short time (dozens of instructions); in kernel interrupt handlers (can't sleep in interrupt context).

### Sleeping Mutex (Blocking Lock)

If the lock is held, the thread **sleeps** (yields the CPU) and is **woken up** when the lock is released.

```c
void mutex_lock(mutex_t *m) {
    while (test_and_set(&m->guard, 1) == 1)
        ;  // brief spin on guard
    if (m->flag == 0) {
        m->flag = 1;          // lock acquired
        m->guard = 0;
    } else {
        enqueue(m->wait_queue, self);
        m->guard = 0;
        sleep();              // yield CPU, deschedule self
    }
}

void mutex_unlock(mutex_t *m) {
    while (test_and_set(&m->guard, 1) == 1)
        ;
    if (queue_empty(m->wait_queue)) {
        m->flag = 0;          // no one waiting, release lock
    } else {
        wakeup(dequeue(m->wait_queue));  // wake one waiter, transfer lock
    }
    m->guard = 0;
}
```

Key insight: The **guard** spin lock protects the lock's internal state (flag, queue) and is held for only a few instructions — so spinning on it is acceptable. The actual blocking is done by sleeping.

- **Pros**: No CPU waste while waiting; fair with FIFO wait queue.
- **Cons**: Context switch overhead (save/restore registers, potential TLB flush if switching to a different process).
- **Use when**: Lock is held for a long time (I/O, computation); user-space general-purpose synchronization.

```
  Spin Lock vs. Sleeping Mutex — Cost Comparison:

  Time ──────────────────────────────────────────────►

  Spin Lock:
  T1:  ┌─LOCK──────────────────────────────────UNLOCK─┐
  T2:  ─────[SPIN SPIN SPIN SPIN SPIN]─────LOCK─UNLOCK──
             ^^^ Wasting CPU cycles ^^^

  Sleeping Mutex:
  T1:  ┌─LOCK──────────────────────────────────UNLOCK─┐
  T2:  ─────[SLEEP]───────────────────────[WAKE]─LOCK─UNLOCK──
             ^^^ CPU free for other work ^^^
             (but context switch overhead on sleep + wake)
```

### Linux Implementation: `futex` (Fast Userspace Mutex)

Linux provides the **`futex`** syscall — the building block for efficient user-space synchronization. The key idea: **fast path in user space, slow path in kernel.**

```c
#include <linux/futex.h>
#include <sys/syscall.h>

// futex(addr, op, val, ...)
// Key operations:
//   FUTEX_WAIT: if *addr == val, sleep (atomically checked in kernel)
//   FUTEX_WAKE: wake up N threads sleeping on addr
```

**How glibc's `pthread_mutex_lock` works (simplified, using futex):**

```c
void mutex_lock(int *mutex) {
    // Fast path: try to acquire with atomic CAS (no syscall!)
    if (atomic_cas(mutex, 0, 1) == 0)
        return;  // Got the lock, no kernel involved!

    // Slow path: lock is contended
    while (1) {
        // Mark as contended (value 2 means "locked + waiters")
        int old = atomic_exchange(mutex, 2);
        if (old == 0)
            return;  // Lucky — lock was just released

        // Sleep in kernel until *mutex != 2
        syscall(SYS_futex, mutex, FUTEX_WAIT, 2, NULL, NULL, 0);

        // Woken up — retry the CAS
    }
}

void mutex_unlock(int *mutex) {
    // If no contention (value was 1), just release
    if (atomic_exchange(mutex, 0) == 1)
        return;  // No waiters, no kernel involved!

    // There are waiters (value was 2), wake one up
    syscall(SYS_futex, mutex, FUTEX_WAKE, 1, NULL, NULL, 0);
}
```

The futex brilliance:
- **Uncontended case** (common): Single atomic instruction in user space. No syscall. No kernel transition. Very fast.
- **Contended case** (rare): Falls back to kernel to sleep/wake threads, avoiding busy-waiting.

```
  futex Fast Path vs. Slow Path:

  ┌────────────────────────────────────────┐
  │ User Space                             │
  │                                        │
  │  lock():                               │
  │    CAS(mutex, 0, 1)  ─── success ───► return (FAST PATH)
  │       │                                │
  │       │ (contended)                    │
  │       ▼                                │
  ├────── syscall ─────────────────────────┤
  │ Kernel Space                           │
  │                                        │
  │  FUTEX_WAIT:                           │
  │    check *mutex == val                 │
  │    add thread to wait queue            │
  │    schedule() → sleep                  │
  │                                        │
  │  FUTEX_WAKE:                           │
  │    remove thread from wait queue       │
  │    wake it up                          │
  └────────────────────────────────────────┘
```

### Kernel Spin Locks and Mutexes

Inside the Linux kernel itself:

| Primitive | `spin_lock()` / `spin_lock_irqsave()` | `mutex_lock()` |
|-----------|---------------------------------------|----------------|
| Behavior | Busy-wait spin | Sleep if contended |
| Can sleep? | **NO** — must not sleep while holding | Yes |
| Use context | Interrupt handlers, short critical sections | Process context, longer sections |
| Preemption | Disables preemption (and optionally IRQs) | May sleep and reschedule |
| Header | `<linux/spinlock.h>` | `<linux/mutex.h>` |

```c
// Kernel spin lock example:
spinlock_t my_lock;
spin_lock_init(&my_lock);

spin_lock(&my_lock);       // disable preemption + acquire
// ... short critical section ...
spin_unlock(&my_lock);     // release + re-enable preemption

// Kernel mutex example:
struct mutex my_mutex;
mutex_init(&my_mutex);

mutex_lock(&my_mutex);     // may sleep if contended
// ... longer critical section (can call schedule(), do I/O) ...
mutex_unlock(&my_mutex);
```

---

## 6. Additional Locking Concepts

### Read-Write Locks (`pthread_rwlock`)

When reads are frequent and writes are rare, a standard mutex forces unnecessary serialization of readers. A **read-write lock** allows:
- **Multiple concurrent readers** (shared mode).
- **Exclusive writer** (exclusive mode). No readers allowed while writing.

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Reader:
pthread_rwlock_rdlock(&rwlock);
// ... read shared data ...
pthread_rwlock_unlock(&rwlock);

// Writer:
pthread_rwlock_wrlock(&rwlock);
// ... modify shared data ...
pthread_rwlock_unlock(&rwlock);
```

```
  Read-Write Lock Access Matrix:

              │ Reader holding │ Writer holding
  ────────────┼────────────────┼────────────────
  Reader wants│   ALLOWED      │    BLOCKED
  Writer wants│   BLOCKED      │    BLOCKED

  Multiple readers can proceed in parallel.
  A writer must wait for ALL readers to release.
```

### Recap: `volatile` Is NOT a Synchronization Mechanism

In C/C++, `volatile` tells the compiler not to cache the variable in a register — always read from memory. However:
- `volatile` does **not** provide atomicity (read-modify-write is still non-atomic).
- `volatile` does **not** prevent CPU reordering.
- `volatile` does **not** act as a memory barrier.

**Never use `volatile` as a substitute for locks or atomics in C/C++.**

Use `<stdatomic.h>` (C11) or `<atomic>` (C++11) for lock-free atomic operations. Use pthreads mutexes for lock-based synchronization.

---

## 7. Guidelines for Using Locks

1. **Keep critical sections short.** The longer you hold a lock, the more contention, the worse the performance.

2. **Don't hold locks across blocking operations** (I/O, sleep, etc.) unless absolutely necessary. This blocks all other threads waiting for that lock.

3. **Beware of lock ordering.** If you need multiple locks, always acquire them in the same global order to prevent deadlock (covered in Concurrency Bugs).

4. **Prefer coarse-grained locking for correctness, refine to fine-grained for performance.** Start with one big lock. Profile. Only split into finer locks if contention is a measured bottleneck.

5. **Use the right lock type:**
   - Short kernel critical sections → spin lock.
   - User-space general purpose → `pthread_mutex` (futex-based).
   - Read-heavy workloads → `pthread_rwlock`.

6. **Always pair lock and unlock.** Use RAII in C++ (`std::lock_guard`, `std::unique_lock`) to guarantee unlock even if exceptions are thrown.

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Lock (mutex) | Ensures only one thread in critical section at a time |
| Naive flag approach | Broken — test-and-set race in software |
| Test-and-Set (TAS) | Hardware atomic: read old, write new, return old. Basis for spin locks |
| Compare-and-Swap (CAS) | Hardware atomic: conditional update. More powerful. Basis for lock-free algorithms |
| Ticket lock | Fetch-and-add based. FIFO fairness, no starvation |
| Spin lock | Busy-wait. Fast for short critical sections. Bad on single core |
| Sleeping mutex | Sleep if contended. Good for long critical sections |
| futex | Linux: fast-path atomic in user space, slow-path kernel sleep/wake |
| Kernel spin_lock | Disables preemption. For interrupt context, short CS |
| Kernel mutex | Can sleep. For process context, longer CS |
| Read-write lock | Multiple readers OR one writer. Good for read-heavy workloads |
