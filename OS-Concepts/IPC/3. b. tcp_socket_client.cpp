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
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    std::cout << "Socket created successfully" << std::endl;

    // ============================================
    // STEP 2: Specify server address
    // ============================================
    // sockaddr_in is a structure containing an internet address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;        // IPv4
    server_address.sin_port = htons(PORT);      // htons converts port to network byte order

    // inet_pton converts IP address from text to binary form
    // "127.0.0.1" is localhost (the same machine)
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        std::cerr << "Invalid address or address not supported" << std::endl;
        close(sock_fd);
        return 1;
    }

    // ============================================
    // STEP 3: Connect to the server
    // ============================================
    // connect() establishes a connection to the server
    // This is blocking - it will wait until connection succeeds or fails
    std::cout << "Attempting to connect to server..." << std::endl;
    if (connect(sock_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(sock_fd);
        return 1;
    }
    std::cout << "Connected to server successfully!" << std::endl;

    // ============================================
    // STEP 4: Send data to server
    // ============================================
    const char* message = "Hello from client!";
    
    // send() sends data through the socket
    // Returns number of bytes sent, -1 on error
    ssize_t bytes_sent = send(sock_fd, message, strlen(message), 0);
    if (bytes_sent < 0)
    {
        std::cerr << "Send failed" << std::endl;
        close(sock_fd);
        return 1;
    }
    std::cout << "Message sent to server: " << message << std::endl;

    // ============================================
    // STEP 5: Receive response from server
    // ============================================
    char buffer[BUFFER_SIZE] = {0};
    
    // recv() receives data from the socket
    // Returns number of bytes received, 0 if connection closed, -1 on error
    ssize_t bytes_read = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';  // Null-terminate the string
        std::cout << "Received from server: " << buffer << std::endl;
    }
    else if (bytes_read == 0)
    {
        std::cout << "Server closed connection" << std::endl;
    }
    else
    {
        std::cerr << "recv failed" << std::endl;
    }

    // ============================================
    // STEP 6: Close the socket
    // ============================================
    close(sock_fd);
    std::cout << "Socket closed" << std::endl;

    return 0;
}

/*
COMPILATION AND EXECUTION:
g++ tcp_client.cpp -o client
./client

Make sure the server is running first before starting the client!

TYPICAL FLOW:
1. Start server: ./server
2. Server waits for connections
3. Start client: ./client
4. Client connects to server
5. Client sends message
6. Server receives message and sends response
7. Client receives response
8. Both close connections
*/