# Files and Directories

---

## 1. File System Overview

A **file system** is the OS subsystem that organizes, stores, and retrieves data on persistent storage devices (hard disks, SSDs). It provides the abstraction of **files** and **directories** on top of raw disk blocks.

```
  The Abstraction:

  What the user sees:            What's on disk:
  ┌──────────────────┐           ┌──────────────────────┐
  │ /home/              │           │ Block 0: superblock   │
  │ ├── alice/          │           │ Block 1: inode bitmap │
  │ │   ├── notes.txt   │ ◄─────── │ Block 2: data bitmap  │
  │ │   └── photo.jpg   │  mapping  │ Block 3-10: inodes    │
  │ └── bob/            │           │ Block 11-99: data     │
  │     └── code.c      │           │ ...                   │
  └──────────────────┘           └──────────────────────┘

  Files and directories are an ABSTRACTION.
  On disk, everything is just numbered blocks (typically 512B sectors
  or 4KB filesystem blocks).
```

Key insight: **Disks expose a flat array of blocks.** The file system creates the illusion of a hierarchical namespace (files in directories) on top of this flat block store.

### What the Filesystem Does

1. Translates file operations (`read`, `write`, `open`) into disk block reads and writes.
2. Tracks which blocks belong to which file (metadata).
3. Tracks free/used blocks and inodes (allocation).
4. Organizes files into a directory hierarchy.
5. Ensures persistence — data survives power cycles.

---

## 2. File Abstraction

### What Is a File?

A **file** is a named, persistent sequence of bytes stored on secondary storage. The OS imposes no structure on file contents — it's just a byte array. Applications interpret the bytes (text, binary, image, executable, etc.).

| Attribute | Description |
|-----------|-------------|
| Name | Human-readable identifier (`notes.txt`) |
| Inode number | Unique numeric identifier within the filesystem |
| Size | Number of bytes |
| Owner / Group | UID, GID |
| Permissions | Read, write, execute for owner/group/others |
| Timestamps | Access time (atime), modification time (mtime), change time (ctime) |
| Data block pointers | Which disk blocks hold the file's content |

### Steps to Access a File

All file access follows a three-step pattern:

```c
// 1. Open: get a file descriptor (handle)
int fd = open("/home/alice/notes.txt", O_RDONLY);
// fd is a small integer (e.g., 3)

// 2. Read/Write: use the file descriptor
char buf[4096];
ssize_t n = read(fd, buf, sizeof(buf));
// or: write(fd, data, len);

// 3. Close: release the file descriptor
close(fd);
```

The **file descriptor** (`fd`) is a handle — a small non-negative integer that the process uses to refer to an open file in subsequent syscalls. It's an index into the process's file descriptor table.

---

## 3. File Descriptors in Depth

Every process has an array of **file descriptors**. The first three are pre-assigned:

```
  Process File Descriptor Table:

  Index  │  Points to
  ───────┼──────────────────
    0    │  stdin  (standard input)
    1    │  stdout (standard output)
    2    │  stderr (standard error)
    3    │  /home/alice/notes.txt (opened by user)
    4    │  socket to 10.0.0.1:80
    5    │  pipe (read end)
   ...   │  ...
```

File descriptors are **not just for files** — they are a uniform handle for:
- Regular files
- Directories
- Sockets (network connections)
- Pipes (IPC)
- Device files (`/dev/*`)
- epoll instances, signalfd, timerfd, etc.

This is the UNIX philosophy: **"everything is a file"** — or more precisely, everything is a file descriptor. A uniform interface (`read`, `write`, `close`) works on all of these.

```bash
# View open file descriptors of a process:
$ ls -la /proc/<pid>/fd/
lrwx------ 1 alice alice 64 ... 0 -> /dev/pts/0     # stdin
lrwx------ 1 alice alice 64 ... 1 -> /dev/pts/0     # stdout
lrwx------ 1 alice alice 64 ... 2 -> /dev/pts/0     # stderr
lr-x------ 1 alice alice 64 ... 3 -> /home/alice/notes.txt
lrwx------ 1 alice alice 64 ... 4 -> socket:[12345]

# Count open fds:
$ ls /proc/<pid>/fd | wc -l

# System-wide limit:
$ cat /proc/sys/fs/file-max
```

---

## 4. Read/Write System Calls and File Offsets

### read() and write()

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
// Reads up to 'count' bytes from fd into buf.
// Returns: number of bytes actually read (may be less than count),
//          0 on EOF, -1 on error.

