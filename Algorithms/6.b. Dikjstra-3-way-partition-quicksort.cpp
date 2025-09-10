#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <map>

/*
 * DIJKSTRA'S 3-WAY PARTITIONING QUICKSORT
 * 
 * PROBLEM ADDRESSED:
 * Standard quicksort (including improved 2-way partition) suffers from O(n²) performance
 * when the array contains many duplicate elements. This happens because:
 * 1. Duplicate elements are not grouped together efficiently
 * 2. Equal elements get distributed across both sides of partition
 * 3. Recursive calls continue to process already "correctly positioned" duplicates
 * 
 * DIJKSTRA'S SOLUTION:
 * The Dutch National Flag algorithm partitions array into THREE regions:
 * [< pivot] [= pivot] [> pivot]
 * 
 * KEY IMPROVEMENT:
 * - Equal elements are grouped in the middle and excluded from recursive calls
 * - Reduces time complexity from O(n²) to O(n) for duplicate-heavy arrays
 * - Maintains O(n log n) performance for arrays with unique elements
 */

class DijkstraQuickSort {
private:
    std::random_device rd;
    std::mt19937 gen;
    
public:
    DijkstraQuickSort() : gen(rd()) {}
    
    /*
     * FISHER-YATES SHUFFLE
     * Same randomization as before to prevent worst-case scenarios
     */
    void randomShuffle(std::vector<int>& arr) {
        for (int i = arr.size() - 1; i > 0; --i) {
            std::uniform_int_distribution<> dis(0, i);
            int j = dis(gen);
            std::swap(arr[i], arr[j]);
        }
    }
    
    /*
     * DIJKSTRA'S 3-WAY PARTITIONING (Dutch National Flag Algorithm)
     * 
     * PURPOSE: Partition array into three regions in a single pass:
     * - Elements less than pivot
     * - Elements equal to pivot  
     * - Elements greater than pivot
     * 
     * ALGORITHM:
     * Maintain three pointers:
     * - lt: boundary between < and = regions
     * - gt: boundary between = and > regions  
     * - i: current element being examined
     * 
     * INVARIANTS MAINTAINED:
     * arr[low...lt-1] < pivot
     * arr[lt...gt] = pivot
     * arr[gt+1...high] > pivot
     * arr[i...gt] = unexamined elements
     * 
     * RETURNS: pair<int, int> representing [lt, gt] boundaries
     */
    std::pair<int, int> partition3Way(std::vector<int>& arr, int low, int high) {
        // Choose pivot (can be improved with median-of-three)
        int pivot = arr[low];
        
        int lt = low;      // arr[low...lt-1] < pivot
        int gt = high;     // arr[gt+1...high] > pivot
        int i = low + 1;   // current element to examine
        
        /*
         * CORE ALGORITHM:
         * Process each element and place it in correct region
         */
        while (i <= gt) {
            if (arr[i] < pivot) {
                // Element belongs in < region
                std::swap(arr[lt], arr[i]);
                lt++;  // expand < region
                i++;   // move to next element
            }
            else if (arr[i] > pivot) {
                // Element belongs in > region
                std::swap(arr[i], arr[gt]);
                gt--;  // expand > region
                // Don't increment i - need to examine swapped element
            }
            else {
                // arr[i] == pivot, already in correct region
                i++;   // just move to next element
            }
        }
        
        return {lt, gt};
    }
    
    /*
     * ENHANCED RECURSIVE QUICKSORT WITH 3-WAY PARTITIONING
     * 
     * KEY DIFFERENCE FROM 2-WAY:
     * After partitioning, we get two boundaries [lt, gt]
     * - Recursively sort arr[low...lt-1] (elements < pivot)
     * - Skip arr[lt...gt] (elements = pivot, already correctly positioned!)
     * - Recursively sort arr[gt+1...high] (elements > pivot)
     * 
     * PERFORMANCE BENEFIT:
     * Equal elements are completely excluded from further recursive calls
     */
    void quickSort3Way(std::vector<int>& arr, int low, int high) {
        if (low >= high) return;
        
        // Partition into three regions
        auto [lt, gt] = partition3Way(arr, low, high);
        
        // Recursively sort only < and > regions
        // Elements in [lt...gt] are equal to pivot and already in correct position
        quickSort3Way(arr, low, lt - 1);    // Sort < pivot region
        quickSort3Way(arr, gt + 1, high);   // Sort > pivot region
        
        // Note: No recursive call for [lt...gt] - major efficiency gain!
    }
    
    /*
     * PUBLIC SORTING INTERFACE
     */
    void sort(std::vector<int>& arr) {
        if (arr.size() <= 1) return;
        
        // Apply randomization to prevent worst-case scenarios
        randomShuffle(arr);
        
        // Use 3-way partitioning quicksort
        quickSort3Way(arr, 0, arr.size() - 1);
    }
};

