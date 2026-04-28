# Containers and Container Orchestration

---

## 1. Containers Overview

A **container** is a lightweight, isolated execution environment that shares the host OS kernel. Unlike a VM which virtualizes an entire machine (CPU, memory, devices, full OS), a container virtualizes only the **OS environment** — the filesystem, process tree, network stack, and user/group IDs.

```
  Virtual Machines:                    Containers:

  ┌──────┐ ┌──────┐ ┌──────┐         ┌──────┐ ┌──────┐ ┌──────┐
  │ App  │ │ App  │ │ App  │         │ App  │ │ App  │ │ App  │
  ├──────┤ ├──────┤ ├──────┤         │ Libs │ │ Libs │ │ Libs │
  │ Libs │ │ Libs │ │ Libs │         └──┬───┘ └──┬───┘ └──┬───┘
  ├──────┤ ├──────┤ ├──────┤            │        │        │
  │Guest │ │Guest │ │Guest │         ┌──┴────────┴────────┴──┐
  │  OS  │ │  OS  │ │  OS  │         │   Container Runtime   │
  │(full │ │(full │ │(full │         │   (runc, containerd)  │
  │kernel)│ │kernel)│ │kernel)│       └──────────┬────────────┘
  └──┬───┘ └──┬───┘ └──┬───┘                   │
     │        │        │              ┌──────────┴────────────┐
  ┌──┴────────┴────────┴──┐          │    Host OS Kernel      │
  │      Hypervisor        │          │    (SHARED — one       │
  └──────────┬─────────────┘          │     kernel for all)    │
  ┌──────────┴─────────────┐          └──────────┬────────────┘
  │    Physical Hardware    │          ┌──────────┴────────────┐
  └────────────────────────┘          │    Physical Hardware    │
                                      └────────────────────────┘

  VMs: Each has its own kernel            Containers: Share the host kernel
  (hundreds of MB per VM)                 (just the app + its dependencies)
```

### Containers vs. VMs

| Property | Virtual Machine | Container |
|----------|----------------|-----------|
| **Isolation level** | Hardware-level (separate kernel) | OS-level (shared kernel) |
| **Startup time** | Seconds to minutes (boot full OS) | Milliseconds to seconds |
| **Image size** | GBs (full OS + apps) | MBs (app + libs only) |
| **Resource overhead** | High (duplicate kernel, drivers per VM) | Low (shared kernel, minimal overhead) |
| **Density** | Tens of VMs per host | Hundreds to thousands of containers |
| **Security isolation** | Strong (hardware boundary via hypervisor) | Weaker (kernel bugs can escape) |
| **OS flexibility** | Any OS (Linux VM on Linux host, Windows VM, etc.) | Must match host kernel (Linux containers on Linux) |
| **Use case** | Multi-tenant cloud, running different OSes | Microservices, CI/CD, application packaging |

---

## 2. How Containers Work: Linux Kernel Primitives

Containers are NOT a single kernel feature. They are a **combination of several independent Linux kernel mechanisms** working together:

```
  Container = Namespaces + Cgroups + Union Filesystem + Seccomp/Capabilities

  ┌──────────────────────────────────────────────────┐
  │                   CONTAINER                      │
  │                                                  │
  │  ┌──────────────┐  ┌──────────────┐              │
  │  │  Namespaces  │  │   Cgroups    │              │
  │  │  (isolation) │  │  (resource   │              │
  │  │              │  │   limits)    │              │
  │  └──────────────┘  └──────────────┘              │
  │                                                  │
  │  ┌──────────────┐  ┌──────────────┐              │
  │  │ Union FS     │  │  Seccomp +   │              │
  │  │ (layered     │  │ Capabilities │              │
  │  │  filesystem) │  │ (security)   │              │
  │  └──────────────┘  └──────────────┘              │
  └──────────────────────────────────────────────────┘
```

---

## 3. Namespaces: Isolation

**Namespaces** give each container its own isolated view of system resources. A process inside a namespace sees only the resources assigned to that namespace — it cannot see or affect resources in other namespaces.

### Linux Namespace Types

