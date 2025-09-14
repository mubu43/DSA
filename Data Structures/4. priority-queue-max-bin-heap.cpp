#include <iostream>
#include <vector>
#include <stdexcept>

// Template class for a max-priority queue using a binary heap
template <typename T>
class PriorityQueue
{
private:
    std::vector<T> heap; // Underlying container for the heap

    // Returns the index of the parent node
    int parent(int i) const
    {
        return (i - 1) / 2;
    }

    // Returns the index of the left child
    int leftChild(int i) const
    {
        return 2 * i + 1;
    }

    // Returns the index of the right child
    int rightChild(int i) const
    {
        return 2 * i + 2;
    }

    // Moves the element at index up to restore heap property
    void swim(int index)
    {
        while (index > 0)
        {
            int parentIndex = parent(index);

            // If heap property is satisfied, stop
            if (heap[index] <= heap[parentIndex])
            {
                break;
            }

            // Swap with parent and continue
            std::swap(heap[index], heap[parentIndex]);
            index = parentIndex;
        }
    }

    // Moves the element at index down to restore heap property
    void sink(int index)
    {
        int size = heap.size();

        while (true)
        {
            int largest = index;
            int left = leftChild(index);
            int right = rightChild(index);

            // Find the largest among index, left, and right
            if (left < size && heap[left] > heap[largest])
            {
                largest = left;
            }

            if (right < size && heap[right] > heap[largest])
            {
                largest = right;
            }

            // If index is largest, heap property is restored
            if (largest == index)
            {
                break;
            }

            // Swap with the largest child and continue
            std::swap(heap[index], heap[largest]);
            index = largest;
        }
    }

public:
    PriorityQueue() {}

    // Optional: reserve space for efficiency
    PriorityQueue(int capacity)
    {
        heap.reserve(capacity);
    }

    // Insert a new value and restore heap property
    void insert(const T& value)
    {
        heap.push_back(value);
        swim(heap.size() - 1);
    }

    // Remove and return the maximum element (root)
    T extractMax()
    {
        if (empty())
        {
            throw std::out_of_range("Priority queue is empty");
        }

        T maxElement = heap[0];
        heap[0] = heap.back();
        heap.pop_back();

        // Restore heap property if not empty
        if (!empty())
        {
            sink(0);
        }

        return maxElement;
    }

    // Return the maximum element without removing it
    const T& top() const
    {
        if (empty())
        {
            throw std::out_of_range("Priority queue is empty");
        }
        return heap[0];
    }

    // Check if the queue is empty
    bool empty() const
    {
        return heap.empty();
    }

    // Return the number of elements in the queue
    size_t size() const
    {
        return heap.size();
    }

    // Print the heap as a flat array
    void printHeap() const
    {
        std::cout << "Heap array: ";
        for (const T& element : heap)
        {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }

    // Print the heap as a tree structure
    void printTree() const
    {
        if (empty())
        {
            std::cout << "Empty heap" << std::endl;
            return;
        }

        std::cout << "Heap tree structure:" << std::endl;
        printTreeHelper(0, "", true);
    }

private:
    // Helper function for pretty-printing the tree
    void printTreeHelper(int index, const std::string& prefix, bool isLast) const
    {
        if (index >= heap.size()) return;

        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << heap[index] << std::endl;

        int left = leftChild(index);
        int right = rightChild(index);

        // Print right child first for better visualization
        if (left < heap.size() || right < heap.size())
        {
            if (right < heap.size())
            {
                printTreeHelper(right, prefix + (isLast ? "    " : "│   "), false);
            }
            if (left < heap.size())
            {
                printTreeHelper(left, prefix + (isLast ? "    " : "│   "), true);
            }
        }
    }
};

int main()
{
    std::cout << "=== PRIORITY QUEUE DEMONSTRATION ===\n\n";

    PriorityQueue<int> pq;

    std::cout << "=== Test 1: Basic Insertion and Extraction ===\n";

    std::vector<int> values = {10, 20, 15, 30, 40, 25, 35};

    std::cout << "Inserting values: ";
    for (int val : values)
    {
        std::cout << val << " ";
        pq.insert(val); // Insert each value into the priority queue
    }
    std::cout << "\n\n";

    std::cout << "Heap after insertions:\n";
    pq.printHeap(); // Print heap as array
    pq.printTree(); // Print heap as tree
    std::cout << "\n";

    std::cout << "=== Test 2: Priority-based Extraction ===\n";
    std::cout << "Extracting elements in priority order:\n";

    // Extract elements one by one in order of priority
    while (!pq.empty())
    {
        int max = pq.extractMax();
        std::cout << "Extracted: " << max << " | Remaining size: " << pq.size();
        if (!pq.empty())
        {
            std::cout << " | New top: " << pq.top();
        }
        std::cout << std::endl;
    }
    std::cout << "\n";

    std::cout << "=== Test 3: Task Scheduling Example ===\n";

    // Define a Task struct with priority and name
    struct Task
    {
        int priority;
        std::string name;

        Task(int p, const std::string& n)
        {
            priority = p;
            name = n;
        }

        // Comparison operators for heap ordering
        bool operator>(const Task& other) const
        {
            return priority > other.priority;
        }

        bool operator<=(const Task& other) const
        {
            return priority <= other.priority;
        }

        // For printing Task objects
        friend std::ostream& operator<<(std::ostream& os, const Task& task)
        {
            return os << task.name << "(" << task.priority << ")";
        }
    };

    PriorityQueue<Task> taskQueue;

    // Insert tasks with different priorities
    taskQueue.insert(Task(3, "Email"));
    taskQueue.insert(Task(1, "Documentation"));
    taskQueue.insert(Task(5, "Critical Bug Fix"));
    taskQueue.insert(Task(2, "Code Review"));
    taskQueue.insert(Task(4, "Meeting"));

    std::cout << "Tasks in priority queue:\n";
    taskQueue.printHeap();
    std::cout << "\nProcessing tasks by priority:\n";

    // Process tasks in order of priority
    while (!taskQueue.empty())
    {
        Task nextTask = taskQueue.extractMax();
        std::cout << "Processing: " << nextTask << std::endl;
    }

    std::cout << "\n=== ALL TESTS COMPLETED ===\n";

    return 0;
}
