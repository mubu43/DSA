#include <iostream>

class Stack {
private:
    struct Node {
        int data;
        Node* next;
        
        Node(int value) {
            data = value;
            next = nullptr;
        }
    };
    
    Node* head;
    size_t count;

public:
    Stack() {
        head = nullptr;
        count = 0;
    }
    
    ~Stack() {
        while (!empty()) {
            pop();
        }
    }

    void push(int value) {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
        count++;
    }

    void pop() {
        if (!empty()) {
            Node* temp = head;
            head = head->next;
            delete temp;
            count--;
        } else {
            std::cout << "Stack underflow\n";
        }
    }

    int top() const {
        if (!empty()) {
            return head->data;
        } else {
            std::cout << "Stack is empty\n";
            return -1; // or throw exception
        }
    }

    bool empty() const {
        return head == nullptr;
    }

    size_t size() const {
        return count;
    }
};

int main() {
    Stack s;
    s.push(10);
    s.push(20);
    s.push(30);

    std::cout << "Top element: " << s.top() << std::endl;
    s.pop();
    std::cout << "Top element after pop: " << s.top() << std::endl;

    while (!s.empty()) {
        std::cout << "Popping: " << s.top() << std::endl;
        s.pop();
    }

    s.pop(); // Demonstrate underflow

    return 0;
}
