# Semaphores

---

## 1. What Is a Semaphore?

A **semaphore** is a synchronization primitive built around an integer counter with two atomic operations. Invented by Edsger Dijkstra (1965), it is a more general mechanism than locks or condition variables — it can implement both mutual exclusion and ordering/signaling.

### Core Operations

```c
#include <semaphore.h>

sem_t sem;
sem_init(&sem, 0, initial_value);  // 0 = shared between threads (not processes)

sem_wait(&sem);   // P() / down() / decrement
// Atomically:
//   if (sem > 0) { sem--; return; }
//   else { sleep until sem > 0, then sem--; return; }

sem_post(&sem);   // V() / up() / increment
// Atomically:
//   sem++;
//   if any thread is sleeping on this semaphore, wake one up.
```

```
  Semaphore State Transitions:

  sem_init(sem, 0, N)  →  sem = N

  sem_wait():
    ┌───────────────┐
    │ sem > 0 ?     │── yes ──► sem--; return (no blocking)
    │               │
    │               │── no ───► SLEEP until another thread
    │               │           calls sem_post()
    └───────────────┘

  sem_post():
    sem++;
    if (sleeping threads exist) → wake one up
```

Key properties:
- The semaphore value is **never negative** from the user's perspective (internally, some implementations track negative values to count waiters, but `sem_getvalue()` returns max(0, value)).
- `sem_post` **never blocks**. It always increments and returns.
- `sem_wait` blocks only when the value is 0.

### Historical Naming

| Modern (POSIX) | Dijkstra's Original | Meaning |
|----------------|---------------------|---------|
| `sem_wait()` | `P()` (proberen = "to try") | Try to decrement; sleep if 0 |
| `sem_post()` | `V()` (verhogen = "to increment") | Increment; wake a sleeper |

---

## 2. Binary Semaphore as a Lock

A semaphore initialized to **1** behaves like a lock (mutex):

```c
sem_t mutex;
sem_init(&mutex, 0, 1);  // initial value = 1

sem_wait(&mutex);    // acquire: 1 → 0 (proceeds) or 0 → sleep
// ── critical section ──
counter++;
// ── end critical section ──
sem_post(&mutex);    // release: 0 → 1, wake a sleeper if any
```

Trace with two threads:

```
  Thread 1                        Thread 2
  ────────                        ────────
  sem_wait(mutex): val 1→0
  (enters critical section)
                                  sem_wait(mutex): val is 0 → SLEEP
  counter++ 
  sem_post(mutex): val 0→1, wake T2
                                  (woken up, val 1→0)
                                  counter++
                                  sem_post(mutex): val 0→1
```

This is called a **binary semaphore** — it only takes values 0 and 1, acting as a lock.

### Semaphore vs. Mutex: Key Differences

| Aspect | Mutex (pthread_mutex) | Binary Semaphore |
|--------|----------------------|-----------------|
| Ownership | Has an owner. Only the thread that locked it can unlock it | No ownership. Any thread can call sem_post |
| Error detection | Attempting to unlock a mutex you don't own is undefined behavior (or returns error) | sem_post from any thread is always valid |
| Priority inheritance | Supported on some systems (avoids priority inversion) | Not applicable (no owner) |
| Recursive locking | Possible with PTHREAD_MUTEX_RECURSIVE | Not possible (would deadlock) |

---

## 3. Semaphore for Ordering / Signaling

A semaphore initialized to **0** can enforce ordering: "Thread B must wait until Thread A reaches a certain point."

### Example: T1 → T2 Ordering

Thread 2 must execute `step_B()` only after Thread 1 has completed `step_A()`.

```c
sem_t sem;
sem_init(&sem, 0, 0);  // initial value = 0

// Thread 1:                      // Thread 2:
step_A();                         sem_wait(&sem);  // blocks (val is 0)
sem_post(&sem);                   // woken up after T1 posts
                                  step_B();        // guaranteed after step_A
```

If Thread 2 runs first, it calls `sem_wait` and sleeps (value is 0). When Thread 1 eventually calls `sem_post`, Thread 2 wakes up.

If Thread 1 runs first, it calls `sem_post` (value becomes 1). When Thread 2 later calls `sem_wait`, it decrements to 0 and proceeds immediately. No missed wake-up — the semaphore **remembers** the signal (unlike raw condition variables where a signal is lost if no one is waiting).

### Example: Mutual Barrier (T1 and T2 Wait for Each Other)

