/**
 * 4. Heap Data Structure
 *
 * A heap is a complete binary tree stored in an array where every parent
 * node satisfies the heap property with respect to its children.
 *
 * Two types:
 * - Max-Heap: parent >= children (largest element at root)
 * - Min-Heap: parent <= children (smallest element at root)
 *
 * Array representation (0-indexed):
 * - Parent of node i:      (i - 1) / 2
 * - Left child of node i:  2*i + 1
 * - Right child of node i: 2*i + 2
 *
 * Key operations:
 * - insert:     O(log n) - add element and bubble up
 * - extractMax: O(log n) - remove root and heapify down
 * - peek:       O(1)     - view root element
 * - heapify:    O(n)     - build heap from array
 */

#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class MaxHeap {
private:
    vector<int> data;
    
    // Get index of parent, left child, right child
    int parent(int i) const { return (i - 1) / 2; }
    int left(int i)   const { return 2 * i + 1; }
    int right(int i)  const { return 2 * i + 2; }
    
    /**
     * Bubble Up (Swim)
     * 
     * After inserting at the end, restore heap property by
     * swapping with parent until parent is larger or we reach root.
     *
     *       50                50                 60
     *      /  \     →        /  \      →        /  \
     *    30    40          30    60           30    50
     *   /  \  /  \        /  \  /  \         /  \  /  \
     *  10 20 25 [60]     10 20 25  40       10 20 25  40
     */
    void bubbleUp(int i) {
        while (i > 0 && data[parent(i)] < data[i]) {
            swap(data[parent(i)], data[i]);
            i = parent(i);
        }
    }
    
    /**
     * Bubble Down (Sink / Heapify)
     * 
     * After removing root (replaced with last element), restore heap
     * property by swapping with the larger child until heap property holds.
     *
     *       10                50                 50
     *      /  \      →       /  \      →        /  \
     *    50    40          10    40           30    40
     *   /  \              /  \               /  \
     *  30  20            30  20             10  20
     */
    void bubbleDown(int i) {
        int size = data.size();
        while (true) {
            int largest = i;
            int l = left(i);
            int r = right(i);
            
            // Compare with left child
            if (l < size && data[l] > data[largest])
                largest = l;
            
            // Compare with right child
            if (r < size && data[r] > data[largest])
                largest = r;
            
            // If largest is current node, heap property satisfied
            if (largest == i) break;
            
            swap(data[i], data[largest]);
            i = largest;
        }
    }

public:
    MaxHeap() {}
    
    // Build heap from existing array - O(n)
    MaxHeap(const vector<int>& arr) : data(arr) {
        // Start from last non-leaf node and heapify down
        // Last non-leaf node = parent of last element
        for (int i = parent(data.size() - 1); i >= 0; i--) {
            bubbleDown(i);
        }
    }
    
    // Insert element - O(log n)
    void insert(int value) {
        data.push_back(value);
        bubbleUp(data.size() - 1);
    }
    
    // Get maximum (root) without removing - O(1)
    int peek() const {
        if (data.empty()) throw runtime_error("Heap is empty");
        return data[0];
    }
    
    // Remove and return maximum - O(log n)
    int extractMax() {
        if (data.empty()) throw runtime_error("Heap is empty");
        
        int maxVal = data[0];
        data[0] = data.back();
        data.pop_back();
        
        if (!data.empty()) {
            bubbleDown(0);
        }
        
        return maxVal;
    }
    
    // Get size
    int size() const { return data.size(); }
    
    // Check if empty
    bool empty() const { return data.empty(); }
    
    // Print heap as array
    void print() const {
        cout << "Heap: [";
        for (size_t i = 0; i < data.size(); i++) {
            cout << data[i];
            if (i < data.size() - 1) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    // Print heap as tree (visual representation)
    void printTree() const {
        if (data.empty()) {
            cout << "(empty heap)" << endl;
            return;
        }
        
        int level = 0;
        int levelSize = 1;
        int idx = 0;
        
        while (idx < (int)data.size()) {
            // Print indentation
            int spaces = (1 << (4 - level)) - 1;
            for (int s = 0; s < spaces; s++) cout << " ";
            
            // Print nodes at this level
            for (int i = 0; i < levelSize && idx < (int)data.size(); i++, idx++) {
                cout << data[idx];
                int gap = (1 << (5 - level)) - 1;
                for (int g = 0; g < gap; g++) cout << " ";
            }
            cout << endl;
            
            level++;
            levelSize *= 2;
        }
    }
};

// ============================================================================
// Min-Heap Implementation (just flip the comparisons)
// ============================================================================
class MinHeap {
private:
    vector<int> data;
    
    int parent(int i) const { return (i - 1) / 2; }
    int left(int i)   const { return 2 * i + 1; }
    int right(int i)  const { return 2 * i + 2; }
    
    void bubbleUp(int i) {
        while (i > 0 && data[parent(i)] > data[i]) {  // Note: > instead of <
            swap(data[parent(i)], data[i]);
            i = parent(i);
        }
    }
    
    void bubbleDown(int i) {
        int size = data.size();
        while (true) {
            int smallest = i;
            int l = left(i);
            int r = right(i);
            
            if (l < size && data[l] < data[smallest])
                smallest = l;
            if (r < size && data[r] < data[smallest])
                smallest = r;
            
            if (smallest == i) break;
            
            swap(data[i], data[smallest]);
            i = smallest;
        }
    }

public:
    void insert(int value) {
        data.push_back(value);
        bubbleUp(data.size() - 1);
    }
    
    int peek() const {
        if (data.empty()) throw runtime_error("Heap is empty");
        return data[0];
    }
    
    int extractMin() {
        if (data.empty()) throw runtime_error("Heap is empty");
        int minVal = data[0];
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) bubbleDown(0);
        return minVal;
    }
    
    bool empty() const { return data.empty(); }
    int size() const { return data.size(); }
};

