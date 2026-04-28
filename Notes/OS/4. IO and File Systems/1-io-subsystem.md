# I/O Subsystem

---

## 1. Overview of I/O Devices

From the OS perspective, the system is a CPU connected to main memory (RAM) and a collection of **I/O devices** — disks, network cards, keyboards, displays, GPUs, etc. The OS must manage all of these, abstracting away hardware-specific details from user programs.

```
  ┌─────────┐     ┌───────────────┐     ┌────────────────────────────────┐
  │   CPU   │◄───►│  Main Memory  │◄───►│  I/O Devices                   │
  │         │     │   (RAM)       │     │  ┌─────┐ ┌─────┐ ┌──────────┐ │
  │         │     │               │     │  │Disk │ │ NIC │ │ Keyboard │ │
  └─────────┘     └───────────────┘     │  └─────┘ └─────┘ └──────────┘ │
                                        └────────────────────────────────┘
```

### Device Classification

| Type | Description | Examples |
|------|-------------|---------|
| **Block devices** | Data accessed in fixed-size blocks (sectors). Supports random access — can seek to any block. | Hard disks, SSDs, USB drives |
| **Character devices** | Data accessed as a stream of bytes. No seeking — data arrives/departs sequentially. | Keyboard, mouse, serial port, terminal |
| **Network devices** | Packet-based. Neither purely block nor character — special socket/NIC interfaces. | Ethernet NIC, WiFi adapter |

```bash
# List devices on Linux:
$ ls /dev/sd*       # block devices (SCSI/SATA disks)
$ ls /dev/tty*      # character devices (terminals)
$ ls /dev/nvme*     # NVMe SSDs (block)
$ lsblk             # list block devices
$ cat /proc/devices # registered character and block device drivers
```

---

## 2. Canonical Device Model

Every I/O device, regardless of complexity, can be understood through a simplified model: the device exposes a set of **hardware registers** that the CPU uses to interact with it.

```
  ┌────────────────────────────────────────────────┐
  │  I/O Device                                    │
  │                                                │
  │  ┌──────────────────────────────────────────┐  │
  │  │  Device Interface (Hardware Registers)    │  │
  │  │                                          │  │
  │  │  ┌──────────────┐  Register set visible  │  │
  │  │  │ Status Reg   │  to OS / CPU           │  │
  │  │  ├──────────────┤                        │  │
  │  │  │ Command Reg  │                        │  │
  │  │  ├──────────────┤                        │  │
  │  │  │ Data Reg(s)  │                        │  │
  │  │  └──────────────┘                        │  │
  │  └──────────────────────────────────────────┘  │
  │                                                │
  │  ┌──────────────────────────────────────────┐  │
  │  │  Device Internals (hidden from OS)        │  │
  │  │  - Device controller (firmware/hardware)  │  │
  │  │  - Mechanical components (disk platters)  │  │
  │  │  - Internal buffers, caches              │  │
  │  └──────────────────────────────────────────┘  │
  └────────────────────────────────────────────────┘
```

### The Three Key Registers

| Register | Purpose | Direction |
|----------|---------|-----------|
| **Status** | Reports current state of device (busy, done, error) | Device → CPU (read by OS) |
| **Command** | Tells device what operation to perform (read, write, seek) | CPU → Device (written by OS) |
| **Data** | Transfer buffer for data being read from or written to device | Bidirectional |

The **device controller** is the hardware/firmware inside the device that manages its internals. The controller interprets commands written to the command register and executes them. The rest of the OS doesn't need to know how the device works internally — only the **device driver** speaks the device's "language."

---

## 3. How the OS Accesses Device Registers

The CPU needs to read and write device registers. Two mechanisms:

### 3.1. Port-Mapped I/O (Explicit I/O Instructions)

The CPU has **dedicated I/O instructions** that access a separate I/O address space (not the regular memory address space).

```c
// x86 privileged instructions:
outb(port, value);   // write byte to I/O port
inb(port);           // read byte from I/O port
outl(port, value);   // write 32-bit word
inl(port);           // read 32-bit word
```

- Separate address space from memory (I/O port space, 0x0000–0xFFFF on x86).
- Privileged instructions — only kernel mode code can use them.
- Legacy approach, still used for some devices (serial ports, PIC, PS/2 keyboard).

```bash
# View I/O port assignments on Linux:
$ cat /proc/ioports
0000-0cf7 : PCI Bus 0000:00
  0060-0060 : keyboard
  0064-0064 : keyboard
  03f8-03ff : serial
```

### 3.2. Memory-Mapped I/O (MMIO)

Device registers are mapped into the CPU's **physical memory address space**. The OS reads and writes them using regular load/store instructions — the memory hardware (bus controller, memory management unit) routes these accesses to the device instead of RAM.

