#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cassert>

/**
 * Binary Search Implementation
 * 
 * A comprehensive demonstration of binary search algorithm with multiple variants
 * including iterative, recursive, and specialized search functions.
 * 
 * Time Complexity: O(log n)
 * Space Complexity: O(1) iterative, O(log n) recursive
 */

class BinarySearch
{
private:
    // Helper function to print array with highlighting
    void printArrayWithHighlight(const std::vector<int>& arr, int left, int right, int mid = -1)
    {
        std::cout << "Array: [";
        for (int i = 0; i < arr.size(); i++)
        {
            if (i == mid)
            {
                std::cout << "(" << arr[i] << ")";  // Highlight mid element
            }
            else if (i >= left && i <= right)
            {
                std::cout << arr[i];  // Active search range
            }
            else
            {
                std::cout << "·";  // Outside search range
            }
            
            if (i < arr.size() - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "] ";
        std::cout << "Range: [" << left << ", " << right << "]";
        if (mid != -1)
        {
            std::cout << " Mid: " << mid << " (value: " << arr[mid] << ")";
        }
        std::cout << std::endl;
    }

public:
    /**
     * Standard Binary Search - Iterative Version
     * 
     * Searches for target in sorted array using iterative approach.
     * Returns index if found, -1 if not found.
     * 
     * @param arr: Sorted array to search in
     * @param target: Value to search for
     * @param verbose: Print search steps for educational purposes
     * @return: Index of target if found, -1 otherwise
     */
    int binarySearchIterative(const std::vector<int>& arr, int target, bool verbose = false)
    {
        if (arr.empty())
        {
            return -1;
        }

        int left = 0;
        int right = arr.size() - 1;
        int iterations = 0;

        if (verbose)
        {
            std::cout << "\n=== Binary Search (Iterative) for target: " << target << " ===" << std::endl;
        }

        while (left <= right)
        {
            iterations++;
            int mid = left + (right - left) / 2;  // Avoid overflow

            if (verbose)
            {
                std::cout << "Iteration " << iterations << ": ";
                printArrayWithHighlight(arr, left, right, mid);
            }

            if (arr[mid] == target)
            {
                if (verbose)
                {
                    std::cout << "✓ Found target " << target << " at index " << mid << std::endl;
                    std::cout << "Total iterations: " << iterations << std::endl;
                }
                return mid;
            }
            else if (arr[mid] < target)
            {
                if (verbose)
                {
                    std::cout << "  " << arr[mid] << " < " << target << ", search right half" << std::endl;
                }
                left = mid + 1;
            }
            else
            {
                if (verbose)
                {
                    std::cout << "  " << arr[mid] << " > " << target << ", search left half" << std::endl;
                }
                right = mid - 1;
            }
        }

        if (verbose)
        {
            std::cout << "✗ Target " << target << " not found after " << iterations << " iterations" << std::endl;
        }
        return -1;
    }

    /**
     * Binary Search - Recursive Version
     * 
     * Recursive implementation of binary search.
     * 
     * @param arr: Sorted array to search in
     * @param target: Value to search for
     * @param left: Left boundary of search range
     * @param right: Right boundary of search range
     * @return: Index of target if found, -1 otherwise
     */
    int binarySearchRecursive(const std::vector<int>& arr, int target, int left, int right)
    {
        // Base case: search range is empty
        if (left > right)
        {
            return -1;
        }

        int mid = left + (right - left) / 2;

        if (arr[mid] == target)
        {
            return mid;
        }
        else if (arr[mid] < target)
        {
            return binarySearchRecursive(arr, target, mid + 1, right);
        }
        else
        {
            return binarySearchRecursive(arr, target, left, mid - 1);
        }
    }

    // Wrapper for recursive binary search
    int binarySearchRecursive(const std::vector<int>& arr, int target)
    {
        if (arr.empty())
        {
            return -1;
        }
        return binarySearchRecursive(arr, target, 0, arr.size() - 1);
    }

    /**
     * Find First Occurrence (Lower Bound)
     * 
     * Finds the first occurrence of target in array with duplicates.
     * If target not found, returns -1.
     */
    int findFirstOccurrence(const std::vector<int>& arr, int target)
    {
        if (arr.empty())
        {
            return -1;
        }

        int left = 0;
        int right = arr.size() - 1;
        int result = -1;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;

            if (arr[mid] == target)
            {
                result = mid;          // Found target, but continue searching left
                right = mid - 1;       // Look for earlier occurrence
            }
            else if (arr[mid] < target)
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        return result;
    }

    /**
     * Find Last Occurrence (Upper Bound)
     * 
     * Finds the last occurrence of target in array with duplicates.
     * If target not found, returns -1.
     */
    int findLastOccurrence(const std::vector<int>& arr, int target)
    {
        if (arr.empty())
        {
            return -1;
        }

        int left = 0;
        int right = arr.size() - 1;
        int result = -1;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;

            if (arr[mid] == target)
            {
                result = mid;          // Found target, but continue searching right
                left = mid + 1;        // Look for later occurrence
            }
            else if (arr[mid] < target)
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }

        return result;
    }

    /**
     * Find Insert Position
     * 
     * Finds the position where target should be inserted to maintain sorted order.
     * Equivalent to lower_bound in STL.
     */
    int findInsertPosition(const std::vector<int>& arr, int target)
    {
        int left = 0;
        int right = arr.size();  // Note: size(), not size()-1

        while (left < right)
        {
            int mid = left + (right - left) / 2;

            if (arr[mid] < target)
            {
                left = mid + 1;
            }
            else
            {
                right = mid;
            }
        }

        return left;
    }

    /**
     * Search in Rotated Sorted Array
     * 
     * Binary search variant for arrays that are sorted but rotated.
     * Example: [4,5,6,7,0,1,2] is [0,1,2,4,5,6,7] rotated.
     */
    int searchRotatedArray(const std::vector<int>& arr, int target)
    {
        if (arr.empty())
        {
            return -1;
        }

        int left = 0;
        int right = arr.size() - 1;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;

            if (arr[mid] == target)
            {
                return mid;
            }

            // Determine which half is sorted
            if (arr[left] <= arr[mid])  // Left half is sorted
            {
                if (target >= arr[left] && target < arr[mid])
                {
                    right = mid - 1;  // Target in left half
                }
                else
                {
                    left = mid + 1;   // Target in right half
                }
            }
            else  // Right half is sorted
            {
                if (target > arr[mid] && target <= arr[right])
                {
                    left = mid + 1;   // Target in right half
                }
                else
                {
                    right = mid - 1;  // Target in left half
                }
            }
        }

        return -1;
    }

