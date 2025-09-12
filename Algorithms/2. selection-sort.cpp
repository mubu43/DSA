#include <iostream>
#include <vector>

// Selection Sort implementation
void selectionSort(std::vector<int>& arr)
{
    int n = arr.size();
    for (int i = 0; i < n - 1; ++i)
    {
        int minIdx = i;
        // Find the minimum element in unsorted part
        for (int j = i + 1; j < n; ++j)
        {
            if (arr[j] < arr[minIdx])
            {
                minIdx = j;
            }
        }
        // Swap the found minimum with the first element
        std::swap(arr[i], arr[minIdx]);
    }
}

int main()
{
    std::vector<int> arr = {64, 25, 12, 22, 11};
    selectionSort(arr);
    std::cout << "Sorted array: ";
    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    return 0;
}

/*
Time Complexity:
    - Best, Average, Worst case: O(n^2), where n is the number of elements.
Space Complexity:
    - O(1), as selection sort is an in-place sorting algorithm.
*/