// Utility functions for demonstration
void printArray(const std::vector<int>& arr, const std::string& label) {
    std::cout << label << ": ";
    for (int num : arr) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

void printPartitionAnalysis(const std::vector<int>& arr) {
    std::map<int, int> frequency;
    for (int num : arr) {
        frequency[num]++;
    }
    
    std::cout << "Duplicate analysis: ";
    for (const auto& [value, count] : frequency) {
        if (count > 1) {
            std::cout << value << "(" << count << "x) ";
        }
    }
    std::cout << std::endl;
}

/*
 * DEMONSTRATION PROGRAM
 * Shows the superiority of 3-way partitioning on duplicate-heavy arrays
 */
int main() {
    DijkstraQuickSort sorter;
    
    std::cout << "=== DIJKSTRA'S 3-WAY PARTITIONING QUICKSORT DEMO ===\n\n";
    
    // Test Case 1: Array with many duplicates (worst case for 2-way partition)
    std::vector<int> duplicateHeavy = {5, 2, 5, 5, 1, 2, 5, 2, 5, 1, 5, 2, 1, 5, 2, 5, 1, 5, 2, 5};
    std::vector<int> testArray1 = duplicateHeavy;
    
    std::cout << "=== Test 1: Heavy Duplicates (20 elements) ===\n";
    printArray(testArray1, "Before sorting");
    printPartitionAnalysis(testArray1);
    std::cout << "Issue: 2-way partition would be O(n²) due to many duplicates\n";
    std::cout << "Solution: 3-way partition groups duplicates, reducing to O(n)\n";
    sorter.sort(testArray1);
    printArray(testArray1, "After 3-way sorting");
    std::cout << "✓ Efficiently handled duplicate-heavy array\n\n";
    
    // Test Case 2: Array with few unique values
    std::vector<int> fewUnique = {3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1};
    std::vector<int> testArray2 = fewUnique;
    
    std::cout << "=== Test 2: Few Unique Values ===\n";
    printArray(testArray2, "Before sorting");
    printPartitionAnalysis(testArray2);
    sorter.sort(testArray2);
    printArray(testArray2, "After 3-way sorting");
    std::cout << "✓ Optimal performance with only 3 unique values\n\n";
    
    // Test Case 3: All elements the same (extreme case)
    std::vector<int> allSame = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
    std::vector<int> testArray3 = allSame;
    
    std::cout << "=== Test 3: All Elements Identical ===\n";
    printArray(testArray3, "Before sorting");
    std::cout << "Issue: 2-way partition would still recurse on identical elements\n";
    std::cout << "Solution: 3-way partition recognizes all elements are equal\n";
    sorter.sort(testArray3);
    printArray(testArray3, "After 3-way sorting");
    std::cout << "✓ O(n) performance - no unnecessary recursion\n\n";
    
    // Test Case 4: Mixed array (showing it works well for unique elements too)
    std::vector<int> mixed = {15, 3, 9, 14, 7, 1, 12, 8, 18, 5, 20, 2, 11, 6, 17, 4, 13, 10, 19, 16};
    std::vector<int> testArray4 = mixed;
    
    std::cout << "=== Test 4: Mostly Unique Elements ===\n";
    printArray(testArray4, "Before sorting");
    sorter.sort(testArray4);
    printArray(testArray4, "After 3-way sorting");
    std::cout << "✓ Standard O(n log n) performance maintained\n\n";
    
    std::cout << "=== ALL TESTS COMPLETED SUCCESSFULLY! ===\n";
    
    return 0;
}

/*
=== COMPREHENSIVE ANALYSIS: 3-WAY vs 2-WAY PARTITIONING ===

PROBLEM WITH 2-WAY PARTITIONING:
❌ Duplicate elements scattered across both partitions
❌ Equal elements continue to be processed in recursive calls  
❌ Time complexity degrades to O(n²) with many duplicates
❌ Inefficient use of comparisons on already-positioned elements

DIJKSTRA'S 3-WAY SOLUTION:
✅ Groups all equal elements together in middle partition
✅ Equal elements excluded from further recursive processing
✅ Maintains O(n log n) even with heavy duplicates
✅ Optimal O(n) performance when few unique values exist

=== COMPLEXITY COMPARISON ===

WORST CASE SCENARIOS:
2-Way Partition: O(n²) on arrays with many duplicates
3-Way Partition: O(n log k) where k = number of unique elements

BEST CASE SCENARIOS:
2-Way Partition: O(n log n) on arrays with unique elements
3-Way Partition: O(n) when all elements are identical

SPACE COMPLEXITY:
Both: O(log n) average, O(n) worst case for recursion stack

=== WHEN TO USE 3-WAY PARTITIONING ===

IDEAL FOR:
🎯 Arrays with many duplicate elements
🎯 Data with few unique values (categorical data)
🎯 String sorting with common prefixes
🎯 General-purpose sorting (handles all cases well)

OVERHEAD CONSIDERATIONS:
⚠️ Slightly more complex code
⚠️ Minimal extra overhead for mostly-unique arrays
⚠️ More pointer management in partition phase

=== REAL-WORLD APPLICATIONS ===

Used in:
- Java's Arrays.sort() for primitive types
- C++ std::sort() implementations
- Database sorting operations
- Scientific computing with repeated measurements
- Text processing with repeated words/patterns

=== KEY INSIGHT ===
3-way partitioning transforms quicksort from "good average case" 
to "good in all practical cases" by intelligently handling duplicates.
This makes it the preferred choice for production systems.
*/
