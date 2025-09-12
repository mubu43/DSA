#include <iostream>
#include <vector>

class Stack
{
private:
    std::vector<int> data;

public:
    void push(int value)
    {
        data.push_back(value);
    }

    void pop()
    {
        if (!data.empty())
        {
            data.pop_back();
        }
        else
        {
            std::cout << "Stack underflow\n";
        }
    }

    int top() const
    {
        if (!data.empty())
        {
            return data.back();
        }
        else
        {
            std::cout << "Stack is empty\n";
            return -1; // or throw exception
        }
    }

    bool empty() const
    {
        return data.empty();
    }

    size_t size() const
    {
        return data.size();
    }
};

int main()
{
    Stack s;
    s.push(10);
    s.push(20);
    s.push(30);

    std::cout << "Top element: " << s.top() << std::endl;
    s.pop();
    std::cout << "Top element after pop: " << s.top() << std::endl;

    while (!s.empty())
    {
        std::cout << "Popping: " << s.top() << std::endl;
        s.pop();
    }

    s.pop(); // Demonstrate underflow

    return 0;
}

/**
 * @class Stack
 * @brief A simple stack implementation using std::vector.
 *
 * Provides standard stack operations: push, pop, top, empty, and size.
 */

/**
 * @brief Pushes an integer value onto the top of the stack.
 * @param value The integer value to be added.
 */

/**
 * @brief Removes the top element from the stack.
 *
 * If the stack is empty, prints "Stack underflow".
 */

/**
 * @brief Returns the top element of the stack without removing it.
 * @return The top integer value if the stack is not empty; otherwise, prints "Stack is empty" and returns -1.
 */

/**
 * @brief Checks if the stack is empty.
 * @return true if the stack is empty, false otherwise.
 */

/**
 * @brief Returns the number of elements in the stack.
 * @return The size of the stack.
 */