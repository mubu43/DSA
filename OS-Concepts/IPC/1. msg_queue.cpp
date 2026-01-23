/**
 * 1. Message Queue IPC - Simple Example
 *
 * This demonstrates POSIX message queues for inter-process communication.
 * Run this program twice: once as sender, once as receiver.
 *
 * Usage:
 *   ./msg_queue send "Hello World"
 *   ./msg_queue recv
 *
 * Compile: g++ -o msg_queue "1. msg_queue.cpp" -lrt
 */

#include <iostream>
#include <cstring>
#include <mqueue.h>    // POSIX message queue
#include <fcntl.h>     // O_CREAT, O_RDONLY, etc.
#include <sys/stat.h>  // mode constants
#include <unistd.h>

using namespace std;

const char* QUEUE_NAME = "/my_queue";
const int MAX_MSG_SIZE = 256;
const int MAX_MSGS = 10;

// ============================================================================
// SENDER: Create queue and send a message
// ============================================================================
void send_message(const char* msg) {
    // Set queue attributes
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSGS;      // Max messages in queue
    attr.mq_msgsize = MAX_MSG_SIZE;  // Max message size
    attr.mq_curmsgs = 0;

    // Create or open the queue (read-write, create if not exists)
    mqd_t mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open (sender)");
        return;
    }

    // Send message (priority 0)
    if (mq_send(mq, msg, strlen(msg) + 1, 0) == -1) {
        perror("mq_send");
    } else {
        cout << "[Sender] Sent: \"" << msg << "\"" << endl;
    }

    mq_close(mq);
}

// ============================================================================
// RECEIVER: Open queue and receive a message
// ============================================================================
void receive_message() {
    // Open the existing queue (read-only)
    mqd_t mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open (receiver)");
        cout << "Hint: Run sender first to create the queue." << endl;
        return;
    }

    // Get queue attributes to know max message size
    struct mq_attr attr;
    mq_getattr(mq, &attr);

    // Buffer for received message
    char buffer[MAX_MSG_SIZE + 1];
    unsigned int priority;

    // Receive message (blocks until message available)
    ssize_t bytes = mq_receive(mq, buffer, attr.mq_msgsize, &priority);
    if (bytes == -1) {
        perror("mq_receive");
    } else {
        buffer[bytes] = '\0';
        cout << "[Receiver] Got: \"" << buffer << "\" (priority: " << priority << ")" << endl;
    }

    mq_close(mq);
}

// ============================================================================
// CLEANUP: Remove the queue
// ============================================================================
void cleanup_queue() {
    if (mq_unlink(QUEUE_NAME) == -1) {
        perror("mq_unlink");
    } else {
        cout << "[Cleanup] Queue removed." << endl;
    }
}

// ============================================================================
// MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage:" << endl;
        cout << "  " << argv[0] << " send \"message\"  - Send a message" << endl;
        cout << "  " << argv[0] << " recv             - Receive a message" << endl;
        cout << "  " << argv[0] << " clean            - Remove the queue" << endl;
        return 1;
    }

    string cmd = argv[1];

    if (cmd == "send" && argc >= 3) {
        send_message(argv[2]);
    } else if (cmd == "recv") {
        receive_message();
    } else if (cmd == "clean") {
        cleanup_queue();
    } else {
        cerr << "Unknown command or missing arguments." << endl;
        return 1;
    }

    return 0;
}
