# Network Architecture

---

## 1. Clients and Servers Are Everywhere

Most components in a computer system perform network I/O — sometimes as clients, sometimes as servers, often both:

```
  Typical Web Request Flow:

  ┌────────┐  HTTP   ┌───────────┐  SQL/RPC  ┌───────────┐
  │ Browser│ ──────► │ Web Server│ ────────► │ Database  │
  │(client)│ ◄────── │ (server + │ ◄──────── │ (server)  │
  └────────┘         │  client)  │           └───────────┘
                     └─────┬─────┘
                           │ RPC
                           ▼
                     ┌───────────┐
                     │ Cache     │
                     │ (Redis)   │
                     └───────────┘

  The web server is a SERVER to the browser
  and a CLIENT to the database and cache.
```

At every network boundary, a design choice must be made about how to handle concurrent I/O.

---

## 2. Higher-Level Network APIs

The raw socket API (`socket`, `bind`, `listen`, `accept`, `read`, `write`) is low-level. Programming language libraries and frameworks provide higher-level abstractions:

### Remote Procedure Call (RPC)

RPC makes calling a function on a remote server look like a local function call. The library handles serialization, network transport, and deserialization transparently.

```
  Local function call:             RPC (looks the same!):

  result = compute(x, y);         result = compute(x, y);
  // runs locally                 // Actually:
                                  // 1. Serialize x, y
                                  // 2. Send over network to server
                                  // 3. Server deserializes, calls compute
                                  // 4. Server serializes result
                                  // 5. Send result back
                                  // 6. Client deserializes result
                                  // All hidden by the RPC framework!
```

Examples: gRPC (Google), Apache Thrift, JSON-RPC, XML-RPC.

### Other Abstractions

| Abstraction | Examples | Level |
|------------|---------|-------|
| Raw sockets | POSIX sockets, Winsock | Lowest — manual buffer management |
| HTTP libraries | `libcurl`, Python `requests`, Go `net/http` | Medium — handles HTTP protocol |
| RPC frameworks | gRPC, Thrift, Cap'n Proto | High — function-call semantics |
| Message queues | ZeroMQ, RabbitMQ, Kafka | High — async publish/subscribe |

Regardless of the high-level API, the underlying OS mechanism is one of two architectures.

---

## 3. The Two Architectures

Every networked application ultimately chooses between two fundamental approaches:

### Architecture 1: Thread-per-Connection (Blocking/Synchronous)

```
  ┌──────────────────────────────────────────┐
  │  Thread-per-Connection                   │
  │                                          │
  │  Main Thread: accept loop                │
  │       │                                  │
  │       ├──► Thread 1: handle(C1)          │
  │       ├──► Thread 2: handle(C2)          │
  │       ├──► Thread 3: handle(C3)          │
  │       └──► ...                           │
  │                                          │
  │  Each thread uses BLOCKING I/O:          │
  │    read(fd, ...)  — blocks until data    │
  │    write(fd, ...) — blocks until written │
  │    query(db, ...) — blocks until result  │
  └──────────────────────────────────────────┘
```

| Pros | Cons |
|------|------|
| Simple, sequential code | One thread per connection — memory + scheduling overhead |
| Easy to reason about — no callbacks or state machines | Context switches between thousands of threads |
| Natural error handling (exceptions, stack traces) | Doesn't scale to very large connection counts (C10K+) |
| Easy to use CPU-bound operations | Thread pool mitigates but limits concurrency |

**Used by**: Traditional Java servlets (Tomcat), Apache prefork/worker, many database clients, Python's `socketserver`.

### Architecture 2: Event-Driven (Asynchronous)

```
  ┌──────────────────────────────────────────┐
  │  Event-Driven                            │
  │                                          │
  │  Single (or few) Thread(s):              │
  │                                          │
  │  while (true) {                          │
  │      events = epoll_wait(...)            │
  │      for each event:                     │
  │          if (new connection) → accept    │
  │          if (data ready)    → read       │
  │          if (writable)      → write      │
  │  }                                       │
  │                                          │
  │  Uses NON-BLOCKING I/O:                  │
  │  All operations return immediately.      │
  │  Kernel notifies when I/O is possible.   │
  └──────────────────────────────────────────┘
```

