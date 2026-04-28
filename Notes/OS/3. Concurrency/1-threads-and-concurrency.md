# Introduction to Threads and Concurrency

---

## 1. Concurrency vs. Parallelism

These two terms are often confused but describe fundamentally different things:

**Concurrency**: Multiple tasks make progress over overlapping time periods. They may not be executing at the exact same instant — they *interleave* on a single CPU core via context switching.

**Parallelism**: Multiple tasks execute at the **exact same instant** on different CPU cores or processors. True simultaneous execution.

```
  Concurrency (single core — interleaved):

  Time ───────────────────────────────────────────►
  CPU:  ┌──T1──┐┌──T2──┐┌──T1──┐┌──T2──┐┌──T1──┐
        │ work ││ work ││ work ││ work ││ work │
        └──────┘└──────┘└──────┘└──────┘└──────┘
        Both T1 and T2 make progress, but never run at the same time.
        The OS context-switches between them.


  Parallelism (multi-core — simultaneous):

  Time ───────────────────────────────────────────►
  Core0: ┌────────T1────────┐┌────────T1────────┐
         │       work       ││       work       │
         └──────────────────┘└──────────────────┘
  Core1: ┌────────T2────────┐┌────────T2────────┐
         │       work       ││       work       │
         └──────────────────┘└──────────────────┘
         T1 and T2 truly execute at the same instant.
```

| Aspect | Concurrency | Parallelism |
|--------|------------|-------------|
| Definition | Dealing with multiple things at once (structure) | Doing multiple things at once (execution) |
| Hardware | Possible on single core | Requires multiple cores |
| Example | OS scheduling multiple processes | Matrix multiply across 8 cores |
| Analogy | One cook switching between multiple dishes | Eight cooks each making a dish |

A system can be:
- **Concurrent but not parallel**: Single core, multiple threads interleaving.
- **Parallel but not concurrent**: (Rare) Multiple cores each running exactly one task that never interleaves with others.
- **Both concurrent and parallel**: Multiple cores, each running multiple interleaving threads.
- **Neither**: Single core, single sequential program.

Modern systems are typically both concurrent and parallel.

---

## 2. Why Threads?

A **process** is the unit of resource ownership (address space, open files, etc.). A **thread** is the unit of execution (CPU context: PC, registers, stack) within a process.

### Single-Threaded vs. Multi-Threaded Process

```
  Single-threaded process:           Multi-threaded process:

  ┌────────────────────────┐         ┌────────────────────────┐
  │  Address Space          │         │  Address Space          │
  │                        │         │  (SHARED by all threads)│
  │  ┌──────┐              │         │  ┌──────┐              │
  │  │ Code │              │         │  │ Code │              │
  │  ├──────┤              │         │  ├──────┤              │
  │  │ Data │              │         │  │ Data │ (shared)     │
  │  │(globals,heap)       │         │  │(globals,heap)       │
  │  ├──────┤              │         │  ├──────┤──────┬──────┐│
  │  │Stack │              │         │  │Stack │Stack │Stack ││
  │  │      │              │         │  │ T0   │ T1   │ T2   ││
  │  └──────┘              │         │  └──────┘──────┘──────┘│
  │                        │         │                        │
  │  1 set of registers    │         │  3 sets of registers   │
  │  1 PC                  │         │  3 PCs                 │
  └────────────────────────┘         └────────────────────────┘
  One thread of execution.           Three threads of execution,
                                     sharing code, data, heap,
                                     open files, etc.
```

### What Threads Share and What They Don't

| Shared (per-process) | Private (per-thread) |
|---------------------|---------------------|
| Address space (code, data, heap) | Stack |
| Open file descriptors | Registers (PC, SP, general-purpose) |
| Signal handlers | Thread ID (TID) |
| PID, PPID | `errno` (in glibc, stored in thread-local storage) |
| Working directory, umask | Signal mask |
| Memory mappings | Thread-local storage (TLS) |

### Why Use Threads Instead of Processes?

