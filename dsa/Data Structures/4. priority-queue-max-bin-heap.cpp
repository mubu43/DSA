#include <iostream>
#include <vector>
#include <stdexcept>

/*
 * PRIORITY QUEUE IMPLEMENTATION USING BINARY MAX-HEAP
 * 
 * CONCEPT:
 * A Priority Queue is an abstract data type where elements are served 
 * based on their priority rather than insertion order (unlike regular queues).
 * 
 * KEY PROPERTIES:
 * - Higher priority elements are served before lower priority elements
 * - Elements with same priority are served in any order
 * - Efficient insertion and extraction of highest priority element
 * 
 * IMPLEMENTATION CHOICE - MAX-HEAP:
 * We use a Binary Max-Heap because it provides:
 * - O(log n) insertion
 * - O(log n) extraction of maximum (highest priority)
 * - O(1) access to maximum element
 * - Efficient memory usage (array-based)
 * 
 * MAX-HEAP PROPERTY:
 * In a max-heap, every parent node is GREATER THAN OR EQUAL to its children.
 * This ensures the LARGEST element is always at the root (index 0).
 * For priority queues, larger values = higher priority.
 */

template <typename T>
class PriorityQueue {
private:
    std::vector<T> heap;    // Array representation of binary MAX-HEAP
    
    /*
     * BINARY MAX-HEAP ARRAY INDEXING FORMULAS
     * For element at index i:
     * - Parent index: (i - 1) / 2
     * - Left child index: 2 * i + 1  
     * - Right child index: 2 * i + 2
     * 
     * MAX-HEAP PROPERTY:
     * Every parent node >= its children nodes
     * This ensures the MAXIMUM element is always at the root (index 0)
     * 
     * Example max-heap: [40, 30, 25, 20, 15, 10, 5]
     *        40
     *       /  \
     *      30   25
     *     / |   | \
     *    20 15 10  5
     */
    
    // Get parent index of node at index i
    int parent(int i) const {
        return (i - 1) / 2;
    }
    
    // Get left child index of node at index i
    int leftChild(int i) const {
        return 2 * i + 1;
    }
    
    // Get right child index of node at index i
    int rightChild(int i) const {
        return 2 * i + 2;
    }
    
    /*
     * SWIM OPERATION (Bottom-Up Heapify)
     * Purpose: Restore heap property after insertion
     * 
     * Metaphor: Element "swims up" to its correct level in the heap
     * 
     * Algorithm:
     * 1. Start from newly inserted element
     * 2. Compare with parent
     * 3. If current > parent, swap them (element swims up)
     * 4. Move up and repeat until heap property satisfied
     * 
     * Used after: Insertion at the end of heap
     * Time Complexity: O(log n) - height of tree
     */
    void swim(int index) {
        // Continue until we reach root or heap property is satisfied
        while (index > 0) {
            int parentIndex = parent(index);
            
            // If current element <= parent, max-heap property satisfied
            if (heap[index] <= heap[parentIndex]) {
                break;
            }
            
            // Swap current element with parent (swim up)
            std::swap(heap[index], heap[parentIndex]);
            
            // Move up to parent position
            index = parentIndex;
        }
    }
    
    /*
     * SINK OPERATION (Top-Down Heapify)
     * Purpose: Restore heap property after extraction
     * 
     * Metaphor: Element "sinks down" to its correct level in the heap
     * 
     * Algorithm:
     * 1. Start from root (or given index)
     * 2. Find largest among current node and its children
     * 3. If largest is not current node, swap with largest child (element sinks down)
     * 4. Move down and repeat until heap property satisfied
     * 
     * Used after: Removing root and placing last element at root
     * Time Complexity: O(log n) - height of tree
     */
    void sink(int index) {
        int size = heap.size();
        
        while (true) {
            int largest = index;
            int left = leftChild(index);
            int right = rightChild(index);
            
            // Find largest among current node and its children
            if (left < size && heap[left] > heap[largest]) {
                largest = left;
            }
            
            if (right < size && heap[right] > heap[largest]) {
                largest = right;
            }
            
            // If largest is current node, max-heap property satisfied
            if (largest == index) {
                break;
            }
            
            // Swap current with largest child (sink down)
            std::swap(heap[index], heap[largest]);
            
            // Move down to largest child position
            index = largest;
        }
    }
    
public:
    /*
     * CONSTRUCTOR
     * Initialize empty priority queue
     */
    PriorityQueue() {}
    
    /*
     * CONSTRUCTOR WITH INITIAL CAPACITY
     * Reserve space to avoid frequent reallocations
     */
    PriorityQueue(int capacity) {
        heap.reserve(capacity);
    }
    
    /*
     * INSERT (ENQUEUE) OPERATION
     * Purpose: Add new element while maintaining heap property
     * 
     * Algorithm:
     * 1. Add element at end of array (maintains complete binary tree)
     * 2. Swim up to restore heap property
     * 
     * Time Complexity: O(log n)
     * Space Complexity: O(1) amortized
     */
    void insert(const T& value) {
        // Add element at end of heap
        heap.push_back(value);
        
        // Restore heap property by swimming up
        swim(heap.size() - 1);
    }
    
    /*
     * EXTRACT MAX (DEQUEUE) OPERATION  
     * Purpose: Remove and return highest priority element (MAX in max-heap)
     * 
     * Algorithm:
     * 1. Save root element (maximum)
     * 2. Move last element to root position
     * 3. Remove last element
     * 4. Sink down from root to restore heap property
     * 
     * Time Complexity: O(log n)
     */
    T extractMax() {
        if (empty()) {
            throw std::out_of_range("Priority queue is empty");
        }
        
        // Save maximum element (root)
        T maxElement = heap[0];
        
        // Move last element to root
        heap[0] = heap.back();
        
        // Remove last element
        heap.pop_back();
        
        // Restore heap property if heap is not empty
        if (!empty()) {
            sink(0);
        }
        
        return maxElement;
    }
    
    /*
     * PEEK (TOP) OPERATION
     * Purpose: View highest priority element without removing it
     * 
     * Time Complexity: O(1)
     */
    const T& top() const {
        if (empty()) {
            throw std::out_of_range("Priority queue is empty");
        }
        return heap[0];
    }
    
    /*
     * UTILITY OPERATIONS
     */
    bool empty() const {
        return heap.empty();
    }
    
    size_t size() const {
        return heap.size();
    }
    
    /*
     * DEBUG FUNCTION: Print heap as array
     * Useful for understanding internal structure
     */
    void printHeap() const {
        std::cout << "Heap array: ";
        for (const T& element : heap) {
            std::cout << element << " ";
        }
        std::cout << std::endl;
    }
    
    /*
     * DEBUG FUNCTION: Print heap as tree structure
     * Helps visualize the binary tree
     */
    void printTree() const {
        if (empty()) {
            std::cout << "Empty heap" << std::endl;
            return;
        }
        
        std::cout << "Heap tree structure:" << std::endl;
        printTreeHelper(0, "", true);
    }
    
private:
    void printTreeHelper(int index, const std::string& prefix, bool isLast) const {
        if (index >= heap.size()) return;
        
        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << heap[index] << std::endl;
        
        int left = leftChild(index);
        int right = rightChild(index);
        
        if (left < heap.size() || right < heap.size()) {
            if (right < heap.size()) {
                printTreeHelper(right, prefix + (isLast ? "    " : "│   "), false);
            }
            if (left < heap.size()) {
                printTreeHelper(left, prefix + (isLast ? "    " : "│   "), true);
            }
        }
    }
};

/*
 * DEMONSTRATION PROGRAM
 * Shows priority queue operations and internal heap structure
 */
int main() {
    std::cout << "=== PRIORITY QUEUE DEMONSTRATION ===\n\n";
    
    PriorityQueue<int> pq;
    
    // Test Case 1: Basic Operations
    std::cout << "=== Test 1: Basic Insertion and Extraction ===\n";
    
    std::vector<int> values = {10, 20, 15, 30, 40, 25, 35};
    
    std::cout << "Inserting values: ";
    for (int val : values) {
        std::cout << val << " ";
        pq.insert(val);
    }
    std::cout << "\n\n";
    
    std::cout << "Heap after insertions:\n";
    pq.printHeap();
    pq.printTree();
    std::cout << "\n";
    
    // Test Case 2: Priority-based extraction
    std::cout << "=== Test 2: Priority-based Extraction ===\n";
    std::cout << "Extracting elements in priority order:\n";
    
    while (!pq.empty()) {
        int max = pq.extractMax();
        std::cout << "Extracted: " << max << " | Remaining size: " << pq.size();
        if (!pq.empty()) {
            std::cout << " | New top: " << pq.top();
        }
        std::cout << std::endl;
    }
    std::cout << "\n";
    
    // Test Case 3: Priority Queue for Task Scheduling
    std::cout << "=== Test 3: Task Scheduling Example ===\n";
    
    struct Task {
        int priority;
        std::string name;
        
        Task(int p, const std::string& n) : priority(p), name(n) {}
        
        // Comparison operator for priority (higher number = higher priority)
        bool operator>(const Task& other) const {
            return priority > other.priority;
        }
        
        bool operator<=(const Task& other) const {
            return priority <= other.priority;
        }
        
        friend std::ostream& operator<<(std::ostream& os, const Task& task) {
            return os << task.name << "(" << task.priority << ")";
        }
    };
    
    PriorityQueue<Task> taskQueue;
    
    // Add tasks with different priorities
    taskQueue.insert(Task(3, "Email"));
    taskQueue.insert(Task(1, "Documentation"));
    taskQueue.insert(Task(5, "Critical Bug Fix"));
    taskQueue.insert(Task(2, "Code Review"));
    taskQueue.insert(Task(4, "Meeting"));
    
    std::cout << "Tasks in priority queue:\n";
    taskQueue.printHeap();
    std::cout << "\nProcessing tasks by priority:\n";
    
    while (!taskQueue.empty()) {
        Task nextTask = taskQueue.extractMax();
        std::cout << "Processing: " << nextTask << std::endl;
    }
    
    std::cout << "\n=== ALL TESTS COMPLETED ===\n";
    
    return 0;
}

/*
=== COMPREHENSIVE PRIORITY QUEUE ANALYSIS ===

CORE CONCEPTS:

1. ABSTRACT DATA TYPE:
   - Serves elements by priority, not insertion order
   - Each element has associated priority value
   - Higher priority = served first

2. BINARY HEAP IMPLEMENTATION:
   - Complete binary tree stored in array
   - Max-heap: parent >= children (for max priority queue)
   - Min-heap: parent <= children (for min priority queue)

3. HEAP PROPERTIES:
   - Shape Property: Complete binary tree (filled left to right)
   - Heap Property: Parent-child ordering relationship
   - Root contains highest/lowest priority element

=== OPERATION COMPLEXITIES ===

INSERT (enqueue):
- Time: O(log n) - height of tree
- Space: O(1) amortized
- Process: Add at end + swim up

EXTRACT MAX (dequeue):
- Time: O(log n) - height of tree  
- Space: O(1)
- Process: Remove root + move last to root + sink down

PEEK (top):
- Time: O(1) - direct access to root
- Space: O(1)

SIZE/EMPTY:
- Time: O(1)
- Space: O(1)

=== REAL-WORLD APPLICATIONS ===

1. OPERATING SYSTEMS:
   - Process scheduling (CPU allocation)
   - Memory management
   - I/O request handling

2. NETWORKING:
   - Packet routing and QoS
   - Bandwidth allocation
   - Traffic shaping

3. ALGORITHMS:
   - Dijkstra's shortest path
   - Prim's minimum spanning tree
   - Huffman coding
   - A* pathfinding

4. SIMULATION:
   - Event-driven simulation
   - Discrete event systems
   - Game AI decision making

=== COMPARISON WITH OTHER DATA STRUCTURES ===

vs SORTED ARRAY:
✓ Faster insertion O(log n) vs O(n)
✗ Same extraction time O(log n)

vs UNSORTED ARRAY:
✗ Slower insertion O(log n) vs O(1)
✓ Much faster extraction O(log n) vs O(n)

vs LINKED LIST:
✓ Better time complexities for both operations
✓ Better cache locality (array-based)

vs BALANCED BST:
✓ Simpler implementation
✓ Better constants in practice
✗ Less flexible (only min/max access)

=== KEY INSIGHTS FOR REVISION ===

1. Heap is NOT a sorted structure - only partial ordering
2. Array indexing formulas are crucial for navigation
3. Swim and sink operations maintain the fundamental heap property
4. Complete binary tree ensures O(log n) height
5. Priority queues abstract the priority concept from implementation
6. Choice of max-heap vs min-heap depends on application needs

The beauty of priority queues lies in their ability to efficiently 
maintain partial order while providing fast access to the most 
important element - a perfect balance for many real-world scenarios.

SWIM vs SINK TERMINOLOGY:
- SWIM: Element moves UP the tree (towards root) when it's too large
- SINK: Element moves DOWN the tree (towards leaves) when it's too small
*/
