# Network I/O Subsystem

---

## 1. Overview: What Happens When a Packet Arrives?

When a network packet arrives at a Linux host, it traverses a complex path from the NIC hardware through the kernel networking stack to the application's socket buffer. Understanding this path is essential for performance engineering and debugging.

```
  Packet Reception — Full Path:

  ┌─────────────────────────────────────────────────────────────────┐
  │ HARDWARE                                                        │
  │  ┌─────┐    DMA     ┌──────────────┐   IRQ    ┌──────┐        │
  │  │ NIC │ ──────────► │ RX Ring      │ ────────►│ CPU  │        │
  │  │     │             │ (kernel mem) │          │      │        │
  │  └─────┘             └──────────────┘          └──┬───┘        │
  ├───────────────────────────────────────────────────┼────────────┤
  │ KERNEL                                            │            │
  │                                                   ▼            │
  │  ┌──────────────────────────────────────────────────────────┐  │
  │  │ Top Half IRQ Handler (hardirq)                           │  │
  │  │  - Acknowledge interrupt                                 │  │
  │  │  - Minimal processing                                    │  │
  │  │  - Schedule NAPI / softirq (bottom half)                 │  │
  │  └──────────────────────────┬───────────────────────────────┘  │
  │                             ▼                                  │
  │  ┌──────────────────────────────────────────────────────────┐  │
  │  │ Bottom Half (ksoftirqd / NAPI poll)                      │  │
  │  │  - Poll RX ring for packets                              │  │
  │  │  - Allocate sk_buff for each packet                      │  │
  │  │  - Parse headers, checksum validation                    │  │
  │  └──────────────────────────┬───────────────────────────────┘  │
  │                             ▼                                  │
  │  ┌──────────────────────────────────────────────────────────┐  │
  │  │ Network Stack Processing                                 │  │
  │  │  - IP layer: routing, fragmentation                      │  │
  │  │  - TCP layer: reliability, congestion control            │  │
  │  │  - Map packet to socket (by port number)                 │  │
  │  │  - Enqueue into socket receive buffer                    │  │
  │  └──────────────────────────┬───────────────────────────────┘  │
  ├─────────────────────────────┼──────────────────────────────────┤
  │ USER SPACE                  ▼                                  │
  │  ┌──────────────────────────────────────────────────────────┐  │
  │  │ Application: read(fd, buf, n) / recv(fd, buf, n, 0)     │  │
  │  │  - Copy payload from kernel socket buffer to user buffer │  │
  │  └──────────────────────────────────────────────────────────┘  │
  └─────────────────────────────────────────────────────────────────┘
```

---

## 2. Device Drivers and NIC Interaction

### NIC Configuration

The **device driver** (e.g., `ixgbe` for Intel 10G NICs, `mlx5_core` for Mellanox) manages the NIC:
- Configures the NIC via **Memory-Mapped I/O (MMIO)** — writing to device registers mapped into physical address space.
- Sets up **DMA** mappings so the NIC can write packets directly into kernel memory.
- Registers interrupt handlers for the NIC's IRQ lines.

### TX/RX Ring Buffers

The NIC and kernel communicate through **ring buffers** (circular arrays) in kernel memory:

```
  RX Ring Buffer (receive):

  ┌─────────────────────────────────────────────┐
  │  Ring of DMA buffer descriptors              │
  │                                             │
  │  ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐ ┌─────┐  │
  │  │desc0│→│desc1│→│desc2│→│desc3│→│desc4│──┐│
  │  └─────┘ └─────┘ └─────┘ └─────┘ └─────┘  ││
  │  ▲                                         ││
  │  └─────────────────────────────────────────┘│
  │  (circular — wraps around)                  │
  │                                             │
  │  Each descriptor contains:                  │
  │  - Pointer to a pre-allocated DMA buffer    │
  │  - Packet length (filled by NIC after DMA)  │
  │  - Status flags (owned by NIC or driver)    │
  └─────────────────────────────────────────────┘

  NIC writes packet data via DMA into buffer pointed
  to by next descriptor, updates descriptor, moves to
  next slot in the ring, raises interrupt.
```

```
  TX Ring Buffer (transmit):

  Driver fills descriptors with pointers to packet data.
  NIC reads descriptors, DMA's data to wire, marks done.
```

