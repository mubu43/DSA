# Condition Variables

---

## 1. Beyond Locks: The Need for Waiting

Locks provide mutual exclusion — ensuring only one thread is in the critical section. But what if a thread needs to **wait for a condition to become true** before proceeding?

### Naive Approach: Spin-Waiting on a Condition

```c
// Thread 1: wait until 'done' is set by Thread 2
while (done == 0)
    ;  // spin — wastes CPU!
// ... proceed ...
```

This works for correctness but:
- **Wastes CPU** — the thread burns cycles checking the condition.
- On a single-core system, it can prevent the thread that would set `done = 1` from ever running.

### A Better Approach: Sleep and Be Woken Up

We need a mechanism where a thread can:
1. **Sleep** (yield the CPU) when a condition is not yet true.
2. **Be woken up** by another thread when the condition becomes true.

This is exactly what **condition variables** provide.

---

## 2. Condition Variable Overview

A **condition variable** (CV) is a synchronization primitive that allows threads to wait for a condition and be signaled when the condition changes.

### POSIX API (`<pthread.h>`)

```c
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Wait: atomically release mutex and sleep. Re-acquire mutex on wake-up.
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

// Signal: wake up ONE thread waiting on cond.
int pthread_cond_signal(pthread_cond_t *cond);

// Broadcast: wake up ALL threads waiting on cond.
int pthread_cond_broadcast(pthread_cond_t *cond);
```

**Critical rule**: `pthread_cond_wait` must **always** be called with the mutex **held**. The wait operation atomically:
1. Releases the mutex.
2. Puts the calling thread to sleep on the CV's wait queue.
3. On wake-up, re-acquires the mutex before returning.

---

## 3. The Missed Wake-Up Problem

Why must `wait` be paired with a mutex? Consider what happens without one:

### Broken: Wait Without Mutex

```c
// BROKEN — no mutex protection
// Thread 1 (waiter):                 // Thread 2 (signaler):
if (done == 0)                        done = 1;
    cond_wait(&cond);                 cond_signal(&cond);
// proceed...
```

Race condition:

```
  Thread 1                          Thread 2
  ────────                          ────────
  Check: done == 0  (true)
                                    done = 1
                                    cond_signal(&cond)
                                    ^^^ Signal sent, but nobody
                                        is sleeping yet!
  cond_wait(&cond)
  ^^^ Goes to sleep FOREVER.
      The signal was already sent and lost!

  This is the "MISSED WAKE-UP" (or "lost wake-up") problem.
```

The window between checking the condition and going to sleep is the vulnerability. The signal arrives in this window and is lost.

### Correct: Wait With Mutex

```c
// Thread 1 (waiter):                 // Thread 2 (signaler):
pthread_mutex_lock(&mutex);           pthread_mutex_lock(&mutex);
while (done == 0)                     done = 1;
    pthread_cond_wait(&cond, &mutex); pthread_cond_signal(&cond);
pthread_mutex_unlock(&mutex);         pthread_mutex_unlock(&mutex);
```

Now the check-and-sleep is atomic (under the mutex). Thread 2 cannot set `done = 1` and signal between Thread 1's check and sleep, because Thread 2 needs the mutex which Thread 1 holds.

```
  Thread 1                           Thread 2
  ────────                           ────────
  lock(mutex)
  check done == 0 → true
  cond_wait():                       lock(mutex) → BLOCKED
    - release mutex (atomically)        (can't proceed until T1 sleeps)
    - sleep on CV
                                     lock(mutex) → acquired!
                                     done = 1
                                     cond_signal(&cond)
                                     unlock(mutex)
  woken up by signal
  re-acquire mutex
  check done == 0 → false (loop exits)
  unlock(mutex)
  proceed!
```

### Why `while` and Not `if`?

Always use `while (condition)` around `cond_wait`, never `if (condition)`:

```c
// CORRECT:
while (done == 0)                    // re-check condition after wake
    pthread_cond_wait(&cond, &mutex);

// WRONG:
if (done == 0)                       // do NOT use 'if'
    pthread_cond_wait(&cond, &mutex);
```

