#include <iostream>
#include <stdexcept>

/*
 * Queue Implementation using Linked List
 * 
 * Structure: front_ -> [Node1] -> [Node2] -> [Node3] -> nullptr
 *                                              ^
 *                                            rear_
 * 
 * - Elements are added at the REAR (enqueue)
 * - Elements are removed from the FRONT (dequeue)
 * - Each node's 'next' pointer points toward the rear of the queue
 * - When enqueuing: rear_->next = newNode, then rear_ = newNode
 *   This maintains the linked list chain and adds new elements at the end
 */
template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& val) : data(val), next(nullptr) {}
    };
    Node* front_;
    Node* rear_;
    size_t size_;

public:
    Queue() : front_(nullptr), rear_(nullptr), size_(0) {}

    ~Queue() {
        while (!empty()) {
            dequeue();
        }
    }

    // Adds an element to the end of the queue
    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        if (rear_) {
            // If the queue is not empty, link the new node after the current rear
            rear_->next = newNode;
        } else {
            // If the queue is empty, set front_ to the new node
            front_ = newNode;
        }
        // Update rear_ to point to the new node
        rear_ = newNode;
        ++size_;
    }

    void dequeue() {
        if (empty()) throw std::out_of_range("Queue is empty");
        Node* temp = front_;
        front_ = front_->next;
        if (!front_) rear_ = nullptr;
        delete temp;
        --size_;
    }

    T& front() {
        if (empty()) throw std::out_of_range("Queue is empty");
        return front_->data;
    }

    bool empty() const {
        return size_ == 0;
    }

    size_t size() const {
        return size_;
    }
};

// Example usage
int main() {
    Queue<int> q;
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);

    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.dequeue();
    }
    std::cout << std::endl;
    return 0;
}