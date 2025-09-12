#include <iostream>
#include <vector>

// Insertion Sort implementation
void insertionSort(std::vector<int>& arr)
{
    int n = arr.size();
    for (int i = 1; i < n; ++i)
    {
        int key = arr[i];
        int j = i - 1;
        // Move elements greater than key to one position ahead
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

// Utility function to print the array
void printArray(const std::vector<int>& arr)
{
    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

int main()
{
    std::vector<int> arr = {5, 2, 9, 1, 5, 6};
    std::cout << "Original array: ";
    printArray(arr);

    insertionSort(arr);

    std::cout << "Sorted array: ";
    printArray(arr);

    return 0;
}

/*
Time Complexity:
    - Best Case:    O(n)      (when the array is already sorted)
    - Average Case: O(n^2)
    - Worst Case:   O(n^2)    (when the array is reverse sorted)
Space Complexity:
    - O(1) (in-place sorting)
*/