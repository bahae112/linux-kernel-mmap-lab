
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd = open("/dev/mydevice", O_RDWR);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    char msg[] = "HELLO";
    char buf[20] = {0};

    ssize_t w = write(fd, msg, strlen(msg));

    printf("write() returned: %zd\n", w);

    if (w < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("lseek");
        close(fd);
        return 1;
    }

    ssize_t r = read(fd, buf, sizeof(buf) - 1);

    printf("read() returned: %zd\n", r);

    if (r < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    printf("Received: '%s'\n", buf);

    close(fd);
    return 0;
}

