#include <iostream>
#include <stdexcept>

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

    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        if (rear_) {
            rear_->next = newNode;
        } else {
            front_ = newNode;
        }
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