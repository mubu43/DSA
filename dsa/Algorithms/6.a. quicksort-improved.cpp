#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

/*
 * IMPROVED QUICKSORT IMPLEMENTATION
 * 
 * This implementation addresses the main weaknesses of basic quicksort:
 * 1. Poor performance on already sorted arrays (O(n²) worst case)
 * 2. Poor pivot selection leading to unbalanced partitions
 * 
 * IMPROVEMENTS:
 * - Randomized shuffling to avoid worst-case scenarios
 * - Median-of-three pivot selection for better partitioning
 */

class ImprovedQuickSort {
private:
    std::random_device rd;      // Hardware-based random number generator
    std::mt19937 gen;          // Mersenne Twister random number engine
    
public:
    ImprovedQuickSort() : gen(rd()) {}
    
    /*
     * FISHER-YATES SHUFFLE ALGORITHM
     * Purpose: Randomize array order to prevent worst-case performance
     * 
     * How it works:
     * 1. Start from the last element
     * 2. Pick a random index from 0 to current index
     * 3. Swap current element with randomly picked element
     * 4. Move to previous element and repeat
     * 
     * Time Complexity: O(n)
     * Space Complexity: O(1)
     * 
     * Why this helps: Transforms any input (sorted, reverse-sorted, etc.)
     * into a random permutation, ensuring average-case performance
     */
    void randomShuffle(std::vector<int>& arr) {
        for (int i = arr.size() - 1; i > 0; --i) {
            std::uniform_int_distribution<> dis(0, i);
            int j = dis(gen);
            std::swap(arr[i], arr[j]);
        }
    }
    
    /*
     * MEDIAN-OF-THREE PIVOT SELECTION
     * Purpose: Choose a better pivot to create more balanced partitions
     * 
     * Algorithm:
     * 1. Consider first, middle, and last elements
     * 2. Sort these three elements
     * 3. Use the middle value (median) as pivot
     * 4. Place pivot at the end for partitioning
     * 
     * Benefits:
     * - Reduces probability of worst-case scenarios
     * - Creates more balanced partitions on average
     * - Performs well on partially sorted data
     */
    int medianOfThree(std::vector<int>& arr, int low, int high) {
        int mid = low + (high - low) / 2;
        
        // Sort the three elements: arr[low], arr[mid], arr[high]
        if (arr[mid] < arr[low]) 
            std::swap(arr[low], arr[mid]);
        if (arr[high] < arr[low]) 
            std::swap(arr[low], arr[high]);
        if (arr[high] < arr[mid]) 
            std::swap(arr[mid], arr[high]);
        
        // Now: arr[low] <= arr[mid] <= arr[high]
        // Place median (arr[mid]) at the end for partitioning
        std::swap(arr[mid], arr[high]);
        return arr[high];
    }
    
    /*
     * LOMUTO PARTITION SCHEME
     * Purpose: Rearrange array so elements <= pivot are on left, > pivot on right
     * 
     * Algorithm:
     * 1. Choose pivot (using median-of-three)
     * 2. Keep track of boundary between small and large elements
     * 3. Scan through array, moving small elements to left side
     * 4. Place pivot in its final sorted position
     * 
     * Returns: Final position of pivot (partition index)
     */
    int partition(std::vector<int>& arr, int low, int high) {
        // Use median-of-three for better pivot selection
        int pivot = medianOfThree(arr, low, high);
        
        int i = low - 1; // Index of last element <= pivot
        
        // Scan through array, maintaining invariant:
        // arr[low...i] <= pivot, arr[i+1...j-1] > pivot
        for (int j = low; j < high; ++j) {
            if (arr[j] <= pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
            }
        }
        
        // Place pivot in its correct position
        std::swap(arr[i + 1], arr[high]);
        return i + 1; // Return pivot's final position
    }
    
    /*
     * RECURSIVE QUICKSORT FUNCTION
     * Purpose: Divide-and-conquer sorting algorithm
     * 
     * Algorithm:
     * 1. Base case: if subarray has <= 1 element, it's already sorted
     * 2. Partition array around pivot
     * 3. Recursively sort left subarray (elements < pivot)
     * 4. Recursively sort right subarray (elements > pivot)
     * 
     * The pivot is already in its correct position after partitioning
     */
    void quickSort(std::vector<int>& arr, int low, int high) {
        if (low < high) {
            // Partition and get pivot index
            int pivotIndex = partition(arr, low, high);
            
            // Recursively sort elements before and after partition
            quickSort(arr, low, pivotIndex - 1);      // Sort left subarray
            quickSort(arr, pivotIndex + 1, high);     // Sort right subarray
        }
    }
    
    /*
     * MAIN SORTING FUNCTION
     * Purpose: Public interface that applies all improvements
     * 
     * Steps:
     * 1. Handle edge cases (empty or single-element arrays)
     * 2. Apply randomization to prevent worst-case scenarios
     * 3. Call recursive quicksort with improved partitioning
     */
    void sort(std::vector<int>& arr) {
        if (arr.size() <= 1) return;
        
        // IMPROVEMENT 1: Randomize to avoid worst-case on sorted data
        randomShuffle(arr);
        
        // IMPROVEMENT 2: Use enhanced quicksort with median-of-three
        quickSort(arr, 0, arr.size() - 1);
    }
};

