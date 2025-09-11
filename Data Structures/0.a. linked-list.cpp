#include <iostream>

// Node structure for singly linked list
struct Node {
    int data;       // Data stored in the node
    Node* next;     // Pointer to the next node

    Node(int val) {
        data = val;
        next = nullptr;
    }
};

// Singly Linked List class
class LinkedList {
private:
    Node* head;     // Pointer to the first node

public:
    // Constructor
    LinkedList() {
        head = nullptr;
    }

    // Destructor to free memory
    ~LinkedList() {
        Node* current = head;
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }

    // Insert a new node at the end
    void append(int value) {
        Node* newNode = new Node(value);
        if (!head) {
            head = newNode;
            return;
        }
        Node* current = head;
        while (current->next)
            current = current->next;
        current->next = newNode;
    }

    // Insert a new node at the beginning
    void prepend(int value) {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

    // Delete the first node with the given value
    void remove(int value) {
        if (!head) return;
        if (head->data == value) {
            Node* temp = head;
            head = head->next;
            delete temp;
            return;
        }
        Node* current = head;
        // Traverse the list to find the node just before the one to delete.
        // Stop if we reach the end (current->next == nullptr) or if the next node's data matches 'value'.
        while (current->next && current->next->data != value)
            current = current->next;
        // If the next node exists and its data matches 'value', remove it.
        if (current->next) {
            Node* temp = current->next;         // Store pointer to the node to be deleted
            current->next = temp->next;         // Bypass the node to be deleted
            delete temp;                        // Free memory of the deleted node
        }
    }

    // Print the linked list
    void print() const {
        Node* current = head;
        while (current) {
            std::cout << current->data;
            if (current->next) std::cout << " -> ";
            current = current->next;
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