ssize_t write(int fd, const void *buf, size_t count);
// Writes up to 'count' bytes from buf to fd.
// Returns: number of bytes actually written, -1 on error.
```

### File Offset (Current Position)

Every open file descriptor has an associated **offset** — the position in the file where the next read or write will occur.

```
  File contents: [A B C D E F G H I J K L M]
                  0 1 2 3 4 5 6 7 8 9 ...

  open() → offset = 0

  read(fd, buf, 4) → reads "ABCD", offset advances to 4
  read(fd, buf, 3) → reads "EFG",  offset advances to 7
  read(fd, buf, 100) → reads "HIJKLM", returns 6 (hit EOF), offset = 13
  read(fd, buf, 1) → returns 0 (EOF)
```

The offset advances automatically after each read/write. You can also set it explicitly:

```c
#include <unistd.h>

off_t lseek(int fd, off_t offset, int whence);
// whence:
//   SEEK_SET — set offset to 'offset' bytes from beginning
//   SEEK_CUR — set offset to current + 'offset'
//   SEEK_END — set offset to end of file + 'offset'

lseek(fd, 0, SEEK_SET);   // rewind to beginning
lseek(fd, 100, SEEK_SET); // jump to byte 100
lseek(fd, -10, SEEK_END); // 10 bytes before end of file
```

### Independent Offsets per File Descriptor

If you open the same file twice, you get **two independent file descriptors**, each with its own offset:

```c
int fd1 = open("file.txt", O_RDONLY);  // fd1 has offset 0
int fd2 = open("file.txt", O_RDONLY);  // fd2 has offset 0 (independent)

read(fd1, buf, 10);  // fd1 offset → 10
read(fd2, buf, 5);   // fd2 offset → 5 (not affected by fd1's read)
```

This is because each `open()` creates a new **open file description** (entry in the kernel's open file table) with its own offset. Two `fd`s pointing to the same open file description share the offset (this happens with `dup()` and `fork()` — more on this in the File Systems note).

---

## 5. Directory Tree

Files and directories are organized in a **tree** rooted at `/` (the root directory):

```
  /                           (root)
  ├── bin/                    (essential binaries)
  ├── etc/                    (configuration files)
  ├── home/                   (user home directories)
  │   ├── alice/
  │   │   ├── notes.txt
  │   │   └── photo.jpg
  │   └── bob/
  │       └── code.c
  ├── proc/                   (virtual filesystem — process info)
  ├── dev/                    (device files)
  ├── tmp/                    (temporary files)
  └── var/                    (variable data — logs, etc.)
```

### Paths

- **Absolute path**: Starts from root `/`. E.g., `/home/alice/notes.txt`.
- **Relative path**: Relative to the current working directory. E.g., `notes.txt` if cwd is `/home/alice/`.
- **`.`**: Current directory.
- **`..`**: Parent directory.

---

## 6. Inodes

Every file (and directory) in a filesystem is identified by a unique **inode number**. The inode is the on-disk data structure that stores all **metadata** about a file — everything except the file name and its contents.

```
  Inode (on-disk structure):

  ┌────────────────────────────────────────┐
  │  Inode #: 42                           │
  │  Type:    regular file                 │
  │  Size:    8192 bytes                   │
  │  Owner:   uid=1000, gid=1000           │
  │  Permissions: rw-r--r--                │
  │  Timestamps:                           │
  │    atime: 2025-04-28 10:00:00          │
  │    mtime: 2025-04-27 15:30:00          │
  │    ctime: 2025-04-27 15:30:00          │
  │  Link count: 1                         │
  │  Data block pointers:                  │
  │    [100] [101] → 2 blocks = 8192 bytes │
  └────────────────────────────────────────┘
```

**Key insight**: The file name is NOT stored in the inode. Names are stored in **directories** — a directory maps names to inode numbers.

```bash
# View inode number:
$ ls -i notes.txt
42 notes.txt

# View inode details:
$ stat notes.txt
  File: notes.txt
  Size: 8192       Blocks: 16     IO Block: 4096   regular file