// ============================================================================
// Heapsort using a Max-Heap - O(n log n)
// ============================================================================
void heapsort(vector<int>& arr) {
    int n = arr.size();
    
    // Build max heap - O(n)
    for (int i = n / 2 - 1; i >= 0; i--) {
        // Heapify subtree rooted at i
        int idx = i;
        while (true) {
            int largest = idx;
            int l = 2 * idx + 1;
            int r = 2 * idx + 2;
            
            if (l < n && arr[l] > arr[largest]) largest = l;
            if (r < n && arr[r] > arr[largest]) largest = r;
            
            if (largest == idx) break;
            swap(arr[idx], arr[largest]);
            idx = largest;
        }
    }
    
    // Extract elements one by one
    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);  // Move max to end
        
        // Heapify reduced heap
        int idx = 0;
        while (true) {
            int largest = idx;
            int l = 2 * idx + 1;
            int r = 2 * idx + 2;
            
            if (l < i && arr[l] > arr[largest]) largest = l;
            if (r < i && arr[r] > arr[largest]) largest = r;
            
            if (largest == idx) break;
            swap(arr[idx], arr[largest]);
            idx = largest;
        }
    }
}

// ============================================================================
// Demo
// ============================================================================
int main() {
    cout << "=== MAX-HEAP DEMO ===" << endl << endl;
    
    // 1. Basic operations
    cout << "1. Basic Insert and Extract:" << endl;
    MaxHeap heap;
    
    cout << "   Inserting: 10, 20, 15, 30, 40" << endl;
    heap.insert(10);
    heap.insert(20);
    heap.insert(15);
    heap.insert(30);
    heap.insert(40);
    
    heap.print();
    cout << "   Tree view:" << endl;
    heap.printTree();
    
    cout << "   Peek (max): " << heap.peek() << endl;
    cout << "   Extract max: " << heap.extractMax() << endl;
    cout << "   Extract max: " << heap.extractMax() << endl;
    heap.print();
    cout << endl;
    
    // 2. Build heap from array
    cout << "2. Build Heap from Array [3, 1, 6, 5, 2, 4]:" << endl;
    vector<int> arr = {3, 1, 6, 5, 2, 4};
    MaxHeap heap2(arr);
    heap2.print();
    heap2.printTree();
    cout << endl;
    
    // 3. Min-heap demo
    cout << "3. Min-Heap Demo:" << endl;
    MinHeap minHeap;
    cout << "   Inserting: 50, 30, 40, 10, 20" << endl;
    minHeap.insert(50);
    minHeap.insert(30);
    minHeap.insert(40);
    minHeap.insert(10);
    minHeap.insert(20);
    
    cout << "   Extracting in order: ";
    while (!minHeap.empty()) {
        cout << minHeap.extractMin() << " ";
    }
    cout << endl << endl;
    
    // 4. Heapsort
    cout << "4. Heapsort Demo:" << endl;
    vector<int> sortArr = {64, 34, 25, 12, 22, 11, 90};
    cout << "   Before: ";
    for (int x : sortArr) cout << x << " ";
    cout << endl;
    
    heapsort(sortArr);
    
    cout << "   After:  ";
    for (int x : sortArr) cout << x << " ";
    cout << endl;
    
    return 0;
}