```bash
# View ring buffer sizes:
$ ethtool -g eth0
Ring parameters for eth0:
Pre-set maximums:
RX:     4096
TX:     4096
Current hardware settings:
RX:     256
TX:     256

# Increase ring buffer size:
$ sudo ethtool -G eth0 rx 4096 tx 4096
```

---

## 3. Interrupt Handling: Top Half and Bottom Half

Network interrupt handling is split into two phases to minimize the time spent with interrupts disabled.

### Top Half (Hard IRQ Handler)

Runs in **interrupt context** — all other interrupts on this CPU are masked. Must be **very fast**.

What it does:
1. Acknowledge the interrupt to the NIC/interrupt controller.
2. Optionally disable further NIC interrupts (to prevent interrupt storms).
3. Schedule the **bottom half** (softirq/NAPI poll) for deferred processing.
4. Return immediately.

```c
// Simplified top half handler:
irqreturn_t nic_irq_handler(int irq, void *dev_id) {
    struct net_device *dev = dev_id;

    // Disable further interrupts from this NIC
    disable_nic_irq(dev);

    // Schedule NAPI poll (bottom half)
    napi_schedule(&dev->napi);

    return IRQ_HANDLED;
}
```

### Bottom Half (ksoftirqd / NAPI Poll)

Runs in **softirq context** — with interrupts re-enabled. Can be preempted. Scheduled to run when the CPU is free (after returning from the hard IRQ handler, or via `ksoftirqd` kernel thread).

What it does:
1. **Poll** the RX ring for all packets that arrived since the last poll.
2. For each packet:
   - Allocate an `sk_buff` (socket buffer) structure.
   - Set up pointers to the packet's headers (Ethernet, IP, TCP/UDP).
   - Pass up to the network stack for protocol processing.
3. Re-enable NIC interrupts when the ring is drained (or a budget of packets is reached).

```
  Top Half vs. Bottom Half:

  Time ───────────────────────────────────────────────────►

  NIC:         ┌─ packet arrives ─┐
               │  DMA to RX ring  │
               │  raise IRQ       │
               └──────────────────┘

  Top Half:    ┌──── FAST ────┐
  (hardirq)    │ ack IRQ      │
               │ disable IRQ  │
               │ schedule NAPI│
               └──────┬───────┘
                      │
  Bottom Half:        ▼
  (softirq)    ┌───────────────────────────────────────┐
               │ poll RX ring                           │
               │ allocate sk_buff for each packet       │
               │ process through network stack          │
               │ deliver to sockets                     │
               │ re-enable NIC interrupts               │
               └───────────────────────────────────────┘

  The split minimizes time with interrupts disabled.
  Bottom half processes all accumulated packets in a batch.
```

### The `sk_buff` Structure

The `sk_buff` (socket buffer) is the kernel's representation of a network packet. It's the central data structure of the Linux networking stack.

```c
// Simplified sk_buff:
struct sk_buff {
    struct sk_buff *next, *prev;     // linked list pointers

    // Pointers to protocol headers:
    unsigned char *head;             // start of allocated buffer
    unsigned char *data;             // start of current data
    unsigned char *tail;             // end of current data
    unsigned char *end;              // end of allocated buffer

    // Protocol header offsets:
    __u16 transport_header;          // TCP/UDP header offset
    __u16 network_header;            // IP header offset
    __u16 mac_header;                // Ethernet header offset

    struct sock *sk;                 // socket this packet belongs to
    struct net_device *dev;          // device that received/sends this
    unsigned int len;                // total packet length
    // ... many more fields ...
};
```

```
  sk_buff pointing into packet data:

  ┌────────────────────────────────────────────────┐
  │ head                                           │
  │  ▼                                             │
  │ ┌───────────┬──────────┬──────────┬───────────┐│
  │ │ Ethernet  │ IP       │ TCP      │ Payload   ││
  │ │ Header    │ Header   │ Header   │ (data)    ││
  │ │ 14 bytes  │ 20 bytes │ 20 bytes │ N bytes   ││
  │ └───────────┴──────────┴──────────┴───────────┘│
  │  ▲           ▲          ▲                      │
  │  mac_header  network_   transport_             │
  │              header     header                 │
  │                                         ▲      │
  │                                        tail    │
  └────────────────────────────────────────────────┘

  As the packet moves up the stack, 'data' pointer
  advances past each header (header is "consumed").
```

---

## 4. Network Stack Processing

After the bottom half creates `sk_buff` structures, each packet is processed through the kernel's protocol stack:

```
  Network Stack Layers:

  ┌──────────────────────────────────────────────┐
  │  Layer 2: Link Layer                         │
  │  - Parse Ethernet header (src/dst MAC)       │
  │  - Determine protocol type (IPv4, IPv6, ARP) │
  │  - Pass to appropriate L3 handler            │
  ├──────────────────────────────────────────────┤
  │  Layer 3: Network Layer (IP)                 │
  │  - Validate IP header (checksum, TTL)        │
  │  - IP routing decision (local delivery or    │
  │    forward to another interface)             │
  │  - Reassemble fragmented packets             │
  │  - Netfilter hooks (iptables/nftables)       │
  │  - Pass to L4 handler based on protocol field│
  ├──────────────────────────────────────────────┤
  │  Layer 4: Transport Layer (TCP/UDP)          │
  │  - TCP: sequence numbers, ACKs, congestion   │
  │    control, window management, retransmission│
  │  - UDP: checksum validation, simple delivery │
  │  - Lookup destination socket by (src_ip,     │
  │    src_port, dst_ip, dst_port)               │
  │  - Enqueue packet into socket's receive queue│
  │  - Wake up any process blocked on recv()     │
  ├──────────────────────────────────────────────┤
  │  Socket Layer                                │
  │  - recv()/read() copies payload from kernel  │
  │    socket buffer to user-space buffer        │
  └──────────────────────────────────────────────┘
```

### Packet-to-Socket Mapping

The kernel uses the **4-tuple** (source IP, source port, destination IP, destination port) to look up which socket a packet belongs to:

```
  Incoming packet:  src=10.0.0.5:4321, dst=10.0.0.1:80, proto=TCP

  Kernel looks up in socket hash table:
  → Found: socket with (local_addr=10.0.0.1:80, remote_addr=10.0.0.5:4321)
  → Enqueue packet payload into this socket's receive buffer
  → If a process is blocked in recv() on this socket, wake it up
```

### Delivering Data to User Space

When the application calls `recv(fd, buf, n, 0)` or `read(fd, buf, n)`:
- Kernel copies the payload bytes from the `sk_buff`(s) in the socket's receive queue to the user-space buffer.
- This is a **copy** from kernel memory to user memory — one of the significant overheads of the network stack.

---

## 5. The Problem of Interrupts at High Load

At high packet rates (millions of packets per second on modern 10G/25G/100G NICs), interrupts become a bottleneck:

```
  At 10 Gbps with 64-byte packets:
  ~14.88 million packets/second
  = one interrupt every ~67 nanoseconds!

  Each interrupt involves:
  - CPU trap, save registers
  - Run top-half handler
  - Schedule bottom half
  - Restore registers

  Interrupt overhead DOMINATES CPU time → "interrupt livelock"
  The CPU spends all its time handling interrupts and
  never gets to actually process the packets!
```

### NAPI (New API) Polling

Linux's solution to interrupt storms: **NAPI** (New API) combines interrupts with polling.

```
  NAPI Interrupt Coalescing + Polling:

  1. First packet arrives → NIC raises interrupt (normal)
  2. Top half handler:
     - Disable NIC interrupts!
     - Schedule NAPI poll
  3. Bottom half (NAPI poll):
     - Poll the RX ring: process all available packets
     - No interrupts during polling — just keep checking the ring
     - If ring empty (no more packets): re-enable interrupts
     - If budget reached (e.g., 64 packets): yield CPU,
       schedule next poll round

  ┌─────── interrupt ──────┐ ┌──────── polling ────────────────┐
  │ packet arrives          │ │ process 64 pkts, yield,        │
  │ IRQ → disable IRQ      │ │ process 64 more, yield, ...    │
  │ schedule NAPI           │ │ ring empty → re-enable IRQ     │
  └─────────────────────────┘ └────────────────────────────────┘

  At low load: interrupt-driven (responsive, low latency)
  At high load: polling-driven (no interrupt overhead, high throughput)
```

```bash
# View NAPI statistics:
$ cat /proc/net/softnet_stat
# columns: packets_processed, time_squeeze, cpu_collision, ...

# Tune NAPI budget (packets per poll cycle):
$ sysctl net.core.netdev_budget
net.core.netdev_budget = 300
```

---

## 6. Overheads of the Linux Network Stack

For high-performance networking (10G+), the standard kernel network stack has significant overhead:

| Overhead | Description |
|----------|-------------|
| **Interrupt handling** | Even with NAPI, interrupt + softirq transitions cost cycles |
| **Kernel-user mode transitions** | `recv()`/`send()` syscalls trap into kernel and back |
| **Context switching** | From application thread to `ksoftirqd` and back |
| **Memory copy** | Packet payload copied from kernel `sk_buff` to user buffer |
| **sk_buff allocation** | `sk_buff` struct allocated/freed per packet — memory allocator overhead |
| **Protocol processing** | TCP/IP stack processing for every packet |
| **Lock contention** | Socket locks, routing table locks at high throughput |

```
  Overhead breakdown for a single packet receive:

  NIC → DMA ──► [RX Ring] ──► IRQ ──► Top Half ──► softirq ──►
                                       ~~50ns       ~~20ns

  ──► sk_buff alloc ──► IP processing ──► TCP processing ──►
      ~~100ns            ~~200ns           ~~500ns

  ──► Socket enqueue ──► recv() syscall ──► copy to user ──► app
      ~~50ns              ~~200ns            ~~300ns

  Total: ~1-2 μs per packet through the full stack
  At 14.88 Mpps (10G line rate) — need sub-100ns per packet!
```

---

## 7. Fast Network I/O Techniques

Three approaches to network I/O, from standard to specialized:

```
  ┌──────────────────────────────────────────────────────────────┐
  │                                                              │
  │  1. Generic Kernel Stack          (standard Linux)           │
  │     App ↔ syscall ↔ TCP/IP stack ↔ driver ↔ NIC             │
  │     Full features, moderate performance                     │
  │                                                              │
  │  2. Kernel Bypass (DPDK)          (skip kernel entirely)     │
  │     App ↔ user-space driver ↔ NIC                            │
  │     Maximum performance, lose kernel features               │
  │                                                              │
  │  3. In-Kernel Offload (eBPF/XDP)  (custom code in kernel)   │
  │     App ↔ eBPF program in driver ↔ NIC                      │
  │     High performance, keep kernel integration               │
  │                                                              │
  └──────────────────────────────────────────────────────────────┘
```

---

## 8. Kernel Bypass: DPDK

**DPDK** (Data Plane Development Kit) moves packet processing entirely to user space, bypassing the kernel networking stack completely.

```
  Standard Kernel Path:              DPDK Kernel Bypass:

  ┌─────────┐                        ┌─────────┐
  │ App     │                        │ App     │
  └────┬────┘                        │ + DPDK  │
       │ syscall                     │ Poll    │
  ┌────▼────┐                        │ Mode    │
  │ Kernel  │                        │ Driver  │
  │ TCP/IP  │                        └────┬────┘
  │ Stack   │                             │ MMIO + DMA
  ├─────────┤                             │ (direct HW access)
  │ Driver  │                        ┌────▼────┐
  └────┬────┘                        │   NIC   │
  ┌────▼────┐                        └─────────┘
  │   NIC   │
  └─────────┘                        Kernel driver is just a
                                     passthrough (UIO/VFIO).
                                     No interrupts. No sk_buff.
                                     No copies. No syscalls.
```

### How DPDK Works

1. **Kernel driver replaced**: NIC is bound to a UIO (Userspace I/O) or VFIO driver that simply maps the NIC's registers and DMA regions into the application's address space.
2. **Poll mode driver in user space**: The DPDK application runs a tight loop that continuously polls the NIC's RX ring — no interrupts at all.
3. **Pre-allocated packet buffers**: Uses **huge pages** (2MB/1GB pages) for packet buffers, reducing TLB misses.
4. **Batch processing**: Processes packets in bursts (e.g., 32 at a time) for cache efficiency.
5. **Zero-copy**: Packets are accessed directly in the DMA buffer — no kernel-to-user copy.

```c
// DPDK receive loop (simplified pseudocode):
while (1) {
    // Poll NIC for received packets — no blocking, no syscall
    uint16_t nb_rx = rte_eth_rx_burst(port_id, queue_id, pkts, BURST_SIZE);

    for (int i = 0; i < nb_rx; i++) {
        // Process packet directly — it's already in user memory
        process_packet(pkts[i]);
    }
}
// This loop runs at 100% CPU on a dedicated core.
// No interrupts, no kernel transitions, no copies.
```

### DPDK Tradeoffs

