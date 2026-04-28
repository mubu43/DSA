# File System Internals

---

## 1. On-Disk Layout of a Simple File System

To understand how file systems work, let's start with a simplified layout of a filesystem on disk. The disk is divided into fixed-size **blocks** (typically 4 KB on modern systems).

```
  Simple Filesystem Layout on Disk:

  Block #:  0        1         2         3-10       11 ─────────── N
  ┌────────┬─────────┬─────────┬──────────┬──────────────────────────┐
  │ Super  │ Inode   │ Data    │ Inode    │  Data Blocks             │
  │ Block  │ Bitmap  │ Bitmap  │ Table    │  (file and directory     │
  │        │         │         │          │   contents)              │
  └────────┴─────────┴─────────┴──────────┴──────────────────────────┘
     │         │          │         │              │
     │         │          │         │              └─ Actual file data
     │         │          │         └─ Array of inode structs
     │         │          └─ Which data blocks are free/used
     │         └─ Which inodes are free/used
     └─ Filesystem metadata (size, block count, inode count, magic#)
```

### Superblock

The superblock stores **filesystem-level metadata**:
- Total number of blocks and inodes.
- Size of the inode table.
- Magic number (identifies the filesystem type).
- Block size.
- Mount count, last mount time, etc.

```bash
# View ext4 superblock:
$ sudo dumpe2fs /dev/sda1 | head -40
Filesystem volume name:   <none>
Filesystem UUID:          a1b2c3d4-...
Filesystem magic number:  0xEF53
Block count:              26214400
Block size:               4096
Inode count:              6553600
Free blocks:              18234567
Free inodes:              6500000

# Or via tune2fs:
$ sudo tune2fs -l /dev/sda1
```

### Bitmaps

- **Inode bitmap**: One bit per inode. 1 = allocated, 0 = free.
- **Data bitmap**: One bit per data block. 1 = allocated, 0 = free.

These allow the filesystem to quickly find free inodes or data blocks when creating new files.

### Inode Table

A contiguous array of inode structures on disk. Each inode has a fixed size (typically 128 or 256 bytes in ext4) and is identified by its index in this table (the inode number).

---

## 2. The Inode In Depth

The inode is the central data structure of a UNIX filesystem. It stores everything about a file **except its name**.

### ext4 Inode Structure (Simplified)

```
  ┌────────────────────────────────────────────────────┐
  │  Inode (256 bytes in ext4)                         │
  │                                                    │
  │  Type & permissions   (file, dir, symlink, ...)    │
  │  Owner (UID, GID)                                  │
  │  Size (bytes)                                      │
  │  Timestamps (atime, mtime, ctime, crtime)          │
  │  Link count                                        │
  │  Flags                                             │
  │  Block count (512-byte units)                      │
  │                                                    │
  │  Data Block Pointers:                              │
  │  ┌─────────────────────────────────────────────┐   │
  │  │ 12 direct pointers         → 12 × 4KB = 48K│   │
  │  │ 1 single indirect pointer  → 1024 × 4KB    │   │
  │  │ 1 double indirect pointer  → 1024² × 4KB   │   │
  │  │ 1 triple indirect pointer  → 1024³ × 4KB   │   │
  │  └─────────────────────────────────────────────┘   │
  │  (ext4 uses extents instead — see below)           │
  └────────────────────────────────────────────────────┘
```

### Traditional Block Pointers (ext2/ext3)

```
  How an inode maps to data blocks (traditional):

  Inode
  ┌──────────────────┐
  │ direct[0]  ──────┼──────► Data Block 100  (4 KB)
  │ direct[1]  ──────┼──────► Data Block 205  (4 KB)
  │ ...              │
  │ direct[11] ──────┼──────► Data Block 312  (4 KB)
  │                  │        12 × 4KB = 48 KB directly
  │                  │
  │ single indirect ─┼──► ┌────────────────┐
  │                  │    │ Block 400:     │
  │                  │    │ [500][501]...  │──► 1024 data blocks
  │                  │    │ 1024 pointers  │    = 4 MB
  │                  │    └────────────────┘
  │                  │
  │ double indirect ─┼──► ┌────────────────┐
  │                  │    │ Block 600:     │
  │                  │    │ 1024 ptrs to   │──► 1024 indirect blocks
  │                  │    │ indirect blocks│    → 1024² data blocks
  │                  │    └────────────────┘    = 4 GB
  │                  │
  │ triple indirect ─┼──► (1024³ data blocks = 4 TB)
  └──────────────────┘

  Small files (≤48KB): All data accessible via direct pointers.
  Medium files (≤4MB): One level of indirection.
  Large files: Two or three levels of indirection.
```