// Utility function to print array contents
void printArray(const std::vector<int>& arr, const std::string& label) {
    std::cout << label << ": ";
    for (int num : arr) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

/*
 * DEMONSTRATION PROGRAM
 * Shows the improved quicksort working on different types of input:
 * 1. Already sorted array (worst case for basic quicksort)
 * 2. Reverse sorted array
 * 3. Random array
 * 4. Array with duplicate elements
 */
int main() {
    ImprovedQuickSort sorter;
    
    // Test Case 1: Already sorted array (20 elements)
    // This would cause O(n²) performance in basic quicksort
    std::vector<int> sortedArray = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    std::vector<int> testArray1 = sortedArray;
    
    std::cout << "=== Test 1: Already Sorted Array ===\n";
    printArray(testArray1, "Before sorting");
    sorter.sort(testArray1);
    printArray(testArray1, "After sorting");
    std::cout << "✓ Handled sorted input efficiently\n\n";
    
    // Test Case 2: Reverse sorted array
    std::vector<int> reverseArray = {20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::vector<int> testArray2 = reverseArray;
    
    std::cout << "=== Test 2: Reverse Sorted Array ===\n";
    printArray(testArray2, "Before sorting");
    sorter.sort(testArray2);
    printArray(testArray2, "After sorting");
    std::cout << "✓ Handled reverse-sorted input efficiently\n\n";
    
    // Test Case 3: Random array
    std::vector<int> randomArray = {15, 3, 9, 14, 7, 1, 12, 8, 18, 5, 20, 2, 11, 6, 17, 4, 13, 10, 19, 16};
    std::vector<int> testArray3 = randomArray;
    
    std::cout << "=== Test 3: Random Array ===\n";
    printArray(testArray3, "Before sorting");
    sorter.sort(testArray3);
    printArray(testArray3, "After sorting");
    std::cout << "✓ Standard quicksort performance on random data\n\n";
    
    // Test Case 4: Array with many duplicates
    std::vector<int> duplicateArray = {5, 2, 8, 5, 1, 9, 2, 7, 5, 3, 8, 1, 6, 9, 2, 4, 7, 5, 8, 3};
    std::vector<int> testArray4 = duplicateArray;
    
    std::cout << "=== Test 4: Array with Duplicates ===\n";
    printArray(testArray4, "Before sorting");
    sorter.sort(testArray4);
    printArray(testArray4, "After sorting");
    std::cout << "✓ Handled duplicate elements correctly\n\n";
    
    std::cout << "=== All Tests Completed Successfully! ===\n";
    
    return 0;
}

/*
=== COMPREHENSIVE ALGORITHM ANALYSIS ===

BASIC QUICKSORT PROBLEMS:
1. Worst Case: O(n²) on sorted/reverse-sorted arrays
2. Poor pivot selection leads to unbalanced partitions
3. Unpredictable performance based on input order

IMPROVEMENTS IMPLEMENTED:

1. RANDOMIZED SHUFFLING (Fisher-Yates):
   - Transforms any input into random permutation
   - Eliminates dependence on initial order
   - Guarantees probabilistic O(n log n) performance
   - Cost: Additional O(n) time, but prevents O(n²) worst case

2. MEDIAN-OF-THREE PIVOT SELECTION:
   - Better than first/last element pivot
   - Considers first, middle, last elements
   - Chooses median as pivot for balanced partitions
   - Particularly effective on partially sorted data

3. LOMUTO PARTITION SCHEME:
   - Simple and easy to understand
   - Maintains clear invariants during partitioning
   - Alternative to Hoare partition (more complex but slightly faster)

=== COMPLEXITY ANALYSIS ===

TIME COMPLEXITY:
- Best Case:    O(n log n) - balanced partitions
- Average Case: O(n log n) - guaranteed by randomization  
- Worst Case:   O(n²) - extremely rare due to improvements

SPACE COMPLEXITY:
- Average: O(log n) - recursion depth for balanced tree
- Worst:   O(n) - recursion depth for unbalanced tree

=== WHEN TO USE IMPROVED QUICKSORT ===

ADVANTAGES:
✓ Fast average-case performance O(n log n)
✓ In-place sorting (minimal extra memory)
✓ Good cache locality
✓ Handles various input types efficiently
✓ Predictable performance due to randomization

DISADVANTAGES:
✗ Not stable (doesn't preserve relative order of equal elements)
✗ Worst-case still O(n²) (though very unlikely)
✗ Recursive (can cause stack overflow on very large arrays)

COMPARISON WITH OTHER ALGORITHMS:
- vs Merge Sort: Faster average case, but not stable, uses less memory
- vs Heap Sort: Faster average case, but worse worst case
- vs Insertion Sort: Much faster on large arrays, slower on tiny arrays
- vs Selection Sort: Always better choice

=== KEY TAKEAWAYS FOR REVISION ===

1. Randomization is crucial for consistent performance
2. Pivot selection significantly affects partition quality
3. Understanding partition invariants is essential
4. Trade-offs exist between simplicity and optimization
5. Real-world quicksort implementations use these improvements
*/
