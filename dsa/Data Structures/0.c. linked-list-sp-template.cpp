#include <iostream>
#include <memory>
#include <string>

// Node structure for singly linked list (templated)
template<typename T>
struct Node {
    T data;
    std::unique_ptr<Node<T>> next;

    Node(const T& val) : data(val), next(nullptr) {}
};

// Singly Linked List class (templated)
template<typename T>
class LinkedList {
private:
    std::unique_ptr<Node<T>> head;

public:
    LinkedList() : head(nullptr) {}

    void append(const T& value) {
        auto newNode = std::make_unique<Node<T>>(value);
        if (!head) {
            head = std::move(newNode);
            return;
        }
        Node<T>* current = head.get();
        while (current->next)
            current = current->next.get();
        current->next = std::move(newNode);
    }

    void prepend(const T& value) {
        auto newNode = std::make_unique<Node<T>>(value);
        newNode->next = std::move(head);
        head = std::move(newNode);
    }

    void remove(const T& value) {
        Node<T>* current = head.get();
        Node<T>* prev = nullptr;

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

    void print() const {
        const Node<T>* current = head.get();
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
    // Example 1: Integers
    LinkedList<int> intList;
    intList.append(10);
    intList.append(20);
    intList.prepend(5);
    intList.print(); // Output: 5 -> 10 -> 20

    intList.remove(10);
    intList.print(); // Output: 5 -> 20

    // Example 2: Strings
    LinkedList<std::string> strList;
    strList.append("apple");
    strList.append("banana");
    strList.prepend("orange");
    strList.print(); // Output: orange -> apple -> banana

    strList.remove("apple");
    strList.print(); // Output: orange -> banana

    return 0;
}