### ext4 Extents (Modern Approach)

ext4 replaced individual block pointers with **extents** — each extent describes a **contiguous range** of blocks:

```c
struct ext4_extent {
    __le32 ee_block;    // first logical block number
    __le16 ee_len;      // number of blocks in this extent
    __le16 ee_start_hi; // upper 16 bits of physical block
    __le32 ee_start_lo; // lower 32 bits of physical block
};
```

A single extent can map up to 128 MB of contiguous blocks (32768 blocks × 4KB). For a contiguous 1 GB file, only ~8 extents are needed instead of 262,144 individual block pointers. Much more efficient for large files.

---

## 3. In-Memory Data Structures

When processes open and operate on files, the kernel maintains several in-memory data structures that work together:

```
  In-Memory File System Structures:

  Process A                    Process B
  ┌──────────────┐             ┌──────────────┐
  │ fd table     │             │ fd table     │
  │ 0: ──────────┼──┐          │ 0: ──────────┼──┐
  │ 1: ──────────┼──┤          │ 1: ──────────┼──┤
  │ 2: ──────────┼──┤          │ 2: ──────────┼──┤
  │ 3: ──────────┼──┼─┐        │ 3: ──────────┼──┼─┐
  └──────────────┘  │ │        └──────────────┘  │ │
                    │ │                          │ │
        ┌───────────┘ │            ┌─────────────┘ │
        ▼             ▼            ▼               ▼
  ┌─────────────────────────────────────────────────────┐
  │  System-Wide Open File Table                        │
  │  (one entry per open() call)                        │
  │                                                     │
  │  Entry 1:  offset=0,   flags=O_RDONLY, → inode 42  │
  │  Entry 2:  offset=100, flags=O_RDWR,  → inode 42  │
  │  Entry 3:  offset=0,   flags=O_RDONLY, → inode 78  │
  │  ...                                               │
  └────────────────────────┬────────────────────────────┘
                           │
                           ▼
  ┌─────────────────────────────────────────────────────┐
  │  In-Memory Inode Table (inode cache)                │
  │  (one entry per open file, regardless of how many   │
  │   fds point to it)                                  │
  │                                                     │
  │  Inode 42: type=file, size=8192, blocks=[100,101]  │
  │  Inode 78: type=file, size=4096, blocks=[200]      │
  │  ...                                               │
  └─────────────────────────────────────────────────────┘
```

### 3.1. Per-Process File Descriptor Array

Each process has its own array of file descriptors. Each entry is a pointer to an entry in the system-wide open file table. The array is stored in the process's `task_struct` → `files_struct`.

```c
// Kernel (simplified):
struct files_struct {
    struct fdtable *fdt;    // pointer to fd table
    // fdt->fd[0], fdt->fd[1], ... → pointers to struct file
};
```

### 3.2. System-Wide Open File Table

A **global** (system-wide) table. Each entry (a `struct file` in Linux) is created by an `open()` call and tracks:

| Field | Description |
|-------|-------------|
| `f_pos` | Current file **offset** (position for read/write) |
| `f_flags` | Access mode flags (`O_RDONLY`, `O_WRONLY`, `O_RDWR`, `O_APPEND`, ...) |
| `f_count` | Reference count (how many fds point to this entry) |
| `f_inode` | Pointer to the in-memory inode |
| `f_op` | Pointer to file operations table (VFS function pointers) |

### 3.3. In-Memory Inode Table (Inode Cache)