Device: 801h/2049d  Inode: 42      Links: 1
Access: (0644/-rw-r--r--)  Uid: (1000/alice)  Gid: (1000/alice)
Access: 2025-04-28 10:00:00
Modify: 2025-04-27 15:30:00
Change: 2025-04-27 15:30:00
```

---

## 7. Directories

A **directory** is a special type of file whose contents are a **mapping** from file names to inode numbers.

```
  Directory "/home/alice/" (inode #20):

  ┌───────────────────────────┐
  │  Name          │ Inode #  │
  │────────────────┼──────────│
  │  .             │   20     │  (self)
  │  ..            │   15     │  (parent: /home/)
  │  notes.txt     │   42     │
  │  photo.jpg     │   43     │
  │  projects/     │   44     │  (subdirectory)
  └───────────────────────────┘
```

Every directory contains at least two special entries:
- `.` — points to itself (inode 20 in this example).
- `..` — points to the parent directory (inode 15).

### Directory Operations

Directories can be accessed like files:

```c
#include <dirent.h>

DIR *dir = opendir("/home/alice");
struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
    printf("Name: %s, Inode: %lu\n", entry->d_name, entry->d_ino);
}
closedir(dir);
```

```c
// Other directory syscalls:
mkdir("/home/alice/newdir", 0755);    // create directory
rmdir("/home/alice/newdir");          // remove empty directory
rename("old_name", "new_name");       // rename/move
```

### Pathname Resolution

When you open `/home/alice/notes.txt`, the kernel must resolve this path step by step:

```
  Resolving /home/alice/notes.txt:

  1. Start at root inode (inode 2, by convention)
  2. Read root directory data → find "home" → inode 15
  3. Read inode 15 → it's a directory
  4. Read /home/ directory data → find "alice" → inode 20
  5. Read inode 20 → it's a directory
  6. Read /home/alice/ directory data → find "notes.txt" → inode 42
  7. Read inode 42 → it's a regular file
  8. Now we have the file's inode — can read its data blocks

  Each "/" in the path requires a directory lookup:
  reading the directory's data blocks and scanning for the name.
```

This means **long paths are expensive** — each component requires reading a directory's data from disk (though the disk buffer cache mitigates this).

---

## 8. Hard Links

A **hard link** creates another name (directory entry) pointing to the **same inode**. The inode's **link count** tracks how many names point to it.

```c
// Create a hard link:
link("/home/alice/notes.txt", "/home/alice/backup.txt");
// or from command line:
// ln notes.txt backup.txt
```

```
  After creating a hard link:

  /home/alice/ directory:
  ┌──────────────────┬──────────┐
  │  notes.txt       │ inode 42 │ ──┐
  │  backup.txt      │ inode 42 │ ──┤  Same inode!
  │  photo.jpg       │ inode 43 │   │
  └──────────────────┴──────────┘   │
                                    ▼
                               ┌──────────┐
                               │ Inode 42 │
                               │ links: 2 │  ← link count = 2
                               │ size: 8K │
                               │ blocks..│
                               └──────────┘
```

Key properties:
- Both names are **equally valid** — there's no "original" and "copy."
- Deleting one name (`unlink("notes.txt")`) decrements the link count. The inode and its data blocks are only freed when the link count reaches 0.
- **You can only unlink** — the OS decides when to actually delete the data.
- Hard links cannot span filesystem boundaries (different filesystems have separate inode namespaces).
- Hard links to directories are generally not allowed (would create cycles in the directory tree, breaking pathname resolution and tools like `find`).

```bash
$ echo "hello" > notes.txt
$ ln notes.txt backup.txt
$ ls -li notes.txt backup.txt
42 -rw-r--r-- 2 alice alice 6 ... notes.txt
42 -rw-r--r-- 2 alice alice 6 ... backup.txt
#  ^^                ^
#  same inode        link count = 2

$ rm notes.txt           # unlink: link count 2 → 1
$ cat backup.txt         # still works! data not deleted
hello

$ rm backup.txt          # unlink: link count 1 → 0 → data freed
```

### `unlink()` System Call

The system call to remove a file is actually `unlink()`, not "delete":

```c
#include <unistd.h>
int unlink(const char *pathname);
// Removes the directory entry for 'pathname'.
// Decrements the inode's link count.
// If link count reaches 0 AND no process has the file open → free data blocks.
// If link count is 0 BUT a process still has it open → defer deletion until close.
```

---

## 9. Symbolic (Soft) Links

A **symbolic link** (symlink) is a special file that stores the **pathname** of another file as its contents.

```c
// Create a symbolic link:
symlink("/home/alice/notes.txt", "/home/alice/shortcut.txt");
// or from command line:
// ln -s notes.txt shortcut.txt
```

```
  Symbolic link:

  /home/alice/ directory:
  ┌──────────────────┬──────────┐
  │  notes.txt       │ inode 42 │ ──────────► actual file data
  │  shortcut.txt    │ inode 99 │ ──► inode 99 contents:
  └──────────────────┴──────────┘     "/home/alice/notes.txt"
                                       (just a string!)
