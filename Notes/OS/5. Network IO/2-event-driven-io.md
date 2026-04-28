# Event-Driven I/O

---

## 1. The Idea: Multiplex I/O in a Single Thread

Instead of one thread per connection (which doesn't scale), use a **single thread** (or a few threads) to monitor **many** file descriptors simultaneously. The kernel tells the process which sockets are ready, and the process handles only the ready ones — no blocking.

```
  Thread-per-Connection:              Event-Driven:

  ┌──────┐ ┌──────┐ ┌──────┐         ┌──────────────────────┐
  │ T1   │ │ T2   │ │ T3   │         │  Single Thread       │
  │handle││handle ││handle │         │                      │
  │ C1   │ │ C2   │ │ C3   │         │  epoll_wait() →      │
  │(block)│(block)│(block) │         │  C1 ready? handle C1 │
  └──────┘ └──────┘ └──────┘         │  C3 ready? handle C3 │
  3 threads, 3 clients              │  loop back to wait   │
  10K clients = 10K threads!         │                      │
                                     └──────────────────────┘
                                     1 thread, ANY # of clients
```

This is **event-driven** (or **asynchronous**) I/O — the program is structured around events ("socket X is readable", "socket Y is writable") rather than blocking operations.

---

## 2. I/O Multiplexing APIs: `select`, `poll`, `epoll`

Linux provides three APIs for I/O multiplexing, each an evolution of the previous:

### 2.1. `select()` (POSIX, oldest)

```c
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);
// Blocks until one or more fds in the sets are ready, or timeout expires.
// On return, the fd_sets are modified to indicate which fds are ready.
```

**Limitations**:
- `fd_set` is a fixed-size bitmask (typically 1024 bits — `FD_SETSIZE`). Can't monitor more than 1024 fds.
- Kernel must scan the **entire** set on every call — O(n) per call.
- The fd_sets are modified in-place — must rebuild them before every call.

### 2.2. `poll()` (POSIX, improved)

```c
#include <poll.h>

struct pollfd {
    int   fd;       // file descriptor
    short events;   // requested events (POLLIN, POLLOUT, ...)
    short revents;  // returned events (set by kernel)
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
// Blocks until one or more fds have events, or timeout.
```

**Improvements over select**: No hard-coded `FD_SETSIZE` limit; uses a dynamic array. Events and results are separate fields (no need to rebuild).

**Still O(n)**: Kernel scans the entire array on every call.

### 2.3. `epoll` (Linux-specific, scalable)

`epoll` is the **modern, scalable** I/O multiplexing API on Linux. It solves the O(n) problem.

```c
#include <sys/epoll.h>

// 1. Create an epoll instance
int epfd = epoll_create1(0);
// Returns a file descriptor for the epoll instance.

// 2. Register fds to monitor
struct epoll_event ev;
ev.events = EPOLLIN;       // interested in "readable" events
ev.data.fd = listen_fd;
epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
// Also add client fds as they're accepted.

// 3. Wait for events
struct epoll_event events[MAX_EVENTS];
int n = epoll_wait(epfd, events, MAX_EVENTS, timeout_ms);
// Blocks until at least one registered fd has an event.
// Returns only the READY fds — not the entire set.

// 4. Handle ready fds
for (int i = 0; i < n; i++) {
    if (events[i].data.fd == listen_fd) {
        // New connection — accept and add to epoll
        int conn_fd = accept(listen_fd, NULL, NULL);
        ev.events = EPOLLIN;
        ev.data.fd = conn_fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
    } else {
        // Client data ready — read and process
        handle_client_data(events[i].data.fd);
    }
}
```

---

## 3. Why `epoll` Scales

| API | Add/Remove fd | Wait for events | Returns |
|-----|--------------|----------------|---------|
| `select` | Rebuild fd_set every call — O(n) | Kernel scans all fds — O(n) | Modified bitmask — must scan to find ready fds |
| `poll` | Pass full array every call — O(n) | Kernel scans all fds — O(n) | Must scan `revents` — O(n) |
| `epoll` | `epoll_ctl` — O(1) per add/remove | Kernel maintains ready list — O(1) wait | Returns **only ready fds** — O(ready) |

```
  select/poll on every call:

  User → Kernel: "Here are ALL 10,000 fds. Which are ready?"
  Kernel: Scans all 10,000. "Fds 42 and 7891 are ready."
  Repeat every iteration — 10,000 fds scanned every time.

  epoll:

  Setup (once): "Monitor these 10,000 fds."
  epoll_wait: "Which are ready?"
  Kernel: Maintains a ready list internally.
          "Fds 42 and 7891 are ready." (returns just 2 items)
  Repeat: Kernel only wakes you with the ready fds.
  No scanning of all 10,000.
```

### How epoll Works Internally

```
  ┌──────────────────────────────────────────────────┐
  │  Kernel: epoll instance                          │
  │                                                  │
  │  ┌───────────────────────────────┐               │
  │  │  Interest List (red-black tree)│               │
  │  │  All monitored fds            │               │
  │  │  fd=3, fd=4, fd=5, ...       │               │
  │  └───────────────────────────────┘               │
  │                                                  │
  │  ┌───────────────────────────────┐               │
  │  │  Ready List (linked list)     │               │
  │  │  Fds with pending events      │               │
  │  │  fd=4, fd=7                  │               │
  │  └───────────────────────────────┘               │
  │                                                  │
  │  When data arrives on fd=4:                      │
  │  - NIC interrupt → kernel processes packet       │
  │  - Kernel adds fd=4 to ready list (callback)     │
  │  - epoll_wait returns with fd=4                  │
  └──────────────────────────────────────────────────┘
```

The kernel uses **callbacks** (registered when `epoll_ctl(ADD)` is called) so that when a socket becomes ready, it's added to the ready list automatically — no scanning needed.

### Level-Triggered vs. Edge-Triggered

| Mode | Behavior | Flag |
|------|----------|------|
| **Level-triggered** (default) | `epoll_wait` returns as long as the fd **is** ready. If you don't read all data, it will fire again on the next `epoll_wait`. | (default) |
| **Edge-triggered** | `epoll_wait` returns only when the fd **becomes** ready (state change). If you don't read all data, it won't fire again until new data arrives. | `EPOLLET` |

Edge-triggered is more efficient (fewer spurious wake-ups) but requires reading/writing until `EAGAIN` to avoid missing data. Non-blocking fds are typically used with edge-triggered mode.

---

## 4. Complete Event-Driven Server Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 1024
#define PORT 8080

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);
    set_nonblocking(listen_fd);

    // Create epoll instance
    int epfd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    // Event loop
    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);  // block indefinitely

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == listen_fd) {
                // New connection(s) ready to accept
                while (1) {
                    int conn_fd = accept(listen_fd, NULL, NULL);
                    if (conn_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;  // no more pending connections
                        perror("accept");
                        break;
                    }
                    set_nonblocking(conn_fd);
                    ev.events = EPOLLIN | EPOLLET;  // edge-triggered
                    ev.data.fd = conn_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
                }
            } else {
                // Client data ready to read
                char buf[4096];
                while (1) {
                    ssize_t count = read(events[i].data.fd, buf, sizeof(buf));
                    if (count <= 0) {
                        if (count == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                            // Client disconnected or error
                            close(events[i].data.fd);
                        }
                        break;
                    }
                    // Process data (echo back, handle request, etc.)
                    write(events[i].data.fd, buf, count);
                }
            }
        }
    }
    close(epfd);
    close(listen_fd);
}
```

### The Event Loop Pattern

```
  ┌─────────────────────────────────────────────┐
  │              Event Loop                      │
  │                                             │
  │  ┌─────────────────┐                        │
  │  │  epoll_wait()   │ ◄────── blocks until   │
  │  │  (block)        │         events arrive   │
  │  └────────┬────────┘                        │
  │           │                                 │
  │           ▼                                 │
  │  ┌─────────────────┐                        │
  │  │  For each ready │                        │
  │  │  fd:            │                        │
  │  │                 │                        │
  │  │  listen_fd? ────┼──► accept() new client │
  │  │                 │    add to epoll         │
  │  │                 │                        │
  │  │  client_fd? ────┼──► read/process/write  │
  │  │                 │    (NON-BLOCKING only!) │
  │  │                 │                        │
  │  │  error/close? ──┼──► remove from epoll   │
  │  │                 │    close(fd)            │
  │  └────────┬────────┘                        │
  │           │                                 │
  │           └──────── loop back ──────────────┘
  └─────────────────────────────────────────────┘

  CRITICAL RULE: Never do blocking work inside the event loop!
  A blocking call stops ALL clients, not just one.
