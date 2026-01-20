#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() 
{
    int pipefd[2];  // pipefd[0] = read end, pipefd[1] = write end
    pid_t pid;
    char buffer[100];
    const char* message = "Hello from parent!";
    
    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    // Fork a child process
    pid = fork();
    
    if (pid == 0) {
        // Child process (reader)
        close(pipefd[1]);  // Close write end
        
        read(pipefd[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
        
        close(pipefd[0]);
    } else if (pid > 0) {
        // Parent process (writer)
        close(pipefd[0]);  // Close read end
        
        write(pipefd[1], message, strlen(message) + 1);
        printf("Parent sent: %s\n", message);
        
        close(pipefd[1]);
        wait(NULL);  // Wait for child
    }
    
    return 0;
}