Caches on-disk inodes in memory for fast access. One entry per unique open file, regardless of how many processes have it open. The kernel's `inode_cache` is managed as a hash table for fast lookup.

---

## 4. How Data Structures Change After Key Operations

### After Two `open()` Calls to the Same File

```c
int fd1 = open("file.txt", O_RDONLY);   // fd 3 in Process A
int fd2 = open("file.txt", O_RDWR);     // fd 4 in Process A
```

```
  Process A fd table:
  3 ──────► Open File Entry 1 (offset=0, O_RDONLY) ──► Inode 42
  4 ──────► Open File Entry 2 (offset=0, O_RDWR)  ──► Inode 42

  Two separate open file entries (independent offsets),
  but SAME in-memory inode.
  Reading via fd3 advances Entry 1's offset but not Entry 2's.
```

### After `fork()`

`fork()` duplicates the fd table. Child's fds point to the **same** open file entries (offsets are shared!).

```c
int fd = open("file.txt", O_RDONLY);  // fd 3, offset=0
pid_t pid = fork();
// Both parent and child have fd 3 → same open file entry
```

```
  Parent fd table:                Child fd table:
  3 ──────────────┐               3 ──────────┐
                  ▼                            ▼
          Open File Entry 1 (offset=0, refcount=2)
                  │
                  ▼
              Inode 42

  Parent reads 10 bytes → offset becomes 10.
  Child's fd 3 also sees offset 10! (shared entry)
```

### After `dup()` / `dup2()`

`dup(fd)` creates a new fd pointing to the **same** open file entry. Used for I/O redirection.

```c
int fd = open("output.txt", O_WRONLY | O_CREAT, 0644);
dup2(fd, 1);   // fd 1 (stdout) now points to same entry as fd
close(fd);     // close the original fd; fd 1 still valid
// Now printf() writes to output.txt!
```

```
  fd table after dup2(fd, 1):

  0: ──► stdin entry
  1: ──────────┐
  3: ──────────┤  (both point to same open file entry)
               ▼
       Open File Entry (output.txt, offset=0, refcount=2)
```

This is exactly how the shell implements **I/O redirection** (`> output.txt`).

---

## 5. Disk Buffer Cache (Page Cache)

Reading from disk is **extremely slow** compared to memory (HDD: ~10ms, SSD: ~100μs, RAM: ~100ns). The kernel caches recently accessed disk blocks in memory to avoid repeated disk reads.

```
  Disk Buffer Cache (Page Cache):

  ┌─────────────────────────────────────────┐
  │  Main Memory (RAM)                      │
  │                                         │
  │  ┌───────────────────────────────────┐  │
  │  │  Page Cache                       │  │
  │  │  (cached disk blocks)             │  │
  │  │                                   │  │
  │  │  Block 100: [file data...]  clean │  │
  │  │  Block 101: [file data...]  dirty │  │
  │  │  Block 200: [dir entry...]  clean │  │
  │  │  Block 305: [inode data..] dirty  │  │
  │  │  ...                              │  │
  │  └───────────────────────────────────┘  │
  │                                         │
  └─────────────────┬───────────────────────┘
                    │
                    │  read miss: fetch from disk
                    │  write-back: flush dirty pages
                    ▼
  ┌─────────────────────────────────────────┐
  │  Disk                                   │
  └─────────────────────────────────────────┘
```

In Linux, the page cache and buffer cache are unified — the **page cache** caches file data (indexed by file + offset), and the **buffer cache** is a view into the page cache for block device access.

### Cache States

- **Clean**: Cached copy matches disk copy. Safe to evict.
- **Dirty**: Cached copy has been modified but **not yet written** back to disk. Must be flushed before eviction.

### Write Policies

| Policy | Behavior | Pros | Cons |
|--------|----------|------|------|
| **Write-through** | Every write goes to disk **immediately** (synchronously) | Data safe on disk quickly; crash-safe | Slow — every write waits for disk I/O |
| **Write-back** | Write only to cache; flush to disk **later** (asynchronously) | Fast — writes complete at memory speed | Data loss risk on crash; dirty pages must be flushed |