| Pros | Cons |
|------|------|
| Millions of packets per second (line rate on 10G/25G/100G) | Dedicated CPU cores (100% utilization even when idle) |
| Microsecond or sub-microsecond latency | Kernel networking tools don't work (`tcpdump`, `iptables`, `netstat`) |
| No kernel overhead | Hard to coexist with other applications on the same NIC |
| Predictable performance | Application must implement its own TCP/IP if needed |
| Used by telecom, NFV, high-frequency trading | Complex development — low-level packet processing |

```bash
# Bind NIC to DPDK-compatible driver:
$ sudo dpdk-devbind.py --bind=vfio-pci 0000:03:00.0

# Check NIC binding:
$ dpdk-devbind.py --status
```

---

## 9. In-Kernel Packet Processing: eBPF and XDP

**eBPF** (extended Berkeley Packet Filter) allows running **custom, sandboxed programs inside the kernel** at specific hook points — without modifying kernel source or loading kernel modules.

### What Is eBPF?

```
  eBPF: Custom Code in the Kernel

  ┌──────────────────────────────────────────────────────┐
  │  User Space                                          │
  │  ┌────────────────────────────────────────────────┐  │
  │  │  Write eBPF program (C) → compile with         │  │
  │  │  clang/LLVM to eBPF bytecode                   │  │
  │  │                                                │  │
  │  │  Load into kernel via bpf() syscall            │  │
  │  └──────────────────────┬─────────────────────────┘  │
  │                         │                            │
  ├─────────────────────────┼────────────────────────────┤
  │  Kernel Space           ▼                            │
  │  ┌────────────────────────────────────────────────┐  │
  │  │  eBPF Verifier: checks safety                  │  │
  │  │  - No infinite loops (DAG check)               │  │
  │  │  - No out-of-bounds memory access              │  │
  │  │  - No unsafe operations                        │  │
  │  ├────────────────────────────────────────────────┤  │
  │  │  JIT Compiler: eBPF bytecode → native x86     │  │
  │  │  (runs at near-native speed)                   │  │
  │  ├────────────────────────────────────────────────┤  │
  │  │  Attach to hook point:                         │  │
  │  │  - XDP (NIC driver level)                      │  │
  │  │  - tc (traffic control)                        │  │
  │  │  - socket filters                              │  │
  │  │  - kprobes, tracepoints, ...                   │  │
  │  └────────────────────────────────────────────────┘  │
  └──────────────────────────────────────────────────────┘
```

### The eBPF Pipeline

1. **Write**: eBPF programs are written in C (restricted subset — no loops without bounds, no function pointers, limited stack).
2. **Compile**: `clang -target bpf` compiles C to eBPF bytecode (a custom ISA with 11 registers, 64-bit).
3. **Load**: The `bpf()` syscall loads the bytecode into the kernel.
4. **Verify**: The kernel **verifier** statically analyzes the program to guarantee safety:
   - No unbounded loops (program must terminate).
   - No out-of-bounds memory accesses.
   - No unsafe pointer arithmetic.
   - If verification fails, the program is rejected.
5. **JIT Compile**: eBPF bytecode is JIT-compiled to native machine code (x86, ARM) for near-native performance.
6. **Attach**: Program is attached to a kernel hook point and runs every time that hook is triggered.

### XDP (eXpress Data Path)

**XDP** is an eBPF hook at the **NIC driver level** — the earliest possible point in the receive path, before `sk_buff` allocation and the full networking stack.

```
  XDP Hook Point:

  NIC → DMA → RX Ring → [XDP PROGRAM RUNS HERE] → ?
                          │
                          ├── XDP_PASS  → Continue to normal kernel stack
                          ├── XDP_DROP  → Drop packet immediately (never reaches stack)
                          ├── XDP_TX    → Transmit packet back out the same NIC
                          ├── XDP_REDIRECT → Send to another NIC or CPU
                          └── XDP_ABORTED → Error, drop and log

  No sk_buff allocated! No copies! Processes raw packet data.
  Runs at millions of packets per second.
```

### XDP Example: Simple Firewall

