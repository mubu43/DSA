#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    // ============================================
    // STEP 1: Create a socket
    // ============================================
    // socket() creates an endpoint for communication
    // AF_INET: IPv4 protocol family
    // SOCK_STREAM: TCP (connection-oriented, reliable)
    // 0: Protocol (0 means choose appropriate protocol for given type)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    std::cout << "Socket created successfully" << std::endl;

    // ============================================
    // STEP 2: Set socket options (optional but recommended)
    // ============================================
    // SO_REUSEADDR allows reusing the address immediately after program termination
    // Without this, you might get "Address already in use" error
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        return 1;
    }

    // ============================================
    // STEP 3: Bind socket to IP address and port
    // ============================================
    // sockaddr_in is a structure containing an internet address
    struct sockaddr_in address;
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // Accept connections from any IP (0.0.0.0)
    address.sin_port = htons(PORT);         // htons converts port to network byte order

    // bind() assigns the address to the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return 1;
    }
    std::cout << "Bind successful on port " << PORT << std::endl;

    // ============================================
    // STEP 4: Listen for incoming connections
    // ============================================
    // listen() marks the socket as passive (ready to accept connections)
    // Second parameter (3) is the backlog: max number of pending connections
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // ============================================
    // STEP 5: Accept incoming connection
    // ============================================
    // accept() blocks until a client connects, then returns a new socket for that client
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    
    int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_len);
    if (client_fd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        close(server_fd);
        return 1;
    }

    // Get client IP address and port for logging
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Client connected from " << client_ip 
              << ":" << ntohs(client_address.sin_port) << std::endl;

    // ============================================
    // STEP 6: Receive and send data
    // ============================================
    char buffer[BUFFER_SIZE] = {0};
    
    // recv() receives data from the client
    // Returns number of bytes received, 0 if connection closed, -1 on error
    ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        std::cout << "Received from client: " << buffer << std::endl;

        // send() sends data to the client
        const char* response = "Hello from server!";
        send(client_fd, response, strlen(response), 0);
        std::cout << "Response sent to client" << std::endl;
    }
    else if (bytes_read == 0)
    {
        std::cout << "Client disconnected" << std::endl;
    }
    else
    {
        std::cerr << "recv failed" << std::endl;
    }

    // ============================================
    // STEP 7: Close sockets
    // ============================================
    close(client_fd);
    close(server_fd);
    std::cout << "Sockets closed" << std::endl;

    return 0;
}

/*
COMPILATION AND EXECUTION:
g++ tcp_server.cpp -o server
./server

The server will wait for a client to connect.
*/