    /**
     * Find Peak Element
     * 
     * Finds a peak element in array where arr[i] > arr[i-1] && arr[i] > arr[i+1].
     * Works even when array is not sorted.
     */
    int findPeak(const std::vector<int>& arr)
    {
        if (arr.empty())
        {
            return -1;
        }
        if (arr.size() == 1)
        {
            return 0;
        }

        int left = 0;
        int right = arr.size() - 1;

        while (left < right)
        {
            int mid = left + (right - left) / 2;

            if (arr[mid] > arr[mid + 1])
            {
                right = mid;  // Peak is in left half (including mid)
            }
            else
            {
                left = mid + 1;  // Peak is in right half
            }
        }

        return left;
    }

    /**
     * Square Root using Binary Search
     * 
     * Finds integer square root of a number using binary search.
     * Returns largest integer whose square is <= x.
     */
    int sqrt(int x)
    {
        if (x < 2)
        {
            return x;
        }

        int left = 1;
        int right = x / 2 + 1;
        int result = 0;

        while (left <= right)
        {
            int mid = left + (right - left) / 2;
            long long square = (long long)mid * mid;  // Avoid overflow

            if (square == x)
            {
                return mid;
            }
            else if (square < x)
            {
                result = mid;     // Store potential answer
                left = mid + 1;   // Look for larger value
            }
            else
            {
                right = mid - 1;  // Look for smaller value
            }
        }

        return result;
    }