| Namespace | Flag | What It Isolates | Effect |
|-----------|------|-----------------|--------|
| **PID** | `CLONE_NEWPID` | Process IDs | Container sees its own PID tree; init is PID 1 |
| **Mount** | `CLONE_NEWNS` | Filesystem mount points | Container has its own root filesystem |
| **Network** | `CLONE_NEWNET` | Network stack (interfaces, routing, ports) | Container has its own IP address, ports, routes |
| **UTS** | `CLONE_NEWUTS` | Hostname and domain name | Container has its own hostname |
| **IPC** | `CLONE_NEWIPC` | System V IPC, POSIX message queues | Container has its own shared memory segments |
| **User** | `CLONE_NEWUSER` | User and group IDs | Root (UID 0) inside container ≠ root on host |
| **Cgroup** | `CLONE_NEWCGROUP` | Cgroup root directory view | Container sees its cgroup as the root |
| **Time** | `CLONE_NEWTIME` | System clocks (since Linux 5.6) | Container can have different clock offsets |

### PID Namespace — Example

```
  Host PID namespace:
  ┌───────────────────────────────────────────┐
  │ PID 1: systemd (host init)                │
  │ PID 2: kthreadd                           │
  │ PID 100: sshd                             │
  │ PID 200: containerd                       │
  │ PID 300: container-shim (manages C1)      │
  │ PID 301: nginx         ◄─── container C1  │
  │ PID 302: nginx-worker  ◄─── container C1  │
  │ PID 400: container-shim (manages C2)      │
  │ PID 401: python app    ◄─── container C2  │
  └───────────────────────────────────────────┘

  Container C1's PID namespace:
  ┌───────────────────────┐
  │ PID 1: nginx          │  ◄── PID 301 on host
  │ PID 2: nginx-worker   │  ◄── PID 302 on host
  └───────────────────────┘
  C1 only sees its own processes.
  nginx thinks it's PID 1 (init).
  It cannot see or signal host processes or C2's processes.
```

### Network Namespace

Each container gets its own network stack:

```
  ┌────────────────────────────────────────────────────┐
  │                     Host                           │
  │                                                    │
  │  Container C1              Container C2            │
  │  ┌───────────────┐        ┌───────────────┐       │
  │  │ eth0: 172.17. │        │ eth0: 172.17. │       │
  │  │       0.2      │        │       0.3      │       │
  │  │ lo: 127.0.0.1 │        │ lo: 127.0.0.1 │       │
  │  │ Port 80 ✓     │        │ Port 80 ✓     │       │
  │  └──────┬────────┘        └──────┬────────┘       │
  │         │ veth pair               │ veth pair      │
  │         │                         │                │
  │  ┌──────┴─────────────────────────┴──────┐        │
  │  │           docker0 bridge               │        │
  │  │           172.17.0.1                   │        │
  │  └──────────────────┬────────────────────┘        │
  │                     │ NAT (iptables)               │
  │  ┌──────────────────┴────────────────────┐        │
  │  │          eth0 (host NIC)               │        │
  │  │          192.168.1.100                 │        │
  │  └───────────────────────────────────────┘        │
  └────────────────────────────────────────────────────┘

  Both containers can bind to port 80 — they have separate
  network namespaces. Traffic is bridged/NATed to the host NIC.
```

### Creating Namespaces

```c
// Clone with new namespaces:
int flags = CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET |
            CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWUSER;
pid_t pid = clone(child_fn, stack_top, flags | SIGCHLD, arg);

// Or move an existing process into a new namespace:
unshare(CLONE_NEWPID | CLONE_NEWNS);

// Or enter an existing container's namespace:
int fd = open("/proc/<pid>/ns/net", O_RDONLY);
setns(fd, CLONE_NEWNET);
// This is how "docker exec" works — enter the container's namespaces.
```