1. **Shared address space**: Threads can share data trivially (via global variables, heap). Processes must use IPC (pipes, shared memory, sockets) — more complex and often slower.
2. **Lightweight creation**: Creating a thread is cheaper than creating a process — no need to duplicate the address space (even with COW, there's page table setup overhead).
3. **Lightweight switching**: Switching between threads in the same process doesn't require changing the page table (CR3). TLB entries remain valid.
4. **Parallelism for compute-bound tasks**: Split a CPU-intensive task across threads to use multiple cores.
5. **Responsiveness**: Keep the UI responsive while a background thread does heavy computation or I/O.

### When to Prefer Processes?

- **Isolation**: A bug in one process can't corrupt another's memory. A crashing thread takes down the entire process.
- **Security**: Separate processes have separate privileges and can be sandboxed (e.g., Chrome's site isolation).
- **Different programs**: `fork + exec` to run a different program.

---

## 3. POSIX Threads (pthreads)

POSIX defines a standard threading API: **pthreads** (`<pthread.h>`). This is the standard threading interface on Linux, macOS, and most UNIX systems.

### Thread Creation

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*start_routine)(void *), void *arg);
```

| Parameter | Description |
|-----------|-------------|
| `thread` | Output: stores the new thread's ID |
| `attr` | Thread attributes (stack size, scheduling, detach state). NULL for defaults. |
| `start_routine` | Function the thread will execute. Signature: `void *func(void *arg)` |
| `arg` | Argument passed to `start_routine` |

Returns 0 on success, error code on failure.

### Thread Termination and Joining

```c
// Thread exits by:
// 1. Returning from start_routine
// 2. Calling pthread_exit()
void pthread_exit(void *retval);

// Parent thread waits for a child thread to finish:
int pthread_join(pthread_t thread, void **retval);
// Blocks until 'thread' terminates. Retrieves its return value.
// Analogous to waitpid() for processes.
```

### Complete Example

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *worker(void *arg) {
    int id = *(int *)arg;
    printf("Thread %d: starting work\n", id);
    // ... do some work ...
    printf("Thread %d: done\n", id);
    return NULL;
}

int main() {
    pthread_t threads[4];
    int ids[4];

    // Create 4 threads
    for (int i = 0; i < 4; i++) {
        ids[i] = i;
        int rc = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads done.\n");
    return 0;
}
```

Compile with:
```bash
$ gcc -pthread thread_example.c -o thread_example
$ ./thread_example
```

### Detached Threads

