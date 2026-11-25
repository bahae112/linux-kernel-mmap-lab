## Linux Kernel Memory Mapping – `mmap()` Driver Lab

### 1. Objective

The objective of this lab is to understand how memory can be shared between **kernel space** and **user space** using the Linux `mmap()` system call.

Instead of transferring data using traditional `read()` and `write()` operations, the goal is to expose a memory region allocated by a kernel module directly to a user-space application.

This allows both sides to access the same physical memory pages through different virtual addresses.

### 2. Architecture

The project consists of two main components:

- **Kernel module (`mydevice.c`):** Allocates a memory buffer and exposes it through a character device (`/dev/mydevice`).
- **User-space application (`mmap_app.c`):** Opens the device, maps its memory using `mmap()`, and accesses the shared buffer.

### 3. Kernel-Space Implementation

#### Memory allocation

```c
buffer = vzalloc(PAGE_SIZE);
```

- `vzalloc()` allocates virtually contiguous kernel memory.
- `PAGE_SIZE` represents the system memory page size (typically 4096 bytes).
- The allocated memory is initialized to zero.

#### Memory mapping

```c
static int my_mmap(struct file *file,
                   struct vm_area_struct *vma)
{
    return remap_vmalloc_range(vma, buffer, 0);
}
```

The `mmap()` callback is invoked when a user-space process requests a mapping of `/dev/mydevice`.

`remap_vmalloc_range()` maps the pages belonging to the `vzalloc()` buffer into the process address space.

This establishes access to the same physical memory pages from both kernel space and user space.

#### Device registration

```c
static struct miscdevice dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydevice",
    .fops = &fops,
    .mode = 0600,
};
```

The kernel registers a miscellaneous character device accessible through:

```text
/dev/mydevice
```

The `.mmap` callback is registered through `file_operations`.

### 4. User-Space Implementation

The application performs three main operations.

**Step 1 – Open the device**

```c
int fd = open("/dev/mydevice", O_RDWR);
```

Opens the device and obtains a file descriptor.

**Step 2 – Map the kernel buffer**

```c
char *ptr = mmap(NULL, 4096,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0);
```

Parameters:

| Parameter | Description |
|---|---|
| `NULL` | Kernel chooses the virtual address |
| `4096` | Mapping size in bytes |
| `PROT_READ` | Allows reading |
| `PROT_WRITE` | Allows writing |
| `MAP_SHARED` | Shares modifications with the underlying mapped memory |
| `fd` | Device file descriptor |
| `0` | Mapping offset |

**Step 3 – Access shared memory**

```c
strcpy(ptr, "HELLO FROM USER");
printf("Data: %s\n", ptr);
```

The application writes directly into the mapped memory region and reads the same data back.

Unlike `read()` and `write()`, this operation does not require explicit `copy_to_user()` or `copy_from_user()` calls.

**Step 4 – Unmap memory**

```c
munmap(ptr, 4096);
close(fd);
```

Releases the user-space mapping and closes the device.

### 5. Memory Mapping Mechanism

The mapping can be represented as follows:

```text
          USER SPACE
    +----------------------+
    |                      |
    |  ptr                 |
    |  Virtual Address A   |
    |                      |
    +----------+-----------+
               |
               | mmap()
               |
               v
    +----------------------+
    |                      |
    |  Physical Memory     |
    |  Shared Pages        |
    |                      |
    +----------+-----------+
               ^
               |
               | remap_vmalloc_range()
               |
    +----------+-----------+
    |                      |
    |  buffer              |
    |  Kernel Virtual Addr |
    |                      |
    +----------------------+
          KERNEL SPACE
```

Both virtual addresses refer to the same underlying physical memory pages.

Consequently, modifying the buffer through `ptr` in user space also modifies the memory accessed through `buffer` in kernel space.

### 6. `read()` / `write()` vs `mmap()`

| Feature | `read()` / `write()` | `mmap()` |
|---|---|---|
| Data access | Through system calls | Through mapped memory |
| Data copying | Explicit user/kernel copies | No explicit copy for each access |
| Kernel functions | `copy_to_user()` / `copy_from_user()` | `remap_vmalloc_range()` |
| Access | Operation-based | Direct memory access |
| Typical use | Character device I/O | Shared memory and memory-mapped devices |

### 7. Key Takeaways

Through this lab, I explored:

- Linux kernel modules and miscellaneous character devices.
- Kernel memory allocation using `vzalloc()`.
- The `mmap()` system call and virtual memory mapping.
- The role of `vm_area_struct` in representing a process memory region.
- The use of `remap_vmalloc_range()` to expose kernel-allocated memory.
- The difference between traditional data transfers and shared memory mapping.

**Important:** `mmap()` does not automatically imply DMA or zero-copy hardware transfers. This lab demonstrates kernel/user memory mapping using a `vzalloc()` buffer, not a complete DMA implementation.

