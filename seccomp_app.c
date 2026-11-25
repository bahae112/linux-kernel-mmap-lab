
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include <sys/syscall.h>
#include <stdint.h>

int main()
{
    int fd = open("/dev/mydevice", O_RDWR);

    uint32_t *ptr = mmap(
        NULL, 4096,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0
    );

    if (ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    /* Bloquer read() */
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                 0),
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K,
                 __NR_read, 0, 1),
        BPF_STMT(BPF_RET | BPF_K,
                 SECCOMP_RET_ERRNO | 1),
        BPF_STMT(BPF_RET | BPF_K,
                 SECCOMP_RET_ALLOW)
    };

    struct sock_fprog prog = {
        .len = 4,
        .filter = filter
    };

    prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

    prctl(PR_SET_SECCOMP,
          SECCOMP_MODE_FILTER, &prog);

    uint32_t value;

    if (read(fd, &value, 4) < 0)
        perror("read blocked");

    /* mmap fonctionne toujours */
    ptr[0] = 0x55445566;

    printf("MMAP value: 0x%x\n", ptr[0]);

    munmap(ptr, 4096);
    close(fd);

    return 0;
}