Both threads must complete Phase 1 before either proceeds to Phase 2.

```c
sem_t s1, s2;
sem_init(&s1, 0, 0);
sem_init(&s2, 0, 0);

// Thread 1:                      // Thread 2:
phase1_T1();                      phase1_T2();
sem_post(&s1);   // signal: T1 done    sem_post(&s2);   // signal: T2 done
sem_wait(&s2);   // wait for T2        sem_wait(&s1);   // wait for T1
phase2_T1();                      phase2_T2();
```

```
  Mutual Barrier — Execution Timeline:

  Thread 1          Thread 2
  ────────          ────────
  phase1_T1()       phase1_T2()
  post(s1)          post(s2)         ← both signal they're done
  wait(s2) ◄──────── (s2=1, proceed)
                    wait(s1) ◄────── (s1=1, proceed)
  phase2_T1()       phase2_T2()

  Both phase1 functions complete before either phase2 begins.
```

### Deadlock Trap: Wrong Order

```c
// DEADLOCK!
// Thread 1:                      // Thread 2:
phase1_T1();                      phase1_T2();
sem_wait(&s2);   // wait for T2   sem_wait(&s1);   // wait for T1
sem_post(&s1);                    sem_post(&s2);
```

If both threads reach `sem_wait` before either calls `sem_post`:
- T1 is waiting on s2 (which is 0). T2 is waiting on s1 (which is 0).
- Neither can post because both are blocked. **Deadlock.**

**Rule**: Post (signal) before wait when implementing mutual barriers.

---

## 4. Producer-Consumer with Semaphores

### Recap: The Problem

Bounded buffer of size N. Producers add items, consumers remove items. Producers must block when full, consumers must block when empty.

### Solution: Two Counting Semaphores + One Binary Semaphore

```c
#define N 10

sem_t sem_empty;   // counts empty slots (producers wait on this)
sem_t sem_filled;  // counts filled slots (consumers wait on this)
sem_t mutex;       // binary semaphore for mutual exclusion on buffer

int buffer[N];
int fill_ptr = 0, use_ptr = 0;

void init() {
    sem_init(&sem_empty, 0, N);   // N empty slots initially
    sem_init(&sem_filled, 0, 0);  // 0 filled slots initially
    sem_init(&mutex, 0, 1);       // mutex
}

void *producer(void *arg) {
    for (int i = 0; i < loops; i++) {
        sem_wait(&sem_empty);     // wait for an empty slot (decrement empty)
        sem_wait(&mutex);         // enter critical section
        buffer[fill_ptr] = i;
        fill_ptr = (fill_ptr + 1) % N;
        sem_post(&mutex);         // leave critical section
        sem_post(&sem_filled);    // signal: one more filled slot
    }
}

void *consumer(void *arg) {
    for (int i = 0; i < loops; i++) {
        sem_wait(&sem_filled);    // wait for a filled slot (decrement filled)
        sem_wait(&mutex);         // enter critical section
        int val = buffer[use_ptr];
        use_ptr = (use_ptr + 1) % N;
        sem_post(&mutex);         // leave critical section
        sem_post(&sem_empty);     // signal: one more empty slot
        process(val);
    }
}
```

### How It Works

```
  Initial state: sem_empty = N, sem_filled = 0

  Producer produces an item:
    sem_wait(sem_empty):  N → N-1   (one fewer empty slot)
    [lock mutex, put item, unlock mutex]
    sem_post(sem_filled): 0 → 1    (one more filled slot)

  Consumer consumes an item:
    sem_wait(sem_filled): 1 → 0    (one fewer filled slot)
    [lock mutex, get item, unlock mutex]
    sem_post(sem_empty):  N-1 → N  (one more empty slot)

  When buffer is full (sem_empty = 0):
    Producer calls sem_wait(sem_empty) → BLOCKS
    Woken up when a consumer calls sem_post(sem_empty)

  When buffer is empty (sem_filled = 0):
    Consumer calls sem_wait(sem_filled) → BLOCKS
    Woken up when a producer calls sem_post(sem_filled)
```

### Why This Is Simpler Than CVs

With condition variables, you need:
- A mutex to protect the shared state.
- A `count` variable to track items in the buffer.
- Two CVs (`not_empty`, `not_full`).
- `while` loops to recheck conditions after wake-up.