```c
// XDP program: drop all packets from IP 10.0.0.99
SEC("xdp")
int xdp_firewall(struct xdp_md *ctx) {
    void *data = (void *)(long)ctx->data;
    void *data_end = (void *)(long)ctx->data_end;

    // Parse Ethernet header
    struct ethhdr *eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_ABORTED;

    if (eth->h_proto != htons(ETH_P_IP))
        return XDP_PASS;  // not IPv4, pass through

    // Parse IP header
    struct iphdr *ip = (void *)(eth + 1);
    if ((void *)(ip + 1) > data_end)
        return XDP_ABORTED;

    // Drop packets from 10.0.0.99
    if (ip->saddr == htonl(0x0A000063))
        return XDP_DROP;   // dropped at driver level — extremely fast

    return XDP_PASS;  // allow everything else
}
```

### XDP Use Cases

| Use Case | How |
|----------|-----|
| **DDoS mitigation** | `XDP_DROP` malicious packets at line rate before they reach the stack |
| **Load balancing** | `XDP_TX` / `XDP_REDIRECT` to forward packets to backend servers |
| **Firewalling** | Filter packets at driver level (Facebook's Katran, Cloudflare) |
| **Monitoring** | Inspect packets and update eBPF maps (counters, flow tables) without copying |
| **Key-value stores** | Respond to simple queries entirely in XDP (e.g., BMC) |

### eBPF vs. DPDK

| Aspect | DPDK | eBPF/XDP |
|--------|------|----------|
| Runs in | User space | Kernel (at hook points) |
| Kernel involvement | Bypassed entirely | Kernel runs the eBPF program |
| NIC control | Application owns the NIC exclusively | Kernel still manages the NIC |
| Kernel tools | Don't work (`tcpdump`, `iptables`, etc.) | Work alongside eBPF |
| Safety | None — user code with full access | Verifier guarantees safety |
| Performance | Maximum (no kernel at all) | Near-maximum (runs in driver, JIT compiled) |
| Complexity | Full custom stack needed | Restricted C, fits into existing stack |
| Coexistence | Hard to share NIC with kernel | Natural — packets can pass to kernel stack |

```bash
# Load XDP program:
$ ip link set dev eth0 xdpgeneric obj xdp_firewall.o sec xdp

# View attached XDP program:
$ ip link show eth0
2: eth0: <BROADCAST,MULTICAST,UP> ... xdp

# Remove XDP program:
$ ip link set dev eth0 xdpgeneric off

# Use bpftool to inspect loaded eBPF programs:
$ sudo bpftool prog list
$ sudo bpftool map list
```

---

## 10. Comparison of Approaches

```
  Performance vs. Flexibility Spectrum:

  ◄── More Features / Flexibility          More Performance ──►

  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
  │  Standard     │  │  eBPF / XDP  │  │   DPDK       │
  │  Kernel Stack │  │              │  │              │
  │              │  │  Custom code  │  │  Full kernel │
  │  Full TCP/IP │  │  at driver    │  │  bypass      │
  │  All tools   │  │  level        │  │              │
  │  work        │  │              │  │  Dedicated   │
  │              │  │  Kernel tools │  │  cores       │
  │  ~1-2 μs/pkt│  │  still work   │  │              │
  │              │  │              │  │  ~0.1 μs/pkt│
  │              │  │  ~0.2 μs/pkt│  │              │
  └──────────────┘  └──────────────┘  └──────────────┘
     General            Specialized        Maximum
     purpose            filtering/         packet
     networking         forwarding         processing
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| NIC + DMA | NIC writes packets to kernel memory via DMA; raises IRQ | RX/TX ring buffers; `ethtool -g` |
| Top half IRQ | Minimal work: ack interrupt, schedule bottom half | Hard IRQ context, interrupts disabled |
| Bottom half | Process accumulated packets, allocate `sk_buff`, run protocol stack | `ksoftirqd`, NAPI poll |
| `sk_buff` | Kernel packet representation; pointers to headers | Central networking data structure |
| Protocol stack | L2→L3→L4 processing; map packet to socket; enqueue | IP routing, TCP state machine |
| NAPI | Interrupt + polling hybrid; prevents interrupt livelock | Low load: interrupt. High load: poll |
| Network stack overhead | Interrupts, copies, syscalls, sk_buff alloc, protocol processing | ~1-2 μs per packet |
| DPDK (kernel bypass) | User-space poll-mode driver; bypass kernel entirely | Millions of pps; dedicated cores; no kernel tools |
| eBPF | Safe, custom programs running inside kernel at hook points | Verifier + JIT; `bpf()` syscall |
| XDP | eBPF hook at NIC driver level; pre-sk_buff processing | `XDP_DROP/PASS/TX/REDIRECT`; line-rate filtering |
