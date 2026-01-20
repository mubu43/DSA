#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/uds_demo.sock"

int main() {
    int sock_fd;
    struct sockaddr_un addr;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(1);
    }

    write(sock_fd, "Hello from client", 17);

    char buf[128];
    int n = read(sock_fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';

    printf("Client received: %s\n", buf);

    close(sock_fd);
    return 0;
}
