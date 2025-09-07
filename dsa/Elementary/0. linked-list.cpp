#include <iostream>
#include <memory>

template<typename T>
class LinkedList {
    struct Node {
        T data;
        std::unique_ptr<Node> next;
        Node(T val) : data(val), next(nullptr) {}
    };
    std::unique_ptr<Node> head;

public:
    void push_front(T val) {
        auto new_node = std::make_unique<Node>(val);
        new_node->next = std::move(head);
        head = std::move(new_node);
    }

    void push_back(T val) {
        auto new_node = std::make_unique<Node>(val);
        if (!head) {
            head = std::move(new_node);
            return;
        }
        Node* curr = head.get();
        while (curr->next) {
            curr = curr->next.get();
        }
        curr->next = std::move(new_node);
    }

    void print() const {
        Node* curr = head.get();
        while (curr) {
            std::cout << curr->data << " ";
            curr = curr->next.get();
        }
        std::cout << std::endl;
    }
};

int main() {
    LinkedList<int> list;
    list.push_back(1);
    list.push_front(0);
    list.push_back(2);
    list.print(); // Output: 0 1 2
    return 0;
}

/**
 * @brief A singly linked list implementation using smart pointers.
 * 
 * Operations:
 * - push_front(T val): Inserts a new element at the front of the list.
 * - push_back(T val): Appends a new element at the end of the list.
 * - print() const: Prints all elements in the list in order.
 * 
 * Internally, each node contains:
 * - data: The value stored in the node.
 * - next: A unique pointer to the next node in the list.
 * 
 * The list manages memory automatically using std::unique_ptr,
 * ensuring proper cleanup of nodes when the list is destroyed.
 */