```

### Hard Link vs. Symbolic Link

| Aspect | Hard Link | Symbolic Link |
|--------|----------|---------------|
| What it stores | Same inode number | Pathname string |
| Inode | Shares inode with target | Has its own inode |
| Link count | Incremented on target inode | Not affected |
| Cross-filesystem | No | Yes |
| Target deleted | Data still accessible via hard link | **Dangling reference** — link points to nonexistent path |
| Links to directories | Generally not allowed | Allowed |

```bash
# Symbolic link becomes dangling:
$ ln -s notes.txt shortcut.txt
$ rm notes.txt
$ cat shortcut.txt
cat: shortcut.txt: No such file or directory   # dangling!

$ ls -l shortcut.txt
lrwxrwxrwx 1 alice alice 9 ... shortcut.txt -> notes.txt  # target gone
```

---

## 10. Mounting a Filesystem

**Mounting** connects a filesystem (stored on a device or partition) to a specific point in the directory tree.

```bash
# Mount a filesystem:
$ mount /dev/sdb1 /mnt/usb
# Now files on /dev/sdb1 are accessible under /mnt/usb/

# List mounted filesystems:
$ mount
/dev/sda1 on / type ext4 (rw,relatime)
/dev/sdb1 on /mnt/usb type vfat (rw,relatime)
tmpfs on /tmp type tmpfs (rw,nosuid,nodev)
proc on /proc type proc (rw,nosuid,nodev,noexec,relatime)

# Unmount:
$ umount /mnt/usb
```

```
  Before mounting:                   After mounting:

  /                                  /
  ├── home/                          ├── home/
  ├── mnt/                           ├── mnt/
  │   └── usb/  (empty)              │   └── usb/  ── mounted! ──┐
  └── ...                            └── ...                     │
                                                                  ▼
                                     ┌───────────────────────────┐
                                     │ /dev/sdb1 filesystem:     │
                                     │ ├── photos/               │
                                     │ │   └── vacation.jpg      │
                                     │ └── music/                │
                                     │     └── song.mp3          │
                                     └───────────────────────────┘
```

The mount point (`/mnt/usb/`) becomes the root of the mounted filesystem. Accessing `/mnt/usb/photos/vacation.jpg` reads from the filesystem on `/dev/sdb1`.

```bash
# Common virtual/pseudo filesystems mounted at boot:
# /proc   — process and kernel info (procfs)
# /sys    — kernel objects and device info (sysfs)
# /dev    — device nodes (devtmpfs)
# /tmp    — temporary files in RAM (tmpfs)

# View filesystem types and mount options:
$ cat /proc/mounts
$ findmnt            # tree view of mount points
$ df -Th             # disk usage with filesystem types
```

### The `/etc/fstab` File

Filesystems to mount at boot are specified in `/etc/fstab`:

```
# /etc/fstab
# <device>    <mount point>   <type>  <options>        <dump> <pass>
/dev/sda1     /               ext4    defaults         0      1
/dev/sda2     /home           ext4    defaults         0      2
/dev/sdb1     /mnt/data       xfs     noatime          0      0
tmpfs         /tmp            tmpfs   defaults,noatime 0      0
```

---

## Summary

| Concept | Key Idea | Linux Detail |
|---------|----------|--------------|
| File | Named persistent byte sequence on disk | `open()`, `read()`, `write()`, `close()` |
| File descriptor | Integer handle for an open file/socket/pipe | Per-process table; `ls /proc/<pid>/fd/` |
| File offset | Current position for read/write; advances automatically | `lseek()` to reposition |
| Inode | On-disk metadata structure (size, owner, block pointers). No name. | `stat()`, `ls -i`, unique per filesystem |
| Directory | Special file mapping names → inode numbers | `opendir()`, `readdir()`, contains `.` and `..` |
| Hard link | Another name for the same inode; link count tracks references | `link()` / `ln`; data freed when count=0 |
| Symbolic link | File storing a pathname string; can dangle | `symlink()` / `ln -s`; cross-filesystem |
| unlink | Removes a directory entry; OS deletes data when link count = 0 | `unlink()`; not "delete" |
| Mounting | Attaches a filesystem to a directory tree point | `mount`, `/etc/fstab`, `findmnt` |
