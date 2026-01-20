// shm_reader.c
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

#define SHM_NAME "/demo_shm"

int main() {
    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);

    int *shared = mmap(NULL, sizeof(int),
                       PROT_READ,
                       MAP_SHARED, fd, 0);

    printf("Reader read: %d\n", *shared);

    munmap(shared, sizeof(int));
    close(fd);

    shm_unlink(SHM_NAME); // cleanup
    return 0;
}