With semaphores:
- `sem_empty` and `sem_filled` **are** the counters. No separate `count` variable.
- No `while` loop needed — when `sem_wait` returns, the condition is guaranteed true (the semaphore was > 0).
- Ordering is managed by the semaphore values themselves.

### Critical: Order of sem_wait Matters!

```c
// CORRECT order:
sem_wait(&sem_empty);     // 1. wait on counting semaphore
sem_wait(&mutex);         // 2. then acquire mutex

// DEADLOCK-PRONE order:
sem_wait(&mutex);         // 1. acquire mutex FIRST
sem_wait(&sem_empty);     // 2. then wait on counting semaphore
// If buffer is full:
//   - Producer holds mutex
//   - Producer sleeps on sem_empty
//   - Consumer needs mutex to consume and post(sem_empty)
//   - Consumer can't get mutex → DEADLOCK!
```

Always: **wait on the counting semaphore first, then the mutex.**

---

## 5. Semaphore Deadlock: No Auto-Release of Held Locks

This is a crucial difference between semaphores and condition variables:

### With Condition Variables

```c
pthread_mutex_lock(&mutex);
while (!condition)
    pthread_cond_wait(&cond, &mutex);
    // ^^^ Atomically: releases mutex AND sleeps.
    //     Other threads can acquire the mutex while we sleep.
    //     When woken, mutex is re-acquired before returning.
```

The mutex is **automatically released** when the thread sleeps. This is by design — it lets other threads make progress and eventually signal.

### With Semaphores

```c
sem_wait(&mutex);       // acquire binary semaphore (acting as lock)
sem_wait(&sem_filled);  // if sem_filled == 0, we SLEEP...
                        // ...but we're STILL holding 'mutex'!
                        // No auto-release!
```

When a thread calls `sem_wait` and blocks, it does **not** release any other semaphores it holds. If another thread needs one of those semaphores to make progress and eventually `sem_post` the semaphore we're waiting on — deadlock.

```
  Deadlock from Holding Lock While Waiting on Semaphore:

  Consumer:
    sem_wait(mutex)    → acquired (mutex = 0)
    sem_wait(sem_filled) → sem_filled = 0, SLEEP!
    ^^^ Still holding mutex! Nobody else can acquire it!

  Producer:
    sem_wait(mutex) → mutex = 0, BLOCKS!
    ^^^ Needs mutex to produce and post(sem_filled)
    ^^^ Can't get it because consumer holds it while sleeping!

  DEADLOCK: Consumer holds mutex, waits on sem_filled.
            Producer waits on mutex, would post sem_filled.
```

**This is why the correct producer-consumer uses `sem_wait(sem_empty)` BEFORE `sem_wait(mutex)` — the counting semaphore wait happens outside the mutex.**

---

## 6. Batched Processing with Semaphores

The batched processing problem from the CV notes can also be solved with semaphores:

```c
#define BATCH_SIZE 4

sem_t request_sem;   // batch processor waits on this
sem_t done_sem;      // request threads wait on this

int request_count = 0;
sem_t mutex;         // protects request_count

void init() {
    sem_init(&request_sem, 0, 0);  // batch processor sleeps until signaled
    sem_init(&done_sem, 0, 0);     // requests sleep until batch is done
    sem_init(&mutex, 0, 1);
}

void *request_thread(void *arg) {
    sem_wait(&mutex);
    request_count++;
    int is_nth = (request_count == BATCH_SIZE);
    sem_post(&mutex);

    if (is_nth) {
        // I'm the Nth request — wake up batch processor
        sem_post(&request_sem);
    }

    // Wait for batch to complete
    sem_wait(&done_sem);

    // ... proceed with response ...
}

void *batch_processor(void *arg) {
    // Wait until Nth request signals us
    sem_wait(&request_sem);

    // Process the batch
    process_batch();

    // Wake up ALL N request threads
    for (int i = 0; i < BATCH_SIZE; i++)
        sem_post(&done_sem);
}
```

Notice: the semaphore `done_sem` is posted N times to wake N threads. Each `sem_post` increments the counter by 1 and wakes one waiter. This is the semaphore equivalent of `pthread_cond_broadcast`.

---

## 7. Guidelines for Using Semaphores

1. **Know the initial value.** The initial value determines behavior:
   - **1**: Binary semaphore (lock). Mutual exclusion.
   - **0**: Signaling / ordering. Wait for an event.
   - **N**: Counting semaphore. Allow up to N concurrent accesses.