**Linux uses write-back by default.** Dirty pages are flushed by:
- **pdflush / writeback kernel threads**: Periodically flush dirty pages (default: every 30 seconds, or when dirty ratio exceeds threshold).
- **`sync()` / `fsync()` / `fdatasync()`**: Explicit flush by application.
- **Memory pressure**: When the system needs to reclaim pages, dirty ones are written back first.

```bash
# View page cache usage:
$ free -h
              total     used     free    shared  buff/cache   available
Mem:           16G      4.2G      1.8G     256M     10G         11G
#                                                   ^^^
#                                         Buffer/cache is the page cache

# View dirty page settings:
$ cat /proc/sys/vm/dirty_ratio             # % of RAM (default ~20%)
$ cat /proc/sys/vm/dirty_writeback_centisecs   # flush interval (default 500 = 5s)

# Force sync:
$ sync              # flush all dirty pages to disk
```

### Flush System Calls

```c
#include <unistd.h>

void sync(void);           // flush ALL dirty pages (all filesystems)
int fsync(int fd);         // flush all dirty data + metadata for fd to disk
int fdatasync(int fd);     // flush dirty data for fd (not all metadata)

// fsync is what databases (MySQL, PostgreSQL) use to guarantee durability.
// It's expensive — waits for the disk write to complete.
```

---

## 6. The Read System Call: Full Lifecycle

Pulling together file descriptors, inodes, the buffer cache, and device I/O:

```c
ssize_t n = read(fd, user_buf, 4096);
```

```
  read(fd, user_buf, 4096) — Complete Path:

  1. fd table[fd] ──► Open File Entry
                       ├── offset = 1000
                       └──► Inode (in-memory)
                             ├── size = 8192
                             └── block map: offset 1000 → disk block 205

  2. Check page cache: is block 205 cached?
     ┌─── YES (cache hit) ──────────────────────────────────────┐
     │  Copy 4096 bytes from cached page to user_buf.           │
     │  Update offset: 1000 → 5096.                             │
     │  Return 4096.  FAST PATH — no disk I/O!                  │
     └──────────────────────────────────────────────────────────┘

     ┌─── NO (cache miss) ──────────────────────────────────────┐
     │  3. Allocate a page in the page cache.                   │
     │  4. Device driver issues READ command to disk via DMA.   │
     │  5. Process P1 → BLOCKED (sleeping on I/O wait queue).   │
     │  6. OS context-switches to another process P2.           │
     │  7. Disk completes read → DMA copies block to page cache.│
     │  8. Disk raises interrupt → ISR marks P1 as READY.       │
     │  9. Scheduler resumes P1.                                │
     │  10. Copy 4096 bytes from page cache to user_buf.        │
     │  11. Update offset: 1000 → 5096.                         │
     │  12. Return 4096.                                        │
     └──────────────────────────────────────────────────────────┘
```

**Important**: The actual bytes returned may be less than requested (partial read). For example:
- Offset 7000, request 4096 bytes, but file size is 8192 → only 1192 bytes available → return 1192.
- `read()` at EOF → return 0.
- Interrupted by signal → return partial count or `-1` with `errno = EINTR`.

---

## 7. The Write System Call: Full Lifecycle

```c
ssize_t n = write(fd, data_buf, 4096);
```

```
  write(fd, data_buf, 4096) — Complete Path:

  1. fd table[fd] ──► Open File Entry
                       ├── offset = 1000
                       └──► Inode (in-memory)

  2. Determine target block(s) from offset.
     Writing to offset 1000, block size 4096 → block #0 (offset 0-4095).

  3. Writing beyond end of file?
     ┌─── YES ──────────────────────────────────────────────────┐
     │  Allocate new data block(s) on disk:                     │
     │  - Update data bitmap (mark block as used)               │
     │  - Add block number to inode's block map                 │
     │  - Update inode's size field                             │
     │  (All updates initially in memory — buffer cache)        │
     └──────────────────────────────────────────────────────────┘

  4. Is the target block in page cache?
     ┌─── NO ───────────────────────────────────────────────────┐
     │  Read block from disk into page cache first.             │
     │  (Can't just overwrite partial block without reading it) │
     └──────────────────────────────────────────────────────────┘

  5. Copy data_buf into the page cache block(s).
     Mark cache page(s) as DIRTY.

  6. Write-back policy:
     ┌─── Write-through ──► Immediately write to disk (slow).
     │                       Return after disk write completes.
     └─── Write-back ─────► Return immediately (fast).
                             Dirty pages flushed later by kernel.

  7. Update offset: 1000 → 5096. Return bytes written.
```

