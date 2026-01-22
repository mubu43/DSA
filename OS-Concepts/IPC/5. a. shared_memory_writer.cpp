// shm_writer.c
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#define SHM_NAME "/demo_shm"

int main() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(int));

    int *shared = mmap(NULL, sizeof(int),
                       PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0);

    *shared = 42;
    printf("Writer wrote: %d\n", *shared);

    sleep(10); // keep mapping alive

    munmap(shared, sizeof(int));
    close(fd);
    return 0;
}