| Pros | Cons |
|------|------|
| Very few threads — minimal memory and context switch overhead | Complex code — callbacks, state machines, or async/await |
| Scales to tens/hundreds of thousands of connections | One slow handler blocks ALL connections (must not block!) |
| Less overhead per connection | Harder to use multiple CPU cores (need multi-threaded event loop or multiple processes) |
| Lower latency (no scheduling overhead) | Debugging is harder (no linear stack traces) |

**Used by**: Nginx, Node.js, Redis, HAProxy, libuv-based systems.

### Comparison Table

| Aspect | Thread-per-Connection | Event-Driven |
|--------|----------------------|-------------|
| Connections | Hundreds to thousands | Tens of thousands to millions |
| Memory per conn | ~8KB-8MB (thread stack) | ~Bytes-KB (connection state struct) |
| CPU usage | Context switch overhead | Minimal overhead, but single-threaded |
| Code style | Sequential, blocking | Callbacks / async-await / state machines |
| Blocking I/O | Yes (natural) | Forbidden in event loop |
| Multi-core | Natural (threads on different cores) | Requires worker threads or multiple event loops |
| Best for | CPU-bound + I/O mixed workloads | I/O-heavy, many-connection workloads |

---

## 4. Hybrid Architectures

Modern high-performance servers often combine both approaches:

### Multi-Threaded Event Loop

Run one event loop per CPU core. Each event loop handles a subset of connections. No sharing needed between loops.

```
  ┌────────────────────────────────────────────────────┐
  │  Multi-Threaded Event-Driven                       │
  │                                                    │
  │  Core 0: ┌─────────────────────┐                   │
  │          │ Event Loop Thread 0 │ handles C1, C4, C7│
  │          └─────────────────────┘                   │
  │  Core 1: ┌─────────────────────┐                   │
  │          │ Event Loop Thread 1 │ handles C2, C5, C8│
  │          └─────────────────────┘                   │
  │  Core 2: ┌─────────────────────┐                   │
  │          │ Event Loop Thread 2 │ handles C3, C6, C9│
  │          └─────────────────────┘                   │
  │                                                    │
  │  Each thread: independent epoll instance            │
  │  Connections distributed across threads.           │
  │  No locks needed (each thread owns its connections)│
  └────────────────────────────────────────────────────┘
```

**Used by**: Nginx (worker processes), Envoy proxy, many game servers.

### Event Loop + Thread Pool for Blocking Work

Single event loop for I/O, but offload blocking operations (disk I/O, DNS, CPU-heavy computation) to a thread pool.

```
  ┌────────────────────────────────────────────────────┐
  │  Event Loop + Thread Pool                          │
  │                                                    │
  │  Event Loop Thread:                                │
  │    epoll_wait → C1 needs DB query                  │
  │                 → submit to thread pool             │
  │                 → continue handling C2, C3...       │
  │                                                    │
  │  Thread Pool:                                      │
  │    Worker 0: execute DB query for C1               │
  │    Worker 1: read file for C5                      │
  │    → When done, notify event loop (via pipe/eventfd)│
  │    → Event loop sends response to C1               │
  └────────────────────────────────────────────────────┘
```

**Used by**: Node.js (libuv uses a thread pool for file I/O and DNS), Netty, Tokio (Rust).

---

## 5. Real-World Examples

| System | Architecture | Details |
|--------|-------------|---------|
| **Nginx** | Multi-process event-driven | One event loop per worker process; master process manages workers; epoll/kqueue |
| **Node.js** | Single-threaded event loop + thread pool | V8 JS runs on main thread; libuv thread pool for file I/O, DNS |
| **Redis** | Single-threaded event-driven | All data operations on main thread (no locks needed!); I/O threads for network since 6.0 |
| **Apache** (prefork) | Process-per-connection | fork() a child for each request; simple but memory-heavy |
| **Apache** (event MPM) | Hybrid | Event-driven for keep-alive connections; threads for request handling |
| **Go net/http** | Goroutine-per-connection | Lightweight goroutines (~2KB stack); runtime multiplexes onto OS threads; feels like thread-per-connection but scales like event-driven |
| **Envoy** | Multi-threaded event-driven | One event loop per worker thread; shared-nothing design |

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Thread-per-connection | Simple blocking code; doesn't scale past thousands |
| Event-driven | Complex async code; scales to millions of connections |
| Hybrid | Event loop for I/O + thread pool for blocking work |
| Multi-threaded event loop | One event loop per core; shared-nothing |
| RPC | Makes remote calls look local; hides network complexity |
| Design choice | Determined by connection count, workload type (I/O vs CPU), and complexity tolerance |