2. **sem_wait on counting semaphores before mutex.** Never hold the mutex while waiting on a counting semaphore — risk of deadlock.

3. **Remember: no auto-release.** Unlike `pthread_cond_wait`, `sem_wait` does NOT release any held locks. Plan accordingly.

4. **sem_post never blocks.** You can call it safely from anywhere, including signal handlers (it's async-signal-safe).

5. **No ownership.** Any thread can `sem_post` — use this intentionally for signaling patterns, but be careful if you need ownership semantics (use a real mutex instead).

6. **Prefer mutexes + CVs for complex conditions.** Semaphores are elegant for simple counting and ordering, but for complex predicates (e.g., "wait until count > 5 AND flag == true"), condition variables with explicit conditions are clearer.

```
  When to Use What:

  ┌──────────────────────────────────────────────┐
  │                                              │
  │  Mutual exclusion?                           │
  │    → pthread_mutex (preferred)               │
  │    → Binary semaphore (works but no          │
  │      ownership, no priority inheritance)     │
  │                                              │
  │  Wait for a condition?                       │
  │    → Condition variable + mutex              │
  │      (flexible, explicit condition)          │
  │    → Semaphore                               │
  │      (simpler for counting/ordering)         │
  │                                              │
  │  Limit concurrent access to N?               │
  │    → Counting semaphore (init to N)          │
  │                                              │
  │  Signal between threads (fire-and-forget)?   │
  │    → Semaphore (init to 0)                   │
  │      The signal is remembered even if no     │
  │      one is waiting yet.                     │
  │                                              │
  └──────────────────────────────────────────────┘
```

---

## 8. Linux Implementation Details

### POSIX Semaphores

Two types of POSIX semaphores:

| Type | API | Scope |
|------|-----|-------|
| **Unnamed** | `sem_init()` / `sem_destroy()` | Threads (or processes if in shared memory) |
| **Named** | `sem_open()` / `sem_close()` / `sem_unlink()` | Across unrelated processes, backed by filesystem (`/dev/shm/sem.*`) |

```c
// Named semaphore (inter-process):
sem_t *sem = sem_open("/my_semaphore", O_CREAT, 0644, 1);
sem_wait(sem);
// ... critical section ...
sem_post(sem);
sem_close(sem);
sem_unlink("/my_semaphore");  // remove from filesystem

// Under the hood: creates /dev/shm/sem.my_semaphore
```

### Kernel Implementation

glibc's unnamed semaphores use **futex** internally:

- `sem_wait`: Attempts atomic decrement. If value was > 0, succeeds without syscall (fast path). If value is 0, calls `futex(FUTEX_WAIT)` to sleep.
- `sem_post`: Atomic increment. If there were waiters, calls `futex(FUTEX_WAKE, 1)` to wake one.

```
  sem_wait fast path (no contention):

  User space only — no syscall!
  atomic_decrement_if_positive(&sem->value)
    → success → return

  sem_wait slow path (contention):

  value is 0 → syscall(SYS_futex, &sem->value, FUTEX_WAIT, 0)
               → kernel puts thread to sleep
               → woken up when sem_post calls FUTEX_WAKE
```

### System V Semaphores (Legacy)

Older System V IPC semaphores (`semget`, `semop`, `semctl`) still exist but are heavier and more complex. POSIX semaphores are preferred for new code.

```c
// System V (legacy, avoid for new code):
int semid = semget(key, 1, IPC_CREAT | 0666);
struct sembuf op = {0, -1, 0};  // decrement sem 0 by 1
semop(semid, &op, 1);
```

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Semaphore | Integer counter with atomic `wait` (decrement/sleep) and `post` (increment/wake) |
| Binary semaphore (init=1) | Acts as a lock. Mutual exclusion |
| Signaling semaphore (init=0) | Ordering: wait for an event. Signal is remembered |
| Counting semaphore (init=N) | Allow up to N concurrent accesses |
| Producer-consumer | `sem_empty`=N, `sem_filled`=0, `mutex`=1. Wait counting sem before mutex |
| No auto-release | `sem_wait` does NOT release held locks — unlike `cond_wait` |
| Deadlock risk | Holding mutex while waiting on counting semaphore → deadlock |
| Named semaphores | Cross-process synchronization via `/dev/shm/sem.*` |
| Implementation | futex-based: fast path atomic in user space, slow path kernel sleep/wake |