```bash
# View namespaces of a process:
$ ls -la /proc/self/ns/
lrwxrwxrwx 1 root root 0 ... cgroup -> 'cgroup:[4026531835]'
lrwxrwxrwx 1 root root 0 ... ipc -> 'ipc:[4026531839]'
lrwxrwxrwx 1 root root 0 ... mnt -> 'mnt:[4026531841]'
lrwxrwxrwx 1 root root 0 ... net -> 'net:[4026531840]'
lrwxrwxrwx 1 root root 0 ... pid -> 'pid:[4026531836]'
lrwxrwxrwx 1 root root 0 ... user -> 'user:[4026531837]'
lrwxrwxrwx 1 root root 0 ... uts -> 'uts:[4026531838]'
# The number in brackets is the namespace inode — same number = same namespace

# Enter a container's namespace manually:
$ sudo nsenter --target <PID> --mount --uts --ipc --net --pid
# Now you're "inside" the container, seeing its view of the system
```

---

## 4. Cgroups: Resource Limits

**Control Groups (cgroups)** limit how much of each resource a container (or any group of processes) can consume. Without cgroups, a container could monopolize the entire host's CPU, memory, or I/O.

### Cgroups Resource Controllers

| Controller | What It Limits |
|-----------|---------------|
| `cpu` | CPU time (shares, quotas, periods) |
| `cpuset` | Which CPU cores and NUMA nodes a group can use |
| `memory` | Memory usage limit (RSS + swap); OOM killer triggered at limit |
| `blkio` / `io` | Disk I/O bandwidth and IOPS |
| `pids` | Maximum number of processes (prevents fork bomb) |
| `net_cls` / `net_prio` | Network traffic classification and priority |
| `devices` | Which device files (block/char) can be accessed |
| `freezer` | Freeze/thaw a group of processes (for checkpointing) |

### Cgroups v2 Filesystem Interface

Cgroups are managed via a virtual filesystem, typically mounted at `/sys/fs/cgroup/`.

```bash
# View cgroup hierarchy:
$ ls /sys/fs/cgroup/
cgroup.controllers  cpu.stat  memory.current  pids.max  ...

# Create a new cgroup for a container:
$ mkdir /sys/fs/cgroup/my_container

# Set memory limit to 512 MB:
$ echo $((512 * 1024 * 1024)) > /sys/fs/cgroup/my_container/memory.max

# Set CPU limit to 50% of one core (50ms out of every 100ms period):
$ echo "50000 100000" > /sys/fs/cgroup/my_container/cpu.max
# Format: quota(μs) period(μs) → 50000/100000 = 50%

# Limit to 100 processes:
$ echo 100 > /sys/fs/cgroup/my_container/pids.max

# Add a process to the cgroup:
$ echo $PID > /sys/fs/cgroup/my_container/cgroup.procs
```

```
  Cgroups — Resource Enforcement:

  Host Resources:
  ┌────────────────────────────────────────────┐
  │ CPU: 8 cores │ RAM: 32 GB │ Disk: 1 TB    │
  └────────┬──────────┬──────────┬─────────────┘
           │          │          │
  ┌────────┴──┐ ┌─────┴────┐ ┌──┴────────┐
  │ Container │ │Container │ │ Container │
  │ A         │ │ B        │ │ C         │
  │           │ │          │ │           │
  │ cpu: 2    │ │ cpu: 4   │ │ cpu: 2    │
  │ cores     │ │ cores    │ │ cores     │
  │ mem: 4GB  │ │ mem: 16GB│ │ mem: 8GB  │
  │ io: 100MB/s│ │ io: 500MB/s│ │ io: 200MB/s│
  └───────────┘ └──────────┘ └───────────┘

  Each container is capped. If C_A tries to use > 4GB RAM,
  the kernel's OOM killer terminates processes in C_A.
  If C_B tries to use > 4 cores, CPU scheduler throttles it.
  One container cannot starve the others.
```

### Cgroups v1 vs. v2

| Feature | v1 | v2 |
|---------|----|----|
| Hierarchy | Multiple hierarchies (one per controller) | Single unified hierarchy |
| Membership | Process can be in different cgroups for different controllers | Process is in exactly one cgroup |
| Delegation | Complex, security issues | Clean delegation model |
| Pressure info | No | `memory.pressure`, `cpu.pressure`, `io.pressure` (PSI) |
| Status | Legacy but still widely used | Modern; default in recent distros |

---

## 5. Union Filesystems: Layered Images

Containers use **union filesystems** (also called overlay filesystems) to present a merged view of multiple read-only layers plus one writable layer on top.

### Why Layers?

