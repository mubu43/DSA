#include <iostream>
#include <vector>
#include <stdexcept>

template <typename T>
class PriorityQueue
{
private:
    std::vector<T> heap;

    int parent(int i) const
    {
        return (i - 1) / 2;
    }

    int leftChild(int i) const
    {
        return 2 * i + 1;
    }

    int rightChild(int i) const
    {
        return 2 * i + 2;
    }

    void swim(int index)
    {
        while (index > 0)
        {
            int parentIndex = parent(index);

            if (heap[index] <= heap[parentIndex])
            {
                break;
            }

            std::swap(heap[index], heap[parentIndex]);
            index = parentIndex;
        }
    }

    void sink(int index)
    {
        int size = heap.size();

        while (true)
        {
            int largest = index;
            int left = leftChild(index);
            int right = rightChild(index);

            if (left < size && heap[left] > heap[largest])
            {
                largest = left;
            }

            if (right < size && heap[right] > heap[largest])
            {
                largest = right;
            }

            if (largest == index)
            {
                break;
            }

            std::swap(heap[index], heap[largest]);
            index = largest;
        }
    }

public:
    PriorityQueue() {}

    PriorityQueue(int capacity)
    {
        heap.reserve(capacity);
    }

    void insert(const T& value)
    {
        heap.push_back(value);
        swim(heap.size() - 1);
    }

    T extractMax()
    {
        if (empty())
        {
            throw std::out_of_range("Priority queue is empty");
        }

        T maxElement = heap[0];
        heap[0] = heap.back();
        heap.pop_back();

        if (!empty())
        {
            sink(0);
        }

        return maxElement;
    }

    const T& top() const
    {
        if (empty())
        {
            throw std::out_of_range("Priority queue is empty");
        }
        return heap[0];
    }

    bool empty() const
    {
        return heap.empty();
    }

    size_t size() const
    {
        return heap.size();
    }

    void printHeap() const
    {
        std::cout << "Heap array: ";
        for (const T& element : heap)
        {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }

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
    void printTreeHelper(int index, const std::string& prefix, bool isLast) const
    {
        if (index >= heap.size()) return;

        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << heap[index] << std::endl;

        int left = leftChild(index);
        int right = rightChild(index);

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
        pq.insert(val);
    }
    std::cout << "\n\n";

    std::cout << "Heap after insertions:\n";
    pq.printHeap();
    pq.printTree();
    std::cout << "\n";

    std::cout << "=== Test 2: Priority-based Extraction ===\n";
    std::cout << "Extracting elements in priority order:\n";

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

    struct Task
    {
        int priority;
        std::string name;

        Task(int p, const std::string& n)
        {
            priority = p;
            name = n;
        }

        bool operator>(const Task& other) const
        {
            return priority > other.priority;
        }

        bool operator<=(const Task& other) const
        {
            return priority <= other.priority;
        }

        friend std::ostream& operator<<(std::ostream& os, const Task& task)
        {
            return os << task.name << "(" << task.priority << ")";
        }
    };

    PriorityQueue<Task> taskQueue;

    taskQueue.insert(Task(3, "Email"));
    taskQueue.insert(Task(1, "Documentation"));
    taskQueue.insert(Task(5, "Critical Bug Fix"));
    taskQueue.insert(Task(2, "Code Review"));
    taskQueue.insert(Task(4, "Meeting"));

    std::cout << "Tasks in priority queue:\n";
    taskQueue.printHeap();
    std::cout << "\nProcessing tasks by priority:\n";

    while (!taskQueue.empty())
    {
        Task nextTask = taskQueue.extractMax();
        std::cout << "Processing: " << nextTask << std::endl;
    }

    std::cout << "\n=== ALL TESTS COMPLETED ===\n";

    return 0;
}