Reasons:
1. **Spurious wake-ups**: The POSIX spec allows `pthread_cond_wait` to return even without a signal (an implementation artifact of the kernel). The condition must be rechecked.
2. **Multiple waiters**: If `broadcast` wakes all waiters but only one should proceed (e.g., only one item available), the others must go back to sleep.
3. **Mesa semantics**: After being woken, another thread may have already changed the condition before the woken thread re-acquires the mutex. (See below.)

---

## 4. Mesa Semantics vs. Hoare Semantics

Two design choices for what happens when a thread is signaled:

| Aspect | Mesa Semantics | Hoare Semantics |
|--------|---------------|-----------------|
| After signal | Signaled thread is made **runnable** but doesn't run immediately | Signaled thread runs **immediately**; signaler is suspended |
| Condition guarantee | Condition **may not hold** when woken thread runs | Condition **guaranteed to hold** when woken thread runs |
| Wait loop | Must use `while` loop to recheck | `if` is sufficient |
| Implementations | pthreads, Java, Go, most real systems | Theoretical; rarely implemented |

**All practical systems use Mesa semantics.** After a signal, the condition may change before the woken thread gets the CPU and acquires the mutex. Therefore: **always recheck in a `while` loop**.

---

## 5. Producer-Consumer Problem

The **producer-consumer** (or **bounded-buffer**) problem is the classic synchronization problem. It appears everywhere in systems programming.

### Setup

- A **bounded buffer** of size N (e.g., a circular queue).
- **Producers** add items to the buffer.
- **Consumers** remove items from the buffer.
- Producers must wait when the buffer is **full**.
- Consumers must wait when the buffer is **empty**.

### Real-World Example: Multi-Threaded Server

```
  ┌───────────────────────────────────────────────────────┐
  │  Multi-Threaded Server                                │
  │                                                       │
  │  Master Thread                                        │
  │  ┌─────────────────┐                                  │
  │  │ accept() loop   │── new connection fd ──►┌────────┐│
  │  │ (producer)      │                        │ Bounded ││
  │  └─────────────────┘                        │ Buffer  ││
  │                                             │ (size N)││
  │  Worker Threads (consumers)                 └────┬───┘│
  │  ┌─────────┐ ┌─────────┐ ┌─────────┐           │    │
  │  │Worker 0 │ │Worker 1 │ │Worker 2 │◄──────────┘    │
  │  │ read fd │ │ read fd │ │ read fd │                 │
  │  │ service │ │ service │ │ service │                 │
  │  └─────────┘ └─────────┘ └─────────┘                 │
  └───────────────────────────────────────────────────────┘

  Master thread accepts client connections, puts file
  descriptors into a bounded buffer. Worker threads pick
  up descriptors and service client requests.
```

### Attempt 1: One CV (BROKEN)

```c
int buffer[MAX];
int count = 0;  // items in buffer
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg) {
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == MAX)
            pthread_cond_wait(&cond, &mutex);  // buffer full → wait
        put(i);  // add item to buffer, count++
        pthread_cond_signal(&cond);            // signal a waiting thread
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg) {
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0)
            pthread_cond_wait(&cond, &mutex);  // buffer empty → wait
        int val = get();  // remove item from buffer, count--
        pthread_cond_signal(&cond);            // signal a waiting thread
        pthread_mutex_unlock(&mutex);
    }
}
```

**The bug**: With multiple consumers, a consumer's signal might wake another **consumer** instead of a **producer**:

```
  State: buffer is empty (count == 0)

  Consumer C1: waits (buffer empty)
  Consumer C2: waits (buffer empty)

  Producer P1: puts item, count = 1, signals → wakes C1

  C1 wakes up, gets item, count = 0
  C1 signals → wakes C2 (WRONG! Should wake a producer!)

  C2 wakes up, checks count == 0 → goes back to sleep (OK with while loop)
  But now: C1 is done, C2 is sleeping, P1 might be sleeping too
  → EVERYONE IS SLEEPING. DEADLOCK!
```