Container images are built in layers. Each layer represents a change (install packages, copy files, etc.). Layers are:
- **Shared**: Multiple containers from the same image share the same read-only layers → saves disk and memory.
- **Cached**: Building a new image reuses unchanged layers → fast rebuilds.
- **Immutable**: Base layers never change → reproducible deployments.

```
  Docker Image Layers (Dockerfile → Layers):

  FROM ubuntu:22.04           ──► Layer 1: Base Ubuntu (shared by all)
  RUN apt-get install nginx   ──► Layer 2: Nginx package
  COPY app /var/www           ──► Layer 3: Application files
  CMD ["nginx"]

  Image:
  ┌───────────────────────────┐
  │ Layer 3: COPY app         │  (read-only)
  ├───────────────────────────┤
  │ Layer 2: RUN apt install  │  (read-only)
  ├───────────────────────────┤
  │ Layer 1: ubuntu:22.04     │  (read-only, shared)
  └───────────────────────────┘
```

### OverlayFS (Linux's Union Filesystem)

OverlayFS merges a **lower** (read-only) directory and an **upper** (read-write) directory into a single **merged** view:

```
  OverlayFS:

  ┌─────────────────────────────────┐
  │         Merged View             │  ◄── What the container sees
  │  /etc/nginx/nginx.conf          │      (union of all layers)
  │  /var/www/index.html            │
  │  /var/log/nginx/access.log (new)│
  └──────────────┬──────────────────┘
                 │ overlay mount
       ┌─────────┴──────────┐
       │                    │
  ┌────┴────────┐    ┌──────┴──────────┐
  │ Upper Layer │    │  Lower Layers   │
  │ (read-write)│    │  (read-only)    │
  │             │    │                 │
  │ access.log  │    │ nginx.conf     │
  │ (new file)  │    │ index.html     │
  │             │    │ /bin, /lib ... │
  └─────────────┘    └────────────────┘

  Read: check upper first, then lower.
  Write: goes to upper layer (copy-on-write for existing files).
  Delete: "whiteout" file in upper layer hides the lower file.
```

```bash
# View overlay mount of a running Docker container:
$ mount | grep overlay
overlay on /var/lib/docker/overlay2/.../merged type overlay
  (lowerdir=...,upperdir=...,workdir=...)

# Docker stores layers at:
$ ls /var/lib/docker/overlay2/
```

---

## 6. Security: Capabilities and Seccomp

### Linux Capabilities

Instead of giving containers full root privileges, Linux **capabilities** split root's powers into fine-grained permissions:

| Capability | Allows |
|-----------|--------|
| `CAP_NET_BIND_SERVICE` | Bind to ports below 1024 |
| `CAP_NET_RAW` | Use raw sockets (ping) |
| `CAP_SYS_ADMIN` | Mount filesystems, configure namespaces, etc. (very broad) |
| `CAP_SYS_PTRACE` | Trace/debug other processes |
| `CAP_CHOWN` | Change file ownership |
| `CAP_DAC_OVERRIDE` | Bypass file read/write/execute permission checks |

Docker drops most capabilities by default, running containers with a minimal set.

### Seccomp (Secure Computing)

**Seccomp-BPF** filters which syscalls a container can make. Docker's default seccomp profile blocks ~44 dangerous syscalls (out of ~300+):

```
  Seccomp Filter:

  Container Process
       │
       │ syscall(write, ...)
       ▼
  ┌──────────────────────┐
  │  Seccomp BPF Filter  │
  │                      │
  │  write   → ALLOW     │
  │  read    → ALLOW     │
  │  open    → ALLOW     │
  │  mount   → DENY      │  ◄── blocked!
  │  reboot  → DENY      │
  │  ptrace  → DENY      │
  │  kexec   → DENY      │
  └──────────────────────┘
       │
       ▼ (if allowed)
  ┌──────────────────────┐
  │      Kernel           │
  └──────────────────────┘
```

### AppArmor / SELinux

Mandatory Access Control (MAC) systems provide additional confinement:
- **AppArmor**: Path-based access control. Docker loads a default profile that restricts mount, ptrace, etc.
- **SELinux**: Label-based access control. Used in RHEL/CentOS; containers get confined labels.

---

## 7. Container Runtime Architecture

