# Concurrent Network I/O

---

## 1. The Problem: One Server, Many Clients

A network server must handle **multiple clients simultaneously**. Consider a simple TCP server:

```c
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(listen_fd, &addr, sizeof(addr));
listen(listen_fd, SOMAXCONN);

while (1) {
    int conn_fd = accept(listen_fd, NULL, NULL);  // blocks until a client connects
    handle_client(conn_fd);                        // blocks while servicing this client
    close(conn_fd);
}
```

The problem is immediately visible: while `handle_client()` processes one client (which may involve `read()`/`recv()` calls that block waiting for data), the server is **unable to accept new connections** or service other existing clients.

```
  Single-Threaded Sequential Server:

  Time ──────────────────────────────────────────────────────────►

  Server:  accept(C1) ──── handle(C1) ──── accept(C2) ──── handle(C2)
                           ^^^^^^^^^^^^                     ^^^^^^^^^^^^
                           C2 is WAITING                    C3 is WAITING
                           to connect!                      to connect!

  Client 1: ──── connected ──── being served ──── done
  Client 2: ──── waiting... ──── waiting... ──── connected ──── served
  Client 3: ──── waiting... ──── waiting... ──── waiting... ────────────
```

### Multiple Sockets to Manage

A typical server has:
- **One listen socket**: Accepts new incoming connections.
- **N connected sockets**: One per active client, for reading/writing data.

Both `accept()` on the listen socket and `read()`/`recv()` on connected sockets are **blocking calls** — they don't return until data arrives. If the server blocks on one socket, all other sockets are neglected.

---

## 2. Solution: One Process/Thread per Client

The classic approach: the main server process accepts connections, then spawns a **child process** or **thread** for each client.

### Fork-Based (One Process per Client)

```c
int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(listen_fd, &addr, sizeof(addr));
listen(listen_fd, SOMAXCONN);

while (1) {
    int conn_fd = accept(listen_fd, NULL, NULL);  // main process blocks here

    pid_t pid = fork();
    if (pid == 0) {
        // Child process: handle this client
        close(listen_fd);          // child doesn't need the listen socket
        handle_client(conn_fd);    // can block on read/recv — only affects this child
        close(conn_fd);
        exit(0);
    }
    // Parent process: continue accepting
    close(conn_fd);  // parent doesn't need this client's socket
}
```

```
  Fork-Based Server:

  Main Process:  accept(C1) → fork → accept(C2) → fork → accept(C3) → ...
                      │                   │                   │
                      ▼                   ▼                   ▼
                 Child P1             Child P2            Child P3
                 handle(C1)           handle(C2)          handle(C3)
                 (independent)        (independent)       (independent)

  Each child blocks independently on its client's socket.
  Main process is always ready to accept new connections.
```

### Thread-Based (One Thread per Client)

```c
void *client_handler(void *arg) {
    int conn_fd = *(int *)arg;
    free(arg);
    handle_client(conn_fd);
    close(conn_fd);
    return NULL;
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listen_fd, &addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);

    while (1) {
        int *conn_fd = malloc(sizeof(int));
        *conn_fd = accept(listen_fd, NULL, NULL);

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, conn_fd);
        pthread_detach(tid);  // auto-cleanup when thread exits
    }
}
```

### Process vs. Thread per Client

| Aspect | Process per Client (`fork`) | Thread per Client |
|--------|---------------------------|-------------------|
| Isolation | Strong — separate address spaces. One crash doesn't affect others | Weak — shared address space. Bug in one thread can corrupt everything |
| Resource overhead | Heavy — each process has its own page table, address space (even with COW) | Lighter — shared address space, just a new stack + kernel thread |
| Communication | IPC required (pipes, shared memory) | Shared memory (easy but needs synchronization) |
| Scalability limit | Hundreds to low thousands (process table, memory) | Thousands (but stack memory, kernel threads) |
| Common usage | Legacy (Apache prefork MPM) | Common (many web servers, application servers) |

### The Scaling Problem

Both approaches hit a wall with large numbers of clients:

```
  10,000 clients → 10,000 processes/threads:

  Memory:    10,000 × ~8MB stack = 80 GB of virtual memory (stacks alone)
  Scheduling: OS scheduler managing 10,000+ runnable entities
  Context switches: Frequent switches between thousands of threads
  Kernel resources: Thread stacks, task_struct, file descriptors

  This is the "C10K problem" — handling 10,000 concurrent connections.
  Thread-per-connection doesn't scale beyond a few thousand.
```

```bash
# View system limits:
$ ulimit -u          # max user processes
$ cat /proc/sys/kernel/threads-max    # system-wide thread limit
$ cat /proc/sys/kernel/pid_max        # max PID value
```

The solution to the C10K problem is **event-driven I/O**, covered in the next note.

---

## 3. Practical Considerations

### Handling Zombie Processes (Fork-Based)

When a child process exits, it becomes a **zombie** until the parent calls `wait()`/`waitpid()`. With many clients, zombies accumulate. Solutions:

```c
// Option 1: Signal handler to reap children
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;  // reap all finished children
}
signal(SIGCHLD, sigchld_handler);

// Option 2: Double fork (child forks grandchild and exits immediately)
// Grandchild is adopted by init, which reaps it automatically.
```

### Thread Pool Pattern

Instead of creating a new thread per connection (expensive at high rates), pre-create a fixed pool of worker threads:

```c
// Simplified thread pool:
#define POOL_SIZE 64

int conn_queue[QUEUE_MAX];  // bounded buffer of accepted connections
// ... (protected by mutex + CVs, producer-consumer pattern)

void *worker(void *arg) {
    while (1) {
        int conn_fd = dequeue_connection();  // blocks if queue empty
        handle_client(conn_fd);
        close(conn_fd);
    }
}

int main() {
    // Pre-create worker threads
    for (int i = 0; i < POOL_SIZE; i++)
        pthread_create(&threads[i], NULL, worker, NULL);

    // Main thread: accept and enqueue
    while (1) {
        int conn_fd = accept(listen_fd, NULL, NULL);
        enqueue_connection(conn_fd);  // wake a worker
    }
}
```

```
  Thread Pool Architecture:

  Main Thread:   accept() ──► enqueue(fd) ──► accept() ──► ...
                                  │
                                  ▼
                         ┌────────────────┐
                         │ Bounded Buffer │
                         │ (connection    │
                         │  queue)        │
                         └───────┬────────┘
                                 │
                    ┌────────────┼────────────┐
                    ▼            ▼            ▼
               Worker 0    Worker 1    Worker 2   ...
               dequeue()   dequeue()   dequeue()
               handle()    handle()    handle()
               (blocks on  (blocks on  (blocks on
                this fd)    this fd)    this fd)
```

This is the **producer-consumer pattern** from concurrency — the main thread produces connections, worker threads consume them. This bounds thread count to POOL_SIZE regardless of client count.

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Problem | Blocking on one socket neglects all others |
| Process per client | `fork()` after `accept()`. Strong isolation, heavy overhead |
| Thread per client | `pthread_create` after `accept()`. Lighter, shared memory, needs synchronization |
| C10K problem | Thread-per-connection doesn't scale past thousands of clients |
| Thread pool | Fixed number of workers + connection queue. Bounds resource usage |
| Zombie processes | Fork-based servers must reap children (`SIGCHLD` handler or double-fork) |