---

## 8. Memory-Mapped Files (`mmap`)

Instead of using `read()`/`write()` system calls, a file can be mapped directly into a process's virtual address space. Accessing the file becomes regular memory load/store operations.

```c
#include <sys/mman.h>

void *addr = mmap(NULL, length, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd, offset);
// addr now points to file contents in memory.
// Reading *addr reads from the file.
// Writing *addr writes to the file.

munmap(addr, length);
```

```
  mmap: File Mapped into Virtual Address Space:

  Process Virtual Address Space:
  ┌──────────────────────────┐
  │  Code segment            │
  │  Data segment            │
  │  Heap                    │
  │  ...                     │
  │  ┌──────────────────┐   │
  │  │  mmap region      │   │ ◄── virtual pages backed by file
  │  │  0x7f0000000000   │   │     on disk (via page cache)
  │  │  ... file data .. │   │
  │  └──────────────────┘   │
  │  Stack                  │
  └──────────────────────────┘
         │
         │  Page fault on first access
         ▼
  ┌──────────────────┐
  │  Page Cache      │ ◄── file blocks cached here
  └────────┬─────────┘
           │
           ▼
  ┌──────────────────┐
  │  Disk (file)     │
  └──────────────────┘
```

### How mmap Works

1. `mmap()` sets up the virtual → physical mapping in the process's page table, but **doesn't load data yet** (lazy / demand paging).
2. First access to a mapped page → **page fault** → kernel loads the page from disk (via page cache).
3. Subsequent accesses are direct memory operations — no syscall overhead.
4. `MAP_SHARED` changes are visible to other processes mapping the same file and are eventually written back to disk.
5. `MAP_PRIVATE` uses copy-on-write — changes are private to the process and not written to disk.

### `mmap` vs. `read()`/`write()`