```
  Container Ecosystem Stack:

  ┌────────────────────────────────────────┐
  │  User Interface                        │
  │  docker CLI / podman / nerdctl         │
  └──────────────────┬─────────────────────┘
                     │ API
  ┌──────────────────┴─────────────────────┐
  │  Container Engine (High-Level Runtime)  │
  │  dockerd / containerd / CRI-O          │
  │                                        │
  │  - Image management (pull, build, push)│
  │  - Container lifecycle (create, start) │
  │  - Networking, volumes, logging        │
  └──────────────────┬─────────────────────┘
                     │ OCI runtime spec
  ┌──────────────────┴─────────────────────┐
  │  Low-Level Runtime (OCI Runtime)        │
  │  runc / crun / kata-containers          │
  │                                        │
  │  - Create namespaces                   │
  │  - Set up cgroups                      │
  │  - Mount rootfs (overlay)              │
  │  - Apply seccomp + capabilities        │
  │  - exec() the container's entrypoint   │
  └────────────────────────────────────────┘
```

**What `runc` does** when creating a container (simplified):

```
1. clone() with CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | ...
   → new namespaces created

2. mount() overlay filesystem as root
   → container sees its own filesystem

3. Set up cgroup:
   write PID to /sys/fs/cgroup/<container>/cgroup.procs
   set memory.max, cpu.max, pids.max

4. Apply seccomp filter (prctl(PR_SET_SECCOMP, ...))

5. Drop capabilities (capset())

6. pivot_root() to new root filesystem

7. execve("/entrypoint") — replace with container's main process
```

---

## 8. Container Orchestration

### The Need for Orchestration

Running a single container on a single host is straightforward. But in production:
- Applications consist of **dozens to hundreds** of microservices.
- Each microservice runs **multiple replicas** for availability and scale.
- Containers must be distributed across **many hosts** (a cluster).
- Containers crash and need to be **restarted automatically**.
- Traffic must be **load-balanced** across replicas.
- New versions must be **rolled out without downtime**.

This is too complex to manage manually. **Container orchestrators** automate it.

### Kubernetes — The Dominant Orchestrator

```
  Kubernetes Architecture:

  ┌──────────────────────────────────────────────────────────────┐
  │                     Control Plane                            │
  │                                                              │
  │  ┌──────────────┐  ┌──────────┐  ┌─────────────────────┐   │
  │  │ API Server   │  │Scheduler │  │ Controller Manager  │   │
  │  │ (kube-api)   │  │          │  │                     │   │
  │  │              │  │ Decides   │  │ Desired state →     │   │
  │  │ All requests │  │ which node│  │ actual state        │   │
  │  │ go through   │  │ runs each │  │ reconciliation      │   │
  │  │ here         │  │ pod       │  │ loops               │   │
  │  └──────┬───────┘  └──────────┘  └─────────────────────┘   │
  │         │                                                    │
  │  ┌──────┴───────┐                                            │
  │  │   etcd       │  ◄── distributed key-value store           │
  │  │              │      (cluster state, config)               │
  │  └──────────────┘                                            │
  └──────────────────────────────────────────────────────────────┘
                              │
               ┌──────────────┼──────────────┐
               │              │              │
  ┌────────────┴──┐  ┌───────┴────────┐  ┌──┴─────────────┐
  │   Worker Node │  │  Worker Node   │  │  Worker Node   │
  │               │  │                │  │                │
  │ ┌───────────┐ │  │ ┌────────────┐ │  │ ┌────────────┐ │
  │ │ kubelet   │ │  │ │ kubelet    │ │  │ │ kubelet    │ │
  │ │(agent)    │ │  │ │(agent)     │ │  │ │(agent)     │ │
  │ ├───────────┤ │  │ ├────────────┤ │  │ ├────────────┤ │
  │ │ kube-proxy│ │  │ │ kube-proxy │ │  │ │ kube-proxy │ │
  │ │(networking)│ │  │ │(networking)│ │  │ │(networking)│ │
  │ ├───────────┤ │  │ ├────────────┤ │  │ ├────────────┤ │
  │ │ Container │ │  │ │ Container  │ │  │ │ Container  │ │
  │ │ Runtime   │ │  │ │ Runtime    │ │  │ │ Runtime    │ │
  │ │(containerd)│ │  │ │(containerd)│ │  │ │(containerd)│ │
  │ ├───────────┤ │  │ ├────────────┤ │  │ ├────────────┤ │
  │ │┌──┐ ┌──┐ │ │  │ │┌──┐ ┌──┐  │ │  │ │┌──┐ ┌──┐  │ │
  │ ││P1│ │P2│ │ │  │ ││P3│ │P4│  │ │  │ ││P5│ │P6│  │ │
  │ │└──┘ └──┘ │ │  │ │└──┘ └──┘  │ │  │ │└──┘ └──┘  │ │
  │ └───────────┘ │  │ └────────────┘ │  │ └────────────┘ │
  └───────────────┘  └────────────────┘  └────────────────┘

  P1-P6 = Pods (smallest deployable unit; one or more containers)
```

