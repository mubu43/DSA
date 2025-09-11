#include <iostream>
#include <memory>

// Node structure for singly linked list
struct Node {
    int data;
    std::unique_ptr<Node> next;

    Node(int val) {
        data = val;
        next = nullptr;
    }
};

// Singly Linked List class
class LinkedList {
private:
    std::unique_ptr<Node> head;

public:
    LinkedList() {
        head = nullptr;
    }

    // No need for a custom destructor; unique_ptr handles cleanup

    // Insert a new node at the end
    void append(int value) {
        auto newNode = std::make_unique<Node>(value);
        if (!head) {
            head = std::move(newNode);
            return;
        }
        Node* current = head.get();
        while (current->next)
            current = current->next.get();
        current->next = std::move(newNode);
    }

    // Insert a new node at the beginning
    void prepend(int value) {
        auto newNode = std::make_unique<Node>(value);
        newNode->next = std::move(head);
        head = std::move(newNode);
    }

    // Delete the first node with the given value
    void remove(int value) {
        Node* current = head.get();
        Node* prev = nullptr;

        while (current) {
            if (current->data == value) {
                if (prev) {
                    prev->next = std::move(current->next);
                } else {
                    head = std::move(current->next);
                }
                return;
            }
            prev = current;
            current = current->next.get();
        }
    }

    // Print the linked list
    void print() const {
        const Node* current = head.get();
        while (current) {
            std::cout << current->data;
            if (current->next) std::cout << " -> ";
            current = current->next.get();
        }
        std::cout << std::endl;
    }
};

// Example usage
int main() {
    LinkedList list;
    list.append(10);
    list.append(20);
    list.prepend(5);
    list.print(); // Output: 5 -> 10 -> 20

    list.remove(10);
    list.print(); // Output: 5 -> 20

    return 0;
}