The fundamental issue: with one CV, a signal can wake the wrong type of thread (consumer wakes consumer instead of producer, or vice versa).

### Solution: Two CVs

Use separate condition variables — one for "buffer not empty" (consumers wait on this), one for "buffer not full" (producers wait on this).

```c
int buffer[MAX];
int count = 0;
int fill_ptr = 0, use_ptr = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;  // consumers wait here
pthread_cond_t not_full  = PTHREAD_COND_INITIALIZER;  // producers wait here

void put(int value) {
    buffer[fill_ptr] = value;
    fill_ptr = (fill_ptr + 1) % MAX;
    count++;
}

int get() {
    int val = buffer[use_ptr];
    use_ptr = (use_ptr + 1) % MAX;
    count--;
    return val;
}

void *producer(void *arg) {
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == MAX)
            pthread_cond_wait(&not_full, &mutex);   // wait: buffer full
        put(i);
        pthread_cond_signal(&not_empty);            // signal: item available
        pthread_mutex_unlock(&mutex);
    }
}

void *consumer(void *arg) {
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 0)
            pthread_cond_wait(&not_empty, &mutex);  // wait: buffer empty
        int val = get();
        pthread_cond_signal(&not_full);             // signal: slot available
        pthread_mutex_unlock(&mutex);
        process(val);
    }
}
```

Now producers always signal `not_empty` (waking consumers) and consumers always signal `not_full` (waking producers). No wrong-type wake-ups.

```
  Two CVs — Signaling Paths:

  Producer                              Consumer
  ────────                              ────────
  wait on: not_full                     wait on: not_empty
  signal:  not_empty ──────────────────► wakes consumer
                      ◄────────────────  signal: not_full
                         wakes producer

  Producers and consumers can ONLY wake each other,
  never the same type.
```

---

## 6. Batched Processing Problem

A more subtle synchronization problem that illustrates common pitfalls with CVs.

### Setup

- Request threads arrive one at a time.
- A **batch processor** thread should process a batch of N requests at once.
- The Nth request thread must wake up the batch processor.
- After processing, the batch processor signals all N request threads to continue.

```
  ┌──────────────────────────────────────────────────┐
  │  Batched Processing                              │
  │                                                  │
  │  Request Threads:     T1  T2  T3  ... TN         │
  │                       │   │   │       │          │
  │                       ▼   ▼   ▼       ▼          │
  │                     ┌──────────────────┐         │
  │                     │ Wait until N     │         │
  │                     │ requests arrive  │         │
  │                     └────────┬─────────┘         │
  │                              │ Nth arrival       │
  │                              ▼                   │
  │                     ┌──────────────────┐         │
  │                     │ Batch Processor  │         │
  │                     │ processes all N  │         │
  │                     └────────┬─────────┘         │
  │                              │                   │
  │                              ▼                   │
  │                     All N requests get response   │
  └──────────────────────────────────────────────────┘
```

### Solution Using Two CVs

```c
int request_count = 0;
int batch_done = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t batch_ready  = PTHREAD_COND_INITIALIZER; // batch processor waits
pthread_cond_t batch_complete = PTHREAD_COND_INITIALIZER; // request threads wait

void *request_thread(void *arg) {
    pthread_mutex_lock(&mutex);
    request_count++;

    if (request_count == N) {
        // I'm the Nth request — wake up batch processor
        pthread_cond_signal(&batch_ready);
    }

    // Wait for batch processing to complete
    while (!batch_done)
        pthread_cond_wait(&batch_complete, &mutex);

    pthread_mutex_unlock(&mutex);
    // ... proceed with response ...
}

void *batch_processor(void *arg) {
    pthread_mutex_lock(&mutex);

    // Wait until N requests have arrived
    while (request_count < N)
        pthread_cond_wait(&batch_ready, &mutex);

    // Process the batch
    process_batch();

    // Signal all waiting request threads
    batch_done = 1;
    pthread_cond_broadcast(&batch_complete);

    pthread_mutex_unlock(&mutex);
}
```

### The Deadlock Trap