### Key Kubernetes Concepts

| Concept | Description |
|---------|-------------|
| **Pod** | Smallest deployable unit. One or more containers sharing network/storage. |
| **Deployment** | Declares desired state: "run 3 replicas of image X". K8s ensures it. |
| **Service** | Stable network endpoint (virtual IP) load-balancing across pod replicas. |
| **Node** | A worker machine (physical or VM) running pods. |
| **Namespace** | Logical partition of cluster resources (not Linux namespaces). |
| **ConfigMap / Secret** | Inject configuration / credentials into containers. |
| **Ingress** | HTTP routing rules (hostname/path → Service). |

### Benefits of Orchestration

| Benefit | How |
|---------|-----|
| **Self-healing** | Crashed container → automatically restarted. Failed node → pods rescheduled elsewhere. |
| **Scaling** | `kubectl scale deployment/web --replicas=10` → instant horizontal scaling |
| **Rolling updates** | Gradually replace old pods with new version; rollback on failure |
| **Service discovery** | Pods find each other via DNS names (e.g., `my-service.default.svc.cluster.local`) |
| **Resource management** | CPU/memory requests and limits per container → efficient bin-packing |
| **Declarative config** | Define desired state in YAML; K8s reconciles actual state to match |

### Use Cases for Container Orchestration

| Use Case | Why Orchestration Helps |
|----------|------------------------|
| Microservices architecture | Manage hundreds of services, each with its own scaling and update lifecycle |
| CI/CD pipelines | Spin up build/test environments as containers; tear down when done |
| Batch processing | Run jobs across cluster; automatic retry on failure |
| ML training | Distribute training across GPU-equipped nodes; schedule based on resource availability |
| Edge computing | Deploy and manage containers across distributed edge locations |

---

## Summary

| Concept | Key Idea |
|---------|----------|
| Container | Lightweight isolated environment sharing host kernel |
| VM vs. Container | VMs virtualize hardware (strong isolation, heavy); containers virtualize OS (weak isolation, lightweight) |
| Namespaces | Isolate what a container can **see**: PID, network, mount, UTS, IPC, user, cgroup |
| PID namespace | Container has its own PID tree; its init is PID 1 |
| Network namespace | Container has its own IP, ports, routes; connected via veth + bridge |
| `clone()` / `unshare()` / `setns()` | Syscalls to create/enter namespaces |
| Cgroups | Limit what a container can **use**: CPU, memory, I/O, PIDs |
| Cgroups v2 | Single hierarchy; `/sys/fs/cgroup/`; memory.max, cpu.max, pids.max |
| OverlayFS | Union filesystem: read-only layers + writable upper layer; copy-on-write |
| Image layers | Shared, cached, immutable; efficient disk usage and fast builds |
| Capabilities | Fine-grained root privileges; containers run with minimal set |
| Seccomp | Syscall filter; blocks dangerous syscalls in containers |
| runc | Low-level OCI runtime; creates namespaces, cgroups, mounts, execve |
| containerd / CRI-O | High-level runtime; image management, lifecycle, networking |
| Kubernetes | Container orchestration: scheduling, scaling, self-healing, rolling updates |
| Pod | Smallest K8s unit; one or more containers sharing network/storage |
| Service | Stable endpoint load-balancing across pod replicas |
