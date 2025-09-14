/*
 * HEAPSORT ALGORITHM IMPLEMENTATION
 * 
 * CONCEPT:
 * Heapsort is a comparison-based sorting algorithm that uses a binary heap data structure.
 * It leverages the heap property to efficiently find and extract the maximum (or minimum) 
 * element repeatedly, resulting in a sorted array.
 * 
 * KEY PROPERTIES:
 * - Time Complexity: O(n log n) in all cases (best, average, worst)
 * - Space Complexity: O(1) - sorts in-place
 * - NOT stable (relative order of equal elements may change)
 * - In-place sorting algorithm
 * - Comparison-based algorithm
 * 
 * ALGORITHM STEPS:
 * 1. Build a max-heap from the input array
 * 2. Repeatedly extract the maximum element and place it at the end
 * 3. Reduce heap size and re-heapify
 * 4. Continue until all elements are sorted
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

class HeapSort 
{
private:
    /*
     * ARRAY INDEXING FOR BINARY HEAP
     * For element at index i:
     * - Parent: (i - 1) / 2
     * - Left child: 2 * i + 1
     * - Right child: 2 * i + 2
     */
    
    int parent(int i) 
    {
        return (i - 1) / 2;
    }
    
    int leftChild(int i) 
    {
        return 2 * i + 1;
    }
    
    int rightChild(int i) 
    {
        return 2 * i + 2;
    }
    
    /*
     * MAX-HEAPIFY (SINK OPERATION)
     * Purpose: Maintain max-heap property by moving element down
     * 
     * Assumption: Left and right subtrees are already max-heaps
     * Action: Ensure the subtree rooted at index i satisfies max-heap property
     * 
     * Time Complexity: O(log n)
     */
    void maxHeapify(std::vector<int>& arr, int heapSize, int i) 
    {
        int largest = i;
        int left = leftChild(i);
        int right = rightChild(i);
        
        // Find largest among root, left child, and right child
        if (left < heapSize && arr[left] > arr[largest]) 
        {
            largest = left;
        }
        
        if (right < heapSize && arr[right] > arr[largest]) 
        {
            largest = right;
        }
        
        // If largest is not root, swap and continue heapifying
        if (largest != i) 
        {
            std::swap(arr[i], arr[largest]);
            maxHeapify(arr, heapSize, largest);
        }
    }
    
    /*
     * BUILD MAX-HEAP
     * Purpose: Convert an arbitrary array into a max-heap
     * 
     * Strategy: Start from the last non-leaf node and heapify each node
     * Last non-leaf node is at index (n/2 - 1)
     * 
     * Time Complexity: O(n) - not O(n log n)!
     * This is because most nodes are near the bottom and require less work
     */
    void buildMaxHeap(std::vector<int>& arr) 
    {
        int n = arr.size();
        
        // Start from last non-leaf node and heapify each node
        for (int i = n / 2 - 1; i >= 0; i--) 
        {
            maxHeapify(arr, n, i);
        }
    }
    
