
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

int main()
{
    int fd = open("/dev/mydevice", O_RDWR);
    uint32_t *ptr = mmap(
        NULL, 4096,
        PROT_READ | PROT_WRITE,
        MAP_SHARED, fd, 0
    );

    /* Ecriture directe */
    ptr[0] = 0x55445566;

    printf("Value: 0x%x\n", ptr[0]);

    /* Verification avec read() */
    uint32_t value;

    lseek(fd, 0, SEEK_SET);
    read(fd, &value, 4);

    printf("Read syscall: 0x%x\n", value);

    munmap(ptr, 4096);
    close(fd);

    return 0;
}