```

### The Golden Rule: No Blocking in the Event Handler

Since a single thread handles all clients, **any blocking operation** (disk I/O, DNS lookup, database query, sleep) blocks **ALL** clients:

```
  Single thread processing C1, C2, C3:

  epoll_wait → C1 ready → handle C1 (fast: 1ms) ✓
             → C2 ready → handle C2 → database query (200ms) BLOCKS!
                          ^^^ C3 is ready but has to wait 200ms!

  Solution: offload blocking work to a thread pool or use async I/O.
```

### `epoll` Is for Network I/O, Not Disk

`epoll` monitors sockets, pipes, and other network-oriented fds efficiently. For disk files, `epoll` always reports them as ready (disk reads/writes go through the page cache). To achieve async disk I/O, Linux provides:
- `io_uring` (modern, since kernel 5.1) — true asynchronous I/O for both disk and network.
- `aio` (older POSIX AIO) — limited and rarely used.

---

## 5. Other Platforms

| Platform | API | Notes |
|----------|-----|-------|
| Linux | `epoll` | Scalable, O(1); edge or level triggered |
| macOS / BSD | `kqueue` | Similar to epoll; unified interface for sockets, files, signals, timers |
| Windows | IOCP (I/O Completion Ports) | Proactor model (notifies when I/O is complete, not when fd is ready) |
| Portable | `libuv` (Node.js), `libevent`, `libev` | Cross-platform libraries abstracting the OS-specific API |

---

## Summary

| Concept | Key Idea |
|---------|----------|
| I/O multiplexing | One thread monitors many fds; kernel reports which are ready |
| `select` | Oldest; O(n) scan, 1024 fd limit |
| `poll` | No fd limit; still O(n) per call |
| `epoll` | O(1) for waiting; returns only ready fds; interest list + ready list |
| `epoll_ctl` | Add/modify/remove fds from the interest set — O(1) |
| `epoll_wait` | Block until events; returns only the ready fds |
| Level vs. edge triggered | Level: fires while ready. Edge: fires on state change |
| Non-blocking fds | Required for edge-triggered; `read`/`write` until `EAGAIN` |
| No blocking in event loop | Blocking one handler stalls ALL clients |
| Disk I/O | `epoll` not useful for disk; use `io_uring` for async disk I/O |