    /**
     * Performance Testing
     * 
     * Compares binary search with linear search performance.
     */
    void performanceTest(const std::vector<int>& arr, int target)
    {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Binary Search
        int binaryResult = binarySearchIterative(arr, target);
        auto binaryTime = std::chrono::high_resolution_clock::now();
        
        // Linear Search
        int linearResult = -1;
        for (int i = 0; i < arr.size(); i++)
        {
            if (arr[i] == target)
            {
                linearResult = i;
                break;
            }
        }
        auto linearTime = std::chrono::high_resolution_clock::now();

        auto binaryDuration = std::chrono::duration_cast<std::chrono::microseconds>(binaryTime - start);
        auto linearDuration = std::chrono::duration_cast<std::chrono::microseconds>(linearTime - binaryTime);

        std::cout << "\n=== Performance Comparison ===" << std::endl;
        std::cout << "Array size: " << arr.size() << std::endl;
        std::cout << "Target: " << target << std::endl;
        std::cout << "Binary Search: " << binaryDuration.count() << " μs (result: " << binaryResult << ")" << std::endl;
        std::cout << "Linear Search: " << linearDuration.count() << " μs (result: " << linearResult << ")" << std::endl;
        
        if (linearDuration.count() > 0)
        {
            double speedup = (double)linearDuration.count() / binaryDuration.count();
            std::cout << "Speedup: " << speedup << "x faster" << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    BinarySearch bs;

    std::cout << "=== BINARY SEARCH COMPREHENSIVE DEMONSTRATION ===" << std::endl;

    // Test Case 1: Basic Binary Search
    std::cout << "\n1. BASIC BINARY SEARCH" << std::endl;
    std::vector<int> arr1 = {2, 5, 8, 12, 16, 23, 38, 45, 56, 67, 78};
    
    std::cout << "\nSearching for 23 (verbose mode):" << std::endl;
    bs.binarySearchIterative(arr1, 23, true);
    
    std::cout << "\nSearching for 99 (not present):" << std::endl;
    bs.binarySearchIterative(arr1, 99, true);

    // Test Case 2: Recursive vs Iterative
    std::cout << "\n\n2. RECURSIVE vs ITERATIVE COMPARISON" << std::endl;
    int target = 16;
    int iterResult = bs.binarySearchIterative(arr1, target);
    int recResult = bs.binarySearchRecursive(arr1, target);
    
    std::cout << "Target: " << target << std::endl;
    std::cout << "Iterative result: " << iterResult << std::endl;
    std::cout << "Recursive result: " << recResult << std::endl;
    std::cout << "Results match: " << (iterResult == recResult ? "✓" : "✗") << std::endl;

    // Test Case 3: Arrays with Duplicates
    std::cout << "\n\n3. HANDLING DUPLICATES" << std::endl;
    std::vector<int> arr2 = {1, 2, 2, 2, 3, 4, 4, 5, 5, 5, 5, 6};
    target = 5;
    
    int first = bs.findFirstOccurrence(arr2, target);
    int last = bs.findLastOccurrence(arr2, target);
    int any = bs.binarySearchIterative(arr2, target);
    
    std::cout << "Array with duplicates: ";
    for (int x : arr2) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "Target: " << target << std::endl;
    std::cout << "First occurrence: " << first << std::endl;
    std::cout << "Last occurrence: " << last << std::endl;
    std::cout << "Any occurrence: " << any << std::endl;
    std::cout << "Total occurrences: " << (last - first + 1) << std::endl;

    // Test Case 4: Insert Position
    std::cout << "\n\n4. FINDING INSERT POSITION" << std::endl;
    std::vector<int> arr3 = {1, 3, 5, 6};
    std::vector<int> targets = {5, 2, 7, 0};
    
    std::cout << "Array: ";
    for (int x : arr3) std::cout << x << " ";
    std::cout << std::endl;
    
    for (int t : targets)
    {
        int pos = bs.findInsertPosition(arr3, t);
        std::cout << "Insert " << t << " at position: " << pos << std::endl;
    }

    // Test Case 5: Rotated Array Search
    std::cout << "\n\n5. ROTATED ARRAY SEARCH" << std::endl;
    std::vector<int> rotated = {4, 5, 6, 7, 0, 1, 2};
    std::vector<int> searchTargets = {0, 3, 4, 7};
    
    std::cout << "Rotated array: ";
    for (int x : rotated) std::cout << x << " ";
    std::cout << std::endl;
    
    for (int t : searchTargets)
    {
        int result = bs.searchRotatedArray(rotated, t);
        std::cout << "Search " << t << ": " << (result != -1 ? "found at " + std::to_string(result) : "not found") << std::endl;
    }

    // Test Case 6: Peak Finding
    std::cout << "\n\n6. PEAK ELEMENT FINDING" << std::endl;
    std::vector<std::vector<int>> peakArrays = {
        {1, 2, 3, 1},
        {1, 2, 1, 3, 5, 6, 4},
        {1, 2, 3, 4, 5}
    };
    
    for (const auto& arr : peakArrays)
    {
        int peak = bs.findPeak(arr);
        std::cout << "Array: ";
        for (int x : arr) std::cout << x << " ";
        std::cout << " → Peak at index " << peak << " (value: " << arr[peak] << ")" << std::endl;
    }

    // Test Case 7: Square Root
    std::cout << "\n\n7. SQUARE ROOT USING BINARY SEARCH" << std::endl;
    std::vector<int> numbers = {4, 8, 16, 25, 100, 1000};
    
    for (int num : numbers)
    {
        int root = bs.sqrt(num);
        std::cout << "sqrt(" << num << ") = " << root << " (verification: " << root * root << " <= " << num << " < " << (root + 1) * (root + 1) << ")" << std::endl;
    }

    // Test Case 8: Performance Analysis
    std::cout << "\n\n8. PERFORMANCE ANALYSIS" << std::endl;
    
    // Create large sorted array
    std::vector<int> largeArray;
    for (int i = 0; i < 1000000; i++)
    {
        largeArray.push_back(i * 2);  // Even numbers
    }
    
    // Test with element near the end
    bs.performanceTest(largeArray, 1999998);

    // Edge Cases Testing
    std::cout << "\n\n9. EDGE CASES" << std::endl;
    std::vector<int> empty = {};
    std::vector<int> single = {42};
    std::vector<int> two = {1, 2};
    
    std::cout << "Empty array search for 5: " << bs.binarySearchIterative(empty, 5) << std::endl;
    std::cout << "Single element [42] search for 42: " << bs.binarySearchIterative(single, 42) << std::endl;
    std::cout << "Single element [42] search for 1: " << bs.binarySearchIterative(single, 1) << std::endl;
    std::cout << "Two elements [1,2] search for 1: " << bs.binarySearchIterative(two, 1) << std::endl;
    std::cout << "Two elements [1,2] search for 2: " << bs.binarySearchIterative(two, 2) << std::endl;

    std::cout << "\n=== Binary Search Demonstration Complete ===" << std::endl;
    std::cout << "Key Takeaway: Binary search reduces O(n) linear search to O(log n) by" << std::endl;
    std::cout << "systematically eliminating half the search space in each iteration." << std::endl;

    return 0;
}