Consider a subtle bug: What if the Nth request thread calls `cond_wait` **before** calling `cond_signal` to wake the batch processor?

```c
// BUGGY Nth request thread:
void *request_thread(void *arg) {
    pthread_mutex_lock(&mutex);
    request_count++;

    // BUG: Wait FIRST, signal LATER
    while (!batch_done)
        pthread_cond_wait(&batch_complete, &mutex);  // goes to sleep!

    // This signal never executes — we're already asleep!
    if (request_count == N)
        pthread_cond_signal(&batch_ready);

    pthread_mutex_unlock(&mutex);
}
```

```
  The Deadlock Scenario:

  T1, T2, ..., T(N-1):  All waiting on batch_complete (sleeping)
  T(N):                  Increments count to N
                         Then calls wait on batch_complete → SLEEPS!
                         Never signals batch_ready!
  Batch Processor:       Waiting on batch_ready → SLEEPS!

  EVERYONE IS ASLEEP. DEADLOCK!
  ─────────────────────────────
  The signal to wake the batch processor was never sent
  because the Nth thread went to sleep first.
```

**The lesson**: **Signal others before calling wait and going to sleep.** The order of operations matters critically. If you need to both signal and wait, signal first.

---

## 7. Synchronization Patterns with CVs

### Pattern 1: Simple Notification (One-Shot)

One thread waits for another to complete something.

```c
int done = 0;
pthread_mutex_t m;
pthread_cond_t c;

// Waiter:                               // Signaler:
pthread_mutex_lock(&m);                  pthread_mutex_lock(&m);
while (!done)                            done = 1;
    pthread_cond_wait(&c, &m);           pthread_cond_signal(&c);
pthread_mutex_unlock(&m);                pthread_mutex_unlock(&m);
```

### Pattern 2: Barrier

N threads must all reach a point before any can proceed.

```c
int arrived = 0;
pthread_mutex_t m;
pthread_cond_t c;

void barrier() {
    pthread_mutex_lock(&m);
    arrived++;
    if (arrived == N) {
        arrived = 0;  // reset for reuse
        pthread_cond_broadcast(&c);  // wake ALL
    } else {
        while (arrived != 0)         // not all arrived yet
            pthread_cond_wait(&c, &m);
    }
    pthread_mutex_unlock(&m);
}
```

pthreads provides a built-in barrier: `pthread_barrier_t`.

### Pattern 3: Counting / Resource Pool

Wait until a resource count reaches a threshold.

```c
int available = 0;
pthread_mutex_t m;
pthread_cond_t c;

// Wait for a resource:
pthread_mutex_lock(&m);
while (available <= 0)
    pthread_cond_wait(&c, &m);
available--;
pthread_mutex_unlock(&m);

// Release a resource:
pthread_mutex_lock(&m);
available++;
pthread_cond_signal(&c);
pthread_mutex_unlock(&m);
```

---

## 8. Guidelines for Writing Correct Synchronization Code

A systematic approach for getting synchronization right:

### Step 1: Identify Shared State

List all variables/data structures accessed by multiple threads. These need protection.

### Step 2: Identify When Each Entity Should Wait

For each thread type, determine:
- Under what condition should it **block** (not proceed)?
- What shared state determines this condition?

| Entity | Waits When | CV |
|--------|-----------|-----|
| Producer | Buffer is full (`count == MAX`) | `not_full` |
| Consumer | Buffer is empty (`count == 0`) | `not_empty` |

### Step 3: Figure Out Signaling

For each wait, determine:
- **Which thread** will make the condition true?
- **When** should it signal? (Right after changing the state that makes the condition true.)

### Step 4: Ensure Signaling Path Is Not Blocked

This is where bugs creep in:
- **Signal before waiting**, not after. If a thread needs to both signal and wait, signal first.
- **Don't hold unnecessary locks** that might block the signaler.
- Check that the signaling thread can actually reach the signal call.

### Step 5: Update Auxiliary Variables Correctly

Counters, flags, and indices must be updated **under the mutex** and in the right order relative to signal/wait calls.

### Step 6: Watch for Deadlocks

- If using multiple mutexes, ensure consistent lock ordering.
- If using multiple CVs, ensure no circular dependency in signaling.

### Checklist

```
  ┌─────────────────────────────────────────────────────────┐
  │  Synchronization Correctness Checklist                  │
  │                                                         │
  │  □ All shared state protected by a mutex                │
  │  □ cond_wait always inside a while loop                 │
  │  □ cond_wait called with mutex held                     │
  │  □ Separate CVs for separate conditions                 │
  │  □ Signal sent AFTER state change, BEFORE own wait      │
  │  □ broadcast used when multiple waiters need waking     │
  │  □ No path where everyone sleeps (deadlock)             │
  │  □ Tested with thread sanitizer (gcc -fsanitize=thread) │
  └─────────────────────────────────────────────────────────┘
```

---

## 9. Linux Implementation Details

### futex-Based Condition Variables

glibc implements `pthread_cond_wait` and `pthread_cond_signal` using **futexes**:

- `pthread_cond_wait`: Atomically releases the mutex and calls `futex(FUTEX_WAIT)` to sleep on the CV's internal futex word.
- `pthread_cond_signal`: Calls `futex(FUTEX_WAKE, 1)` to wake one waiter.
- `pthread_cond_broadcast`: Calls `futex(FUTEX_WAKE, INT_MAX)` (or uses `FUTEX_REQUEUE` to move waiters to the mutex's futex — avoiding the "thundering herd" of all threads waking and contending on the mutex).

### FUTEX_REQUEUE Optimization

When broadcasting, naive implementation wakes all N threads, each of which tries to acquire the mutex — but only one succeeds, the rest go back to sleep on the mutex. This is wasteful.

`FUTEX_REQUEUE` (or `FUTEX_CMP_REQUEUE`) moves sleeping threads directly from the CV's wait queue to the mutex's wait queue **without waking them**. Only one thread is actually woken.

```
  Without FUTEX_REQUEUE (thundering herd):

  CV wait queue: [T1, T2, T3, T4]
  broadcast → wake ALL
  T1, T2, T3, T4 all wake up and contend on mutex
  T1 gets mutex; T2, T3, T4 go back to sleep on mutex
  (3 unnecessary wake-ups!)


  With FUTEX_REQUEUE:

  CV wait queue: [T1, T2, T3, T4]
  requeue → wake T1, move T2, T3, T4 to mutex wait queue
  Only T1 actually wakes up
  T2, T3, T4 never left the kernel — just moved queues
  (Efficient!)
```

### Kernel Equivalents

Inside the Linux kernel:

```c
#include <linux/wait.h>

// Declare a wait queue head:
DECLARE_WAIT_QUEUE_HEAD(my_wq);

// Sleep until condition is true:
wait_event(my_wq, condition);
wait_event_interruptible(my_wq, condition);  // can be interrupted by signals

// Wake up waiters:
wake_up(&my_wq);       // wake one
wake_up_all(&my_wq);   // wake all

// Kernel completion (one-shot event):
#include <linux/completion.h>
DECLARE_COMPLETION(my_completion);
wait_for_completion(&my_completion);
complete(&my_completion);       // wake one
complete_all(&my_completion);   // wake all
```

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Condition variable | Sleep until condition is true; be woken by another thread |
| `pthread_cond_wait` | Atomically: release mutex + sleep. On wake: re-acquire mutex |
| `pthread_cond_signal` | Wake one waiter |
| `pthread_cond_broadcast` | Wake all waiters |
| Missed wake-up | Signal arrives between condition check and sleep → lost forever. Fix: use mutex |
| Mesa semantics | Signaled thread may not run immediately; must recheck condition in `while` loop |
| Two CVs for producer-consumer | `not_empty` for consumers, `not_full` for producers. Prevents wrong-type wake-ups |
| Batched processing trap | Signal others BEFORE calling wait, or deadlock |
| `FUTEX_REQUEUE` | Kernel optimization: move waiters between queues without waking them |