| Aspect | `read()`/`write()` | `mmap` |
|--------|-------------------|--------|
| Data path | User ↔ kernel copy (buffer cache → user buf) | Direct access via page table (no copy) |
| Syscall per access | Yes (one syscall per read/write) | No (memory operations, faults only on first access) |
| Good for | Sequential I/O, small reads | Random access, large files, shared memory |
| Complexity | Simple | Must manage mapping, alignment, `munmap` |
| Used by | Most applications | Databases (mmap'd files), shared libraries, `malloc` for large allocations |

---

## 9. Crash Consistency

### The Problem

A single filesystem operation (e.g., appending data to a file) updates **multiple disk blocks**:
- The **data block** (new content).
- The **inode** (update size, add block pointer).
- The **data bitmap** (mark new block as allocated).

These updates happen first in memory (page cache). Even with write-through, they are written to disk as **separate I/O operations** — the disk can only write one block atomically. If a crash (power failure, kernel panic) happens in the middle:

```
  Appending data to a file updates 3 blocks:

  ┌─────────┐  ┌─────────┐  ┌─────────┐
  │  Data   │  │  Inode  │  │  Data   │
  │  Block  │  │  Block  │  │  Bitmap │
  └────┬────┘  └────┬────┘  └────┬────┘
       │            │            │
  ─────┼────────────┼────────────┼──── time ──►
       │            │            │
       ▼            │            │
    CRASH!          │            │
                    │            │
  Only the data block was written.
  Inode doesn't point to it. Bitmap doesn't mark it.
  → Orphaned data block (space leak).
```

### Possible Inconsistencies After Crash

| What was written | What was NOT written | Inconsistency |
|-----------------|---------------------|---------------|
| Data block | Inode, bitmap | Written data is lost (inode doesn't reference it). Space leak. |
| Inode (new block pointer) | Data block | File contains **garbage** (inode points to unwritten block). |
| Inode + bitmap | Data block | Garbage data + bitmap says block is used. |
| Bitmap | Inode, data block | Space leak (bitmap says used, but no file references it). |
| Data + bitmap | Inode | Data written, space allocated, but file doesn't reference it. |

### The Core Issue

Individual block writes are atomic (a single 512-byte sector write completes fully or not at all). But **multi-block updates are NOT atomic** — the crash can happen between any two block writes, leaving the filesystem in an inconsistent state.

---

## 10. Filesystem Checkers (`fsck`)

The **oldest** approach to crash recovery: after an unclean shutdown, scan the entire filesystem and fix inconsistencies.

```bash
$ sudo fsck /dev/sda1
# - Checks inode link counts vs. directory references
# - Checks bitmap consistency vs. actual inode block pointers
# - Finds orphaned inodes (allocated but unreferenced)
# - Fixes directory entries, frees leaked blocks
```

**Problems with fsck**:
- **Very slow** for large filesystems — must scan every inode and block.
- On a multi-terabyte filesystem, fsck can take **hours**.
- Only runs after a crash (not preventive).
- Cannot recover lost data — only structural consistency.

---

## 11. Journaling (Write-Ahead Logging)

The modern solution for crash consistency. Used by ext3, ext4, XFS, NTFS, and most production filesystems.

### Core Idea

Before making any changes to the actual filesystem blocks, write a **log** (journal) of all intended changes to a dedicated area on disk. Only after the log is safely on disk, apply the changes to the actual blocks.

### The Journaling Protocol

```
  Journaling: Write-Ahead Log

  Step 1: LOG the changes
  ┌─────────────────────────────────────────────┐
  │  Journal (on-disk log area):                │
  │  ┌──────┬────────┬────────┬────────┬──────┐ │
  │  │ TXN  │ inode  │ bitmap │ data   │COMMIT│ │
  │  │BEGIN │ change │ change │ change │      │ │
  │  └──────┴────────┴────────┴────────┴──────┘ │
  └─────────────────────────────────────────────┘

  Step 2: COMMIT (write commit block — the point of no return)

  Step 3: CHECKPOINT — apply changes to actual disk locations
  ┌─────────────────────────────────────────────┐
  │  Actual filesystem blocks:                  │
  │  Inode block ← updated                     │
  │  Data bitmap ← updated                     │
  │  Data block  ← written                     │
  └─────────────────────────────────────────────┘

  Step 4: Free the journal entries (transaction complete)
```

### Crash Recovery with Journaling

| Crash happens... | Recovery action |
|-----------------|-----------------|
| Before COMMIT written | Discard journal entries. No changes applied. It's as if the operation never happened. Safe. |
| After COMMIT but before checkpoint | **Replay** the journal: apply all logged changes to actual blocks. All changes complete. Safe. |
| After checkpoint | Everything already on disk. Clear journal. Safe. |

```
  Crash Recovery Decision:

  Was the journal entry committed?
       │
       ├── NO  → Discard. No changes made.
       │         Operation "never happened."
       │
       └── YES → Replay journal entries.
                  Apply all changes to actual blocks.
                  Operation completes fully.

  Result: The filesystem is ALWAYS consistent.
  Either the full operation happened or none of it did.
  This is the ATOMICITY guarantee.
```

### Journaling Modes (ext4)

| Mode | What's Journaled | Performance | Safety |
|------|-----------------|-------------|--------|
| **journal** | Data + metadata | Slowest (all writes go to journal first) | Most safe |
| **ordered** (default) | Metadata only; data written before metadata journal commit | Good balance | Data may be stale but metadata consistent |
| **writeback** | Metadata only; no ordering of data writes | Fastest | Data may contain garbage after crash |

```bash
# Check current journal mode:
$ sudo tune2fs -l /dev/sda1 | grep "Journal features"

# Mount with specific mode:
$ mount -o data=ordered /dev/sda1 /mnt    # default
$ mount -o data=journal /dev/sda1 /mnt    # safest
$ mount -o data=writeback /dev/sda1 /mnt  # fastest
```

---

## 12. Virtual File System (VFS)

Linux supports many different filesystem implementations (ext4, XFS, Btrfs, FAT, NFS, procfs, sysfs, ...). How does the kernel handle this diversity without duplicating code?

### The Problem

Different filesystems have different on-disk layouts, different inode structures, different directory implementations, different journaling strategies. Yet `open()`, `read()`, `write()` must work the same way regardless of the underlying filesystem.

### The Solution: VFS Abstraction Layer

The **Virtual File System** is an abstraction layer that defines a set of **objects** and **operations**. Each concrete filesystem implements these operations. The rest of the kernel (and user programs) interact only with the VFS interface.

```
  VFS Architecture:

  User Space
  ──────────────────────────────────────────────────
  │  Application:  open(), read(), write(), ...    │
  ──────────────────────────────────────────────────
  │  System Call Layer:  sys_open, sys_read, ...   │
  ├────────────────────────────────────────────────┤
  │                                                │
  │  VFS Layer (Virtual File System)               │
  │  ┌──────────────────────────────────────────┐  │
  │  │  VFS Objects:                            │  │
  │  │  - superblock (filesystem metadata)      │  │
  │  │  - inode (file metadata)                 │  │
  │  │  - dentry (directory entry / pathname)   │  │
  │  │  - file (open file state)                │  │
  │  │                                          │  │
  │  │  Each object has an operations table:    │  │
  │  │  - super_operations (mount, unmount)     │  │
  │  │  - inode_operations (create, lookup)     │  │
  │  │  - file_operations (read, write, seek)   │  │
  │  │  - dentry_operations (compare, hash)     │  │
  │  └──────────────────────────────────────────┘  │
  │         │              │             │         │
  │         ▼              ▼             ▼         │
  │  ┌──────────┐   ┌──────────┐  ┌──────────┐   │
  │  │  ext4    │   │   XFS    │  │  procfs   │   │
  │  │  driver  │   │  driver  │  │  driver   │   │
  │  └────┬─────┘   └────┬─────┘  └──────────┘   │
  │       │              │         (no disk,       │
  │       │              │          in-memory)     │
  ├───────┼──────────────┼─────────────────────────┤
  │  Block Layer / Page Cache                      │
  ├────────────────────────────────────────────────┤
  │  Device Drivers                                │
  ├────────────────────────────────────────────────┤
  │  Hardware                                      │
  └────────────────────────────────────────────────┘
```

### VFS Objects and Their Operations

| VFS Object | Represents | Key Operations |
|-----------|------------|----------------|
| `super_block` | A mounted filesystem | `alloc_inode`, `destroy_inode`, `write_super`, `sync_fs` |
| `inode` | A file or directory (metadata) | `create`, `lookup`, `link`, `unlink`, `mkdir`, `rename` |
| `dentry` | A directory entry (path component) | `d_compare`, `d_hash`, `d_release` |
| `file` | An open file (per-process state) | `read`, `write`, `llseek`, `mmap`, `open`, `release` |

### How It Works

When you call `read(fd, buf, n)`:

1. Kernel finds the `struct file` from the fd table.
2. `struct file` has a pointer to `file_operations`.
3. `file_operations->read` points to the **specific filesystem's** read function (e.g., `ext4_file_read_iter`).
4. The filesystem's function translates the request into block-level operations.

```c
// Simplified kernel VFS dispatch:
ssize_t vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
    // Call the filesystem-specific read function:
    return file->f_op->read(file, buf, count, pos);
    // f_op was set when the file was opened, based on the filesystem type.
}
```

This is **polymorphism via function pointers** — the C equivalent of virtual methods.

### Why VFS Matters

- **Modularity**: New filesystems can be added as kernel modules without changing any existing code.
- **Uniformity**: User programs use the same syscalls regardless of filesystem.
- **Extensibility**: procfs, sysfs, devtmpfs, FUSE (user-space filesystems) all plug into VFS.

---

## 13. Data Storage on Disk

### Disk Interface

Physical disks expose a simple interface: an array of **sectors** (usually 512 bytes each), each of which can be read or written atomically.

```
  Disk: Array of Sectors

  ┌────────┬────────┬────────┬────────┬────────┬────────┐
  │Sector 0│Sector 1│Sector 2│Sector 3│  ...   │Sector N│
  │ 512 B  │ 512 B  │ 512 B  │ 512 B  │        │ 512 B  │
  └────────┴────────┴────────┴────────┴────────┴────────┘

  Each sector read/write is ATOMIC.
  Multi-sector operations are NOT atomic.
```

### Data Integrity

Disks store data with **error detection/correction** (ECC) bits:
- Can correct random bit flips (soft errors).
- Can detect (but not always correct) larger corruption.
- Disk controller or OS firmware can handle some errors (e.g., remapping bad sectors to spare sectors — "sector blacklisting").
- Unrecoverable errors are reported as disk failures to the OS.

```bash
# View disk health (SMART):
$ sudo smartctl -a /dev/sda
# Shows: Reallocated Sector Count, Current Pending Sector, etc.

# Check for bad blocks:
$ sudo badblocks -v /dev/sda1
```

### RAID (Redundant Array of Inexpensive Disks)

For production systems, single disk failures are unacceptable. RAID replicates data across multiple disks for **reliability** and/or **performance**:

| RAID Level | Description | Min Disks | Redundancy | Performance |
|-----------|-------------|-----------|------------|-------------|
| RAID 0 | Striping (split data across disks) | 2 | None (any disk fails → all data lost) | Best read/write |
| RAID 1 | Mirroring (duplicate data) | 2 | Survives 1 disk failure | Good read, normal write |
| RAID 5 | Striping + distributed parity | 3 | Survives 1 disk failure | Good read, slower write (parity calc) |
| RAID 6 | Striping + double parity | 4 | Survives 2 disk failures | Good read, slowest write |
| RAID 10 | Mirror + stripe (RAID 1+0) | 4 | Survives 1 per mirror pair | Best read, good write |

```bash
# View Linux software RAID:
$ cat /proc/mdstat
md0 : active raid1 sda1[0] sdb1[1]
      1048576 blocks [2/2] [UU]

# Manage with mdadm:
$ sudo mdadm --detail /dev/md0
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| On-disk layout | Superblock + bitmaps + inode table + data blocks | `dumpe2fs`, `tune2fs` |
| Inode | Stores all file metadata + data block pointers | Traditional pointers (ext2) or extents (ext4) |
| fd table → open file table → inode | Three-level indirection for file access | `struct files_struct` → `struct file` → `struct inode` |
| `fork()` + fd | Child shares open file entries (offsets shared) | `refcount` on `struct file` |
| `dup2()` | Two fds → same open file entry (I/O redirection) | Shell `>`, `2>&1` |
| Page cache | Caches disk blocks in RAM; write-back default | `free -h`, `/proc/sys/vm/dirty_*` |
| `fsync()` | Force dirty pages for a file to disk | Databases use for durability |
| Read syscall | fd → open file → inode → page cache → (disk if miss) → user buffer | May block on I/O |
| Write syscall | User buffer → page cache (dirty) → disk (later) | Write-back unless O_SYNC |
| `mmap` | Map file into virtual address space; demand-paged | `mmap()`, page fault loads data |
| Crash consistency | Multi-block updates not atomic → inconsistency on crash | Partially written metadata |
| `fsck` | Post-crash full scan and repair | Slow for large filesystems |
| Journaling | Write-ahead log → commit → checkpoint | ext4: `ordered` (default), `journal`, `writeback` |
| VFS | Abstraction layer: uniform interface for all filesystems | `file_operations`, `inode_operations` |
| RAID | Replicate across disks for reliability + performance | `mdadm`, `/proc/mdstat` |