By default, threads are **joinable** — another thread must call `pthread_join` to reclaim their resources (analogous to wait/waitpid for zombies). A **detached** thread cleans up automatically when it exits — no join needed (but you can't retrieve its return value).

```c
pthread_detach(thread);
// or set attribute before creation:
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
pthread_create(&thread, &attr, func, arg);
```

### Linux Implementation: `clone()` with `CLONE_THREAD`

On Linux, threads are implemented via the `clone()` syscall with sharing flags:

```c
// What pthread_create() does internally (simplified):
clone(thread_func, child_stack,
      CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
      CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS |
      CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
      arg);
```

Key flags:
- `CLONE_VM`: Share address space (same `mm_struct`).
- `CLONE_FILES`: Share file descriptor table.
- `CLONE_THREAD`: Same thread group (same PID from userspace perspective, different TIDs).
- `CLONE_SIGHAND`: Share signal handlers.

In the kernel, threads are **just processes that share resources**. Each thread has its own `task_struct`, its own kernel stack, and its own TID. But they share `mm_struct`, `files_struct`, `signal_struct`, etc.

```bash
# View threads of a process:
$ ps -eLf | grep my_program     # -L shows threads (LWP column = TID)
$ ls /proc/<pid>/task/          # each subdirectory is a thread (by TID)
$ top -H -p <pid>               # show threads for a process
```

---

## 4. Thread Scheduling Overview

Since threads in Linux are `task_struct` objects like processes, they are scheduled by the **same scheduler** (CFS/EEVDF). Each thread is a schedulable entity.

```
  4 threads of Process A on a 2-core system:

  Core 0:  ┌──T0──┐┌──T2──┐┌──T0──┐┌──T3──┐
           └──────┘└──────┘└──────┘└──────┘

  Core 1:  ┌──T1──┐┌──T3──┐┌──T1──┐┌──T2──┐
           └──────┘└──────┘└──────┘└──────┘

  All 4 threads compete for CPU time.
  The scheduler can run them on any core.
  Switching between threads in the same process is cheaper
  (no page table switch needed, same mm_struct).
```

### User-Level Threads vs. Kernel-Level Threads

| Model | Description | Pros | Cons |
|-------|-------------|------|------|
| **Kernel-level (1:1)** | Each user thread maps to one kernel task_struct. Kernel schedules all threads. | True parallelism; blocking one thread doesn't block others | Thread creation involves syscall; heavier |
| **User-level (N:1)** | Multiple user threads multiplexed on one kernel thread by a user-space library | Lightweight creation; no syscall needed | One blocking syscall blocks ALL threads; no parallelism |
| **Hybrid (M:N)** | M user threads multiplexed on N kernel threads | Flexible; can balance overhead and parallelism | Complex to implement; scheduling interactions |

**Linux uses the 1:1 model** — every pthread maps to one kernel-level schedulable entity (task_struct). This is the NPTL (Native POSIX Threads Library) model, the default in modern glibc.

---

## 5. Shared Data Access

When multiple threads share memory (global variables, heap data), simultaneous access can lead to trouble.

### Example: Shared Counter

```c
int counter = 0;  // global shared variable

void *increment(void *arg) {
    for (int i = 0; i < 1000000; i++) {
        counter++;  // NOT ATOMIC!
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %d\n", counter);  // Expected: 2000000. Actual: ???
    return 0;
}
```

Running this will almost certainly print a number **less than 2,000,000** — often significantly less. Why?

### The Problem: `counter++` Is Not Atomic

`counter++` compiles to multiple machine instructions:

```asm
mov eax, [counter]    ; 1. LOAD: read counter from memory into register
add eax, 1            ; 2. ADD:  increment the register
mov [counter], eax    ; 3. STORE: write the result back to memory
```

If two threads interleave these instructions:

```
  Thread 1                    Thread 2
  ──────────                  ──────────
  LOAD counter → eax1 = 0
                              LOAD counter → eax2 = 0
  ADD: eax1 = 1
                              ADD: eax2 = 1
  STORE: counter = 1
                              STORE: counter = 1

  Both incremented, but counter is 1, not 2!
  One increment was LOST — this is a "lost update."
```

This can happen because:
- **Preemption**: The timer interrupt fires between the LOAD and STORE of one thread, and the scheduler switches to the other thread.
- **True parallelism**: On multi-core systems, both threads execute the LOAD simultaneously on different cores, both reading the same old value.

---

## 6. Race Conditions, Critical Sections, and Mutual Exclusion

### Race Condition

A **race condition** occurs when the program's outcome depends on the **relative timing** (interleaving order) of operations in different threads. The result is non-deterministic — the program may produce different results on different runs.

### Critical Section

A **critical section** is a piece of code that accesses shared data and must not be executed by more than one thread at a time.

```c
// Critical section:
counter++;   // Reads AND writes shared variable 'counter'.
             // If two threads execute this simultaneously, race condition!
```

### Mutual Exclusion

**Mutual exclusion** ensures that only one thread at a time can execute the critical section. This is the fundamental requirement for correctness in concurrent programs.

```
  Without mutual exclusion:        With mutual exclusion:

  T1: ─── CRITICAL ───             T1: ─── CRITICAL ───
  T2: ─── CRITICAL ───             T2: ──────────────── CRITICAL ────
       (overlapping = race!)              (serialized = safe)
```

### Requirements for a Correct Mutual Exclusion Solution

1. **Mutual exclusion**: At most one thread is in the critical section at any time.
2. **Progress (no deadlock)**: If no thread is in the critical section and some threads want to enter, one of them must eventually succeed. The system cannot get stuck with everyone waiting forever.
3. **Bounded waiting (no starvation)**: A thread that wants to enter the critical section must eventually get in. It shouldn't wait indefinitely while others keep entering.

The primary mechanism for achieving mutual exclusion is the **lock** (mutex), covered in the next note.

---

## 7. The Need for Synchronization — Beyond Mutual Exclusion

Mutual exclusion (locks) solves the problem of protecting shared data. But concurrent programs also need:

- **Ordering / coordination**: Thread A must complete step X before thread B proceeds to step Y. (Solved by **condition variables**, **semaphores**.)
- **Signaling**: Thread A must wake up thread B when an event occurs. (Solved by **condition variables**.)
- **Resource counting**: At most N threads can access a resource simultaneously. (Solved by **counting semaphores**.)

```
  Synchronization Primitives:

  ┌─────────────────────────────────────────────────────┐
  │                                                     │
  │  LOCKS (Mutexes)                                    │
  │  → Mutual exclusion: only one thread at a time      │
  │                                                     │
  │  CONDITION VARIABLES                                │
  │  → Wait/signal: thread sleeps until a condition     │
  │    is true, another thread signals it               │
  │                                                     │
  │  SEMAPHORES                                         │
  │  → Generalized lock: count-based. Subsumes both     │
  │    mutual exclusion (binary) and signaling (counting)│
  │                                                     │
  └─────────────────────────────────────────────────────┘
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| Concurrency vs. Parallelism | Concurrency = interleaving; Parallelism = simultaneous execution | Both on modern multi-core Linux |
| Thread | Unit of execution; shares address space with other threads in same process | `task_struct` with `CLONE_VM`, `CLONE_THREAD` |
| pthreads | POSIX threading API: `pthread_create`, `pthread_join`, `pthread_exit` | NPTL (1:1 model), glibc's pthread implementation |
| Thread scheduling | Threads are scheduled like processes by the same kernel scheduler | CFS/EEVDF, per-CPU run queues |
| Shared data | Threads share heap, globals → concurrent access is unsafe | Shared `mm_struct` |
| Race condition | Non-deterministic outcome due to uncontrolled thread interleaving | Common bug pattern |
| Critical section | Code accessing shared data that needs mutual exclusion | Protected by locks/mutexes |
