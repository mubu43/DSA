// uds_server.c
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SOCKET_PATH "/tmp/uds_demo.sock"

int main() 
{
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buf[128];

    // 1. Create socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // 2. Bind to filesystem path
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);  // remove old socket if exists

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // 3. Listen
    listen(server_fd, 5);

    printf("UDS server listening at %s\n", SOCKET_PATH);

    // 4. Accept connection
    client_fd = accept(server_fd, NULL, NULL);

    // 5. Read data
    int n = read(client_fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("Server received: %s\n", buf);

    // 6. Reply
    write(client_fd, "Hello from server", 17);

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}