public:
    /*
     * HEAPSORT MAIN ALGORITHM
     * 
     * Steps:
     * 1. Build max-heap from input array - O(n)
     * 2. For each element (from last to first):
     *    a. Swap root (maximum) with last element - O(1)
     *    b. Reduce heap size by 1 - O(1)
     *    c. Restore heap property from root - O(log n)
     * 
     * Total Time Complexity: O(n) + n * O(log n) = O(n log n)
     */
    void heapSort(std::vector<int>& arr) 
    {
        int n = arr.size();
        
        if (n <= 1) 
        {
            return; // Already sorted
        }
        
        // Step 1: Build max-heap
        buildMaxHeap(arr);
        
        // Step 2: Extract elements one by one
        for (int i = n - 1; i > 0; i--) 
        {
            // Move current root (maximum) to end
            std::swap(arr[0], arr[i]);
            
            // Reduce heap size and heapify root
            maxHeapify(arr, i, 0);
        }
    }
    
    /*
     * ITERATIVE VERSION OF MAX-HEAPIFY
     * Alternative implementation without recursion
     */
    void maxHeapifyIterative(std::vector<int>& arr, int heapSize, int i) 
    {
        while (true) 
        {
            int largest = i;
            int left = leftChild(i);
            int right = rightChild(i);
            
            if (left < heapSize && arr[left] > arr[largest]) 
            {
                largest = left;
            }
            
            if (right < heapSize && arr[right] > arr[largest]) 
            {
                largest = right;
            }
            
            if (largest == i) 
            {
                break; // Heap property satisfied
            }
            
            std::swap(arr[i], arr[largest]);
            i = largest;
        }
    }
    
    /*
     * HEAPSORT WITH ITERATIVE HEAPIFY
     */
    void heapSortIterative(std::vector<int>& arr) 
    {
        int n = arr.size();
        
        if (n <= 1) 
        {
            return;
        }
        
        // Build max-heap
        for (int i = n / 2 - 1; i >= 0; i--) 
        {
            maxHeapifyIterative(arr, n, i);
        }
        
        // Extract elements
        for (int i = n - 1; i > 0; i--) 
        {
            std::swap(arr[0], arr[i]);
            maxHeapifyIterative(arr, i, 0);
        }
    }
    
    /*
     * UTILITY FUNCTIONS
     */
    void printArray(const std::vector<int>& arr, const std::string& label) 
    {
        std::cout << label << ": [";
        for (size_t i = 0; i < arr.size(); i++) 
        {
            std::cout << arr[i];
            if (i < arr.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
    
    void printHeapStructure(const std::vector<int>& arr) 
    {
        int n = arr.size();
        if (n == 0) return;
        
        std::cout << "Heap structure:" << std::endl;
        printHeapHelper(arr, 0, "", true);
    }
    
private:
    void printHeapHelper(const std::vector<int>& arr, int index, const std::string& prefix, bool isLast) 
    {
        if (index >= arr.size()) return;
        
        std::cout << prefix;
        std::cout << (isLast ? "└── " : "├── ");
        std::cout << arr[index] << std::endl;
        
        int left = leftChild(index);
        int right = rightChild(index);
        
        if (left < arr.size() || right < arr.size()) 
        {
            if (right < arr.size()) 
            {
                printHeapHelper(arr, right, prefix + (isLast ? "    " : "│   "), false);
            }
            if (left < arr.size()) 
            {
                printHeapHelper(arr, left, prefix + (isLast ? "    " : "│   "), true);
            }
        }
    }
};

/*
 * DEMONSTRATION AND TESTING
 */
int main() 
{
    std::cout << "=== HEAPSORT ALGORITHM DEMONSTRATION ===\n\n";
    
    HeapSort sorter;
    
    // Test Case 1: Basic sorting
    std::cout << "=== Test 1: Basic Heapsort ===\n";
    std::vector<int> arr1 = {64, 34, 25, 12, 22, 11, 90};
    
    sorter.printArray(arr1, "Original array");
    
    // Show heap structure after building max-heap
    std::vector<int> heapDemo = arr1;
    // Build max-heap manually to show structure
    int n = heapDemo.size();
    for (int i = n / 2 - 1; i >= 0; i--) 
    {
        // Simple max-heapify for demonstration
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        
        if (left < n && heapDemo[left] > heapDemo[largest]) largest = left;
        if (right < n && heapDemo[right] > heapDemo[largest]) largest = right;
        
        if (largest != i) 
        {
            std::swap(heapDemo[i], heapDemo[largest]);
        }
    }
    
    sorter.printArray(heapDemo, "After building max-heap");
    sorter.printHeapStructure(heapDemo);
    
    sorter.heapSort(arr1);
    sorter.printArray(arr1, "Sorted array");
    std::cout << std::endl;
    
    // Test Case 2: Edge cases
    std::cout << "=== Test 2: Edge Cases ===\n";
    
    // Empty array
    std::vector<int> arr2;
    sorter.heapSort(arr2);
    sorter.printArray(arr2, "Empty array");
    
    // Single element
    std::vector<int> arr3 = {42};
    sorter.heapSort(arr3);
    sorter.printArray(arr3, "Single element");
    
    // Already sorted
    std::vector<int> arr4 = {1, 2, 3, 4, 5};
    sorter.printArray(arr4, "Already sorted (before)");
    sorter.heapSort(arr4);
    sorter.printArray(arr4, "Already sorted (after)");
    
    // Reverse sorted
    std::vector<int> arr5 = {5, 4, 3, 2, 1};
    sorter.printArray(arr5, "Reverse sorted (before)");
    sorter.heapSort(arr5);
    sorter.printArray(arr5, "Reverse sorted (after)");
    
    // Duplicates
    std::vector<int> arr6 = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    sorter.printArray(arr6, "With duplicates (before)");
    sorter.heapSort(arr6);
    sorter.printArray(arr6, "With duplicates (after)");
    std::cout << std::endl;
    
    // Test Case 3: Performance comparison
    std::cout << "=== Test 3: Performance Test ===\n";
    
    // Generate random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 10000);
    
    const int size = 10000;
    std::vector<int> largeArr;
    largeArr.reserve(size);
    
    for (int i = 0; i < size; i++) 
    {
        largeArr.push_back(dis(gen));
    }
    
    // Test recursive version
    std::vector<int> arr7 = largeArr;
    auto start = std::chrono::high_resolution_clock::now();
    sorter.heapSort(arr7);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Recursive heapsort on " << size << " elements: " 
              << duration.count() << " microseconds" << std::endl;
    
    // Test iterative version
    std::vector<int> arr8 = largeArr;
    start = std::chrono::high_resolution_clock::now();
    sorter.heapSortIterative(arr8);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Iterative heapsort on " << size << " elements: " 
              << duration.count() << " microseconds" << std::endl;
    
    // Verify both give same result
    bool same = (arr7 == arr8);
    std::cout << "Both versions produce same result: " << (same ? "Yes" : "No") << std::endl;
    
    // Verify sorting correctness
    bool sorted = std::is_sorted(arr7.begin(), arr7.end());
    std::cout << "Array is correctly sorted: " << (sorted ? "Yes" : "No") << std::endl;
    
    std::cout << "\n=== HEAPSORT DEMONSTRATION COMPLETE ===\n";
    
    return 0;
}

/*
=== HEAPSORT ALGORITHM ANALYSIS ===

CORE CONCEPTS:

1. HEAP DATA STRUCTURE:
   - Complete binary tree with heap property
   - Max-heap: parent >= children
   - Min-heap: parent <= children
   - Array representation: efficient memory usage

2. HEAPIFY OPERATIONS:
   - Max-heapify: maintain heap property top-down
   - Build-heap: convert array to heap bottom-up
   - Extract-max: remove root and re-heapify

3. SORTING PROCESS:
   - Build max-heap from unsorted array
   - Repeatedly extract maximum to build sorted portion
   - In-place sorting with O(1) extra space

=== COMPLEXITY ANALYSIS ===

TIME COMPLEXITY:
- Build heap: O(n) - mathematical proof using series
- Extract operations: n * O(log n) = O(n log n)
- Overall: O(n log n) for all cases

SPACE COMPLEXITY:
- O(1) for iterative version
- O(log n) for recursive version (call stack)

=== COMPARISON WITH OTHER ALGORITHMS ===

vs QUICKSORT:
✓ Guaranteed O(n log n) worst case
✗ Slower average case performance
✗ Not stable
✓ In-place sorting

vs MERGESORT:
✓ O(1) space complexity
✗ Not stable
✓ In-place sorting
✗ Worse cache performance

vs INSERTION SORT:
✓ Better for large datasets O(n log n) vs O(n²)
✗ Worse for small datasets (overhead)
✗ Not stable

=== PRACTICAL CONSIDERATIONS ===

WHEN TO USE HEAPSORT:
- Need guaranteed O(n log n) performance
- Memory is limited (in-place sorting)
- Stability is not required
- Input size is large

WHEN NOT TO USE:
- Need stable sorting
- Working with small datasets
- Cache performance is critical
- Need best average-case performance

=== KEY INSIGHTS FOR REVISION ===

1. Heapsort combines heap data structure with sorting
2. Build-heap is O(n), not O(n log n) - important optimization
3. Two phases: heap construction + repeated extraction
4. In-place algorithm with optimal worst-case complexity
5. Not stable but guarantees consistent performance
6. Array indexing formulas are crucial for implementation

The elegance of heapsort lies in its use of the heap property 
to efficiently maintain partial order while gradually building 
the final sorted sequence.
*/