```
  Physical Address Space:

  0x00000000 ┌──────────────────┐
             │  RAM              │
             │  (normal memory)  │
  0x40000000 ├──────────────────┤
             │  Device A Regs   │ ← writes here go to Device A
  0x40001000 ├──────────────────┤
             │  Device B Regs   │ ← writes here go to Device B
  0x40002000 ├──────────────────┤
             │  ... more RAM    │
             └──────────────────┘
```

```c
// MMIO access (simplified):
volatile uint32_t *status_reg = (uint32_t *)0xFE200000;
volatile uint32_t *data_reg   = (uint32_t *)0xFE200004;

uint32_t status = *status_reg;    // read device status (load instruction)
*data_reg = 0x42;                 // write to device data register (store instruction)
```

- **Modern standard** — PCIe devices, GPUs, NICs, NVMe controllers all use MMIO.
- No special instructions needed — just memory load/store.
- Device regions are marked as uncacheable (write-through, no caching) in the page tables to ensure every access reaches the device.

```bash
# View MMIO regions on Linux:
$ cat /proc/iomem
00000000-0009ffff : System RAM
000a0000-000bffff : PCI Bus 0000:00
  000a0000-000bffff : Video RAM area
fe000000-fe7fffff : 0000:00:02.0   # GPU MMIO region
```

---

## 4. A Simple I/O Request: Polling

The most basic way to perform I/O: the CPU directly manages every step.

### Example: Reading a Block from Disk

```c
// Pseudocode: CPU-managed disk read with polling
while (status_reg & BUSY)
    ;  // 1. Wait until device is ready

command_reg = READ_BLOCK;    // 2. Issue read command
address_reg = block_number;  // 3. Specify which block

while (!(status_reg & DONE))
    ;  // 4. Poll: busy-wait until operation completes

// 5. Copy data from device data register to memory
for (int i = 0; i < BLOCK_SIZE; i++)
    memory[dest + i] = data_reg;
```

```
  Polling Timeline:

  CPU: ┌─issue cmd─┐┌──POLL──POLL──POLL──POLL──┐┌─copy data─┐
       └───────────┘└──────────────────────────┘└───────────┘
                     ^^^ CPU spinning, doing    
                         NOTHING useful while
                         waiting for device

  Device:            ┌─────────── working ──────────┐
                     └──────────────────────────────┘
```

**Problem**: The CPU is **wasting cycles** spinning in the polling loop. It can't do anything else. For slow devices (disks: milliseconds), this is catastrophic — millions of CPU cycles burned doing nothing.

**When polling is acceptable**: When the device is extremely fast (some high-speed NICs, NVMe SSDs with very low latency), the overhead of an interrupt (context switch, handler setup) may exceed the polling wait time.

---

## 5. Interrupts

### The Idea

Instead of the CPU polling the device, the device **notifies the CPU** when the operation is complete by raising an **interrupt**.

```
  Interrupt-Driven I/O Timeline:

  CPU:    ┌─issue cmd─┐┌─run other process P2──────┐┌─handle─┐
          └───────────┘└────────────────────────────┘│interrupt│
                                                     └────────┘
                                                        ▲
  Device:              ┌────── working ────────────────┐│
                       └───────────────────────────────┘│
                                                   raises IRQ
```

The CPU can do useful work (run another process) while the device processes the request. When the device finishes, it raises a hardware interrupt.

### How Interrupts Work (Recap from Process Management)

1. Device completes operation and asserts an **interrupt line** (IRQ — Interrupt Request).
2. CPU finishes current instruction, checks for pending interrupts.
3. CPU saves current state (registers, PC) onto the kernel stack.
4. CPU looks up the **Interrupt Descriptor Table (IDT)** using the IRQ number to find the appropriate handler.
5. CPU jumps to the **interrupt handler** (Interrupt Service Routine — ISR) in the kernel.
6. Handler processes the I/O completion, wakes up the waiting process.
7. CPU restores state and resumes (possibly a different process, via the scheduler).

```
  Interrupt Flow:

  ┌────────────┐                    ┌────────────┐
  │  Device    │ ── IRQ line ──────►│    CPU      │
  │  (e.g.disk)│                    │             │
  └────────────┘                    └──────┬──────┘
                                           │
                                    save state
                                           │
                                    ┌──────▼──────┐
                                    │     IDT     │
                                    │ IRQ# → ISR  │
                                    └──────┬──────┘
                                           │
                                    ┌──────▼──────────────────┐
                                    │  Interrupt Handler (ISR) │
                                    │  - Read device status    │
                                    │  - Copy data if needed   │
                                    │  - Wake blocked process  │
                                    │  - Start next I/O req    │
                                    └──────────────────────────┘
```

### Interrupt Handler Responsibilities

The interrupt handler (ISR) for a device:

1. **Acknowledges** the interrupt (tells device/interrupt controller it's been handled).
2. **Reads device status** — success or error?
3. **Processes the I/O result** — e.g., makes data available.
4. **Unblocks the waiting process** — marks it as ready/runnable.
5. **Starts the next I/O request** — if other requests are queued.

### Interrupts vs. Polling: Tradeoff

| Aspect | Polling | Interrupts |
|--------|---------|-----------|
| CPU utilization | Wastes cycles spinning | CPU free during I/O |
| Latency | Immediate detection (no handler setup) | Handler setup overhead (save/restore state, IDT lookup) |
| Best for | Very fast devices | Slow devices (disk, network) |
| Throughput | Good at high load (no interrupt storm) | Can overwhelm at very high rates (interrupt livelock) |

**Hybrid approach** (used in modern Linux NAPI for networking): Use interrupts to detect initial activity, then switch to polling during high-throughput bursts to avoid interrupt storms.

---

## 6. Direct Memory Access (DMA)

Even with interrupts, there's still a problem: **the CPU must copy data between device data registers and main memory**, byte by byte or word by word. For large transfers (disk blocks, network packets), this wastes many CPU cycles on data movement.

### The Solution: DMA Engine

A **DMA engine** (or DMA controller) is a dedicated hardware component that can copy data between main memory and device buffers **without CPU involvement**.

```
  Without DMA (Programmed I/O):

  CPU: ┌─issue cmd─┐┌──────copy data──────┐┌─done─┐
       └───────────┘└──────────────────────┘└──────┘
                     ^^^ CPU copies each byte
                         from device → memory
                         (or memory → device)

  With DMA:

  CPU:  ┌─setup DMA─┐┌─── free! run other work ───┐┌─handle IRQ─┐
        └───────────┘└─────────────────────────────┘└────────────┘
                                                       ▲
  DMA:               ┌── copy data device↔memory ──┐   │
                     └─────────────────────────────┘   │
                                                  raises IRQ
```

### How DMA Works

1. **CPU programs the DMA engine**: Gives it the source address, destination address, transfer size, and direction (read/write).
2. **DMA engine performs the transfer**: Copies data directly between device and main memory over the system bus, without CPU involvement.
3. **DMA engine signals completion**: Raises an interrupt when the transfer is done.

```
  DMA Data Flow (Disk Read):

  ┌──────┐                      ┌──────────────┐
  │ CPU  │ ─── (1) setup ────►  │  DMA Engine  │
  │      │      DMA transfer    │              │
  │      │                      │              │
  │      │ ◄── (3) interrupt ── │              │
  └──────┘                      └──────┬───────┘
                                       │
                           (2) DMA transfer
                                       │
                                ┌──────▼───────┐
                                │ Main Memory  │ ◄──── data ──── ┌──────┐
                                │ (RAM)        │                 │ Disk │
                                └──────────────┘                 └──────┘
                                The DMA engine copies data
                                directly from disk to RAM.
                                CPU is not involved in the copy.
```

---

## 7. Disk Read: Complete Lifecycle (Interrupt + DMA)

Putting it all together — a complete disk read from a user program's `read()` syscall to data delivery:

```
  Complete Disk Read Lifecycle:

  User Space         Kernel Space              Hardware
  ──────────         ────────────              ────────
  Process P1:
  read(fd, buf, n)
       │
       ▼
  ┌─ syscall trap ─────────────────────────────────────┐
  │                                                     │
  │  1. Identify disk block(s) from fd, offset, inode   │
  │  2. Check disk buffer cache — MISS                 │
  │  3. Program DMA: src=disk block, dst=cache buffer  │
  │  4. Issue command to disk controller ───────────────┼──► Disk
  │  5. P1 → BLOCKED (sleeping on I/O)                │    starts
  │  6. Context switch to P2 (P1 can't run) ──────────┤    reading
  │                                                     │      │
  │  ─── Time passes, P2 runs ───                     │      │
  │                                                     │      ▼
  │  7. Disk completes read                            │    DMA
  │     DMA copies data: disk → buffer cache ◄─────────┼──  engine
  │  8. Disk raises interrupt (IRQ) ───────────────────┼──  copies
  │                                                     │    data
  │  9. Interrupt handler:                             │
  │     - Acknowledge interrupt                        │
  │     - Data now in buffer cache                     │
  │     - Mark P1 as READY (runnable)                  │
  │     - (interrupt handled in kernel mode of P2)     │
  │                                                     │
  │  10. Scheduler eventually switches back to P1      │
  │  11. Copy data: buffer cache → user buffer (buf)   │
  │  12. Return to user space                          │
  │                                                     │
  └─────────────────────────────────────────────────────┘
       │
       ▼
  P1: read() returns n bytes (or fewer, or error)
  P1 continues execution
```

### Step-by-Step Summary

| Step | What Happens | Who Does It |
|------|-------------|-------------|
| 1 | User calls `read()`, traps into kernel | Process P1 |
| 2 | OS determines which disk block(s) to read (from inode, offset) | Kernel (filesystem layer) |
| 3 | OS checks disk buffer cache — miss | Kernel (buffer cache) |
| 4 | OS programs DMA and issues read command to disk | Kernel (device driver) |
| 5 | P1 is blocked (waiting for I/O) | Kernel (scheduler) |
| 6 | OS context-switches to another runnable process P2 | Kernel (scheduler) |
| 7 | Disk reads data, DMA copies to buffer cache | Hardware (disk + DMA) |
| 8 | Disk controller raises interrupt | Hardware |
| 9 | Interrupt handler runs (in kernel mode of P2), marks P1 as ready | Kernel (ISR) |
| 10 | Scheduler picks P1 to run | Kernel (scheduler) |
| 11 | Data copied from buffer cache to user-space buffer | Kernel |
| 12 | `read()` returns, P1 resumes in user space | Process P1 |

---

## 8. The I/O Stack in the Kernel

The Linux kernel organizes I/O handling in layers, each abstracting away the layer below:

```
  User Space
  ──────────────────────────────────────────────────
  │  Application: read(fd, buf, 4096)              │
  ──────────────────────────────────────────────────
  │  System Call Interface: sys_read()             │
  ├────────────────────────────────────────────────┤
  │  Virtual File System (VFS)                     │
  │  - Dispatches to correct filesystem            │
  │  - Uniform file/dir/inode operations           │
  ├────────────────────────────────────────────────┤
  │  Specific Filesystem (ext4, xfs, btrfs, ...)   │
  │  - Translates file offset → disk block number  │
  │  - Manages inodes, directories, metadata       │
  ├────────────────────────────────────────────────┤
  │  Block Layer / Disk Buffer Cache               │
  │  - Page cache: caches disk blocks in RAM       │
  │  - I/O scheduler: merges/reorders requests     │
  │  - Sends block I/O requests to driver          │
  ├────────────────────────────────────────────────┤
  │  Device Driver (e.g., AHCI, NVMe, virtio-blk) │
  │  - Speaks the device's specific protocol       │
  │  - Programs DMA, issues commands               │
  │  - Handles device interrupts                   │
  ├────────────────────────────────────────────────┤
  │  Hardware: Disk Controller + Disk              │
  └────────────────────────────────────────────────┘
```

### Device Drivers

A **device driver** is kernel code that knows how to talk to a specific device (or class of devices). It:
- Initializes the device at boot time.
- Translates generic block I/O requests into device-specific commands.
- Programs DMA transfers.
- Handles device interrupts.
- Reports errors.

The rest of the kernel interacts with the driver through a **uniform interface** — it doesn't need to know whether the underlying device is a SATA SSD, NVMe drive, or virtual disk.

```bash
# List loaded kernel modules (many are device drivers):
$ lsmod
Module                  Size  Used by
nvme                   45056  3
nvme_core              98304  5 nvme
ahci                   40960  2
e1000e                 282624  0     # Intel NIC driver

# Device driver information:
$ modinfo nvme
$ ls /sys/class/block/sda/device/driver  # which driver manages sda
```

### Why Layers?

- **Modularity**: Can swap filesystem implementations without changing device drivers, and vice versa.
- **Reuse**: Buffer cache, I/O scheduler, and VFS are shared by all filesystems and drivers.
- **Abstraction**: User programs see a uniform `read()`/`write()` interface regardless of underlying hardware.

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| Block vs. character device | Block: random access, fixed-size blocks. Character: sequential byte stream | `/dev/sda` (block), `/dev/ttyS0` (char) |
| Canonical device model | Status, command, data registers | Device controller hides internals |
| Port-mapped I/O | Separate I/O address space, explicit `in`/`out` instructions | Legacy; `/proc/ioports` |
| Memory-mapped I/O (MMIO) | Device registers in physical address space, accessed via load/store | Modern standard; `/proc/iomem` |
| Polling | CPU busy-waits for device completion | Simple but wastes CPU cycles |
| Interrupts | Device notifies CPU via IRQ when done; CPU free to do other work | IDT, ISR, IRQ; used for most devices |
| DMA | Hardware copies data between device and memory without CPU | DMA engine; CPU only programs the transfer |
| Disk read lifecycle | syscall → disk command → DMA → interrupt → buffer cache → user buffer | P1 blocks, P2 runs, ISR wakes P1 |
| I/O stack | VFS → filesystem → buffer cache → device driver → hardware | Layered for modularity and abstraction |
| Device driver | Kernel code that speaks a specific device's protocol | `lsmod`, `/sys/class/block/` |
