#include <iostream>
#include <vector>

// Partition function: places the pivot element at its correct position
// and arranges smaller elements to the left and greater to the right.
int partition(std::vector<int>& arr, int low, int high) {
    int pivot = arr[high]; // Choosing the last element as pivot
    int i = low - 1; // Index of smaller element

    for (int j = low; j < high; ++j) {
        // If current element is less than or equal to pivot
        if (arr[j] <= pivot) {
            ++i;
            std::swap(arr[i], arr[j]); // Swap arr[i] and arr[j]
        }
    }
    std::swap(arr[i + 1], arr[high]); // Place pivot in correct position
    return i + 1; // Return the partition index
}

// QuickSort function: recursively sorts elements before and after partition
void quickSort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        // Partition the array and get the pivot index
        int pi = partition(arr, low, high);

        // Recursively sort elements before and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int main() {
    std::vector<int> arr = {10, 7, 8, 9, 1, 5};
    int n = arr.size();

    std::cout << "Original array: ";
    for (int x : arr) std::cout << x << " ";
    std::cout << std::endl;

    quickSort(arr, 0, n - 1);

    std::cout << "Sorted array:   ";
    for (int x : arr) std::cout << x << " ";
    std::cout << std::endl;

    return 0;
}

/*
Time Complexity:
- Best/Average Case: O(n log n)
- Worst Case: O(n^2) (when the smallest or largest element is always chosen as pivot)
Space Complexity:
- O(log n) due to recursion stack (in-place sorting, no extra array used)
*/