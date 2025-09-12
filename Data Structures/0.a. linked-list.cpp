#include <iostream>

// Node structure for singly linked list
struct Node
{
    int data;       // Data stored in the node
    Node* next;     // Pointer to the next node

    Node(int val)
    {
        data = val;
        next = nullptr;
    }
};

// Singly Linked List class
class LinkedList
{
private:
    Node* head;     // Pointer to the first node

public:
    // Constructor
    LinkedList()
    {
        head = nullptr;
    }

    // Destructor to free memory
    ~LinkedList()
    {
        Node* current = head;
        while (current)
        {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }

    // Insert a new node at the end
    void append(int value)
    {
        Node* newNode = new Node(value);
        if (!head)
        {
            head = newNode;
            return;
        }
        Node* current = head;
        while (current->next)
        {
            current = current->next;
        }
        current->next = newNode;
    }

    // Insert a new node at the beginning
    void prepend(int value)
    {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

    // Insert a new node at a specific index (0-based)
    void insertAt(int index, int value)
    {
        // If index is 0, insert at the beginning
        if (index == 0)
        {
            prepend(value);
            return;
        }

        // Check for negative index
        if (index < 0)
        {
            std::cout << "Error: Index cannot be negative" << std::endl;
            return;
        }

        Node* newNode = new Node(value);
        Node* current = head;

        // Traverse to the position just before the desired index
        for (int i = 0; i < index - 1 && current != nullptr; i++)
        {
            current = current->next;
        }

        // If current is null, index is out of bounds
        if (current == nullptr)
        {
            std::cout << "Error: Index " << index << " is out of bounds" << std::endl;
            delete newNode; // Clean up allocated memory
            return;
        }

        // Insert the new node
        newNode->next = current->next;
        current->next = newNode;
    }

    // Delete the first node with the given value
    void remove(int value)
    {
        if (!head)
        {
            return;
        }
        if (head->data == value)
        {
            Node* temp = head;
            head = head->next;
            delete temp;
            return;
        }
        Node* current = head;
        // Traverse the list to find the node just before the one to delete.
        // Stop if we reach the end (current->next == nullptr) or if the next node's data matches 'value'.
        while (current->next && current->next->data != value)
        {
            current = current->next;
        }
        // If the next node exists and its data matches 'value', remove it.
        if (current->next)
        {
            Node* temp = current->next;         // Store pointer to the node to be deleted
            current->next = temp->next;         // Bypass the node to be deleted
            delete temp;                        // Free memory of the deleted node
        }
    }

    // Delete a node at a specific index (0-based)
    void removeAt(int index)
    {
        // Check for negative index
        if (index < 0)
        {
            std::cout << "Error: Index cannot be negative" << std::endl;
            return;
        }

        // Check if list is empty
        if (!head)
        {
            std::cout << "Error: Cannot remove from empty list" << std::endl;
            return;
        }

        // If index is 0, remove the head node
        if (index == 0)
        {
            Node* temp = head;
            head = head->next;
            delete temp;
            return;
        }

        Node* current = head;
        
        // Traverse to the position just before the desired index
        for (int i = 0; i < index - 1 && current != nullptr; i++)
        {
            current = current->next;
        }
        
        // If current is null or current->next is null, index is out of bounds
        if (current == nullptr || current->next == nullptr)
        {
            std::cout << "Error: Index " << index << " is out of bounds" << std::endl;
            return;
        }
        
        // Remove the node at the specified index
        Node* nodeToDelete = current->next;
        current->next = nodeToDelete->next;
        delete nodeToDelete;
    }

    // Print the linked list
    void print() const
    {
        Node* current = head;
        while (current)
        {
            std::cout << current->data;
            if (current->next)
            {
                std::cout << " -> ";
            }
            current = current->next;
        }
        std::cout << std::endl;
    }
};

// Example usage
int main()
{
    LinkedList list;
    list.append(10);
    list.append(20);
    list.prepend(5);
    list.print(); // Output: 5 -> 10 -> 20

    // Insert at specific indices
    list.insertAt(1, 15); // Insert 15 at index 1
    list.print(); // Output: 5 -> 15 -> 10 -> 20

    list.insertAt(0, 1); // Insert 1 at beginning (index 0)
    list.print(); // Output: 1 -> 5 -> 15 -> 10 -> 20

    list.insertAt(5, 25); // Insert 25 at end (index 5)
    list.print(); // Output: 1 -> 5 -> 15 -> 10 -> 20 -> 25

    // Test error cases for insertion
    list.insertAt(-1, 100); // Should show error message
    list.insertAt(10, 200); // Should show error message (out of bounds)

    // Test removal by value
    list.remove(10);
    list.print(); // Output: 1 -> 5 -> 15 -> 20 -> 25

    // Test removal by index
    list.removeAt(2); // Remove element at index 2 (value 20)
    list.print(); // Output: 1 -> 5 -> 15 -> 25

    list.removeAt(0); // Remove first element (value 1)
    list.print(); // Output: 5 -> 15 -> 25

    list.removeAt(2); // Remove last element (value 25)
    list.print(); // Output: 5 -> 15

    // Test error cases for removal
    list.removeAt(-1); // Should show error message
    list.removeAt(5);  // Should show error message (out of bounds)

    return 0;
}