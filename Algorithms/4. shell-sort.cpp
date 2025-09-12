#include <iostream>
#include <vector>

// Shell Sort with initial gap h=4
void shellSort(std::vector<int>& arr)
{
    int n = arr.size();
    // Start with gap h=4, then reduce by half each time
    for (int gap = 4; gap > 0; gap /= 2)
    {
        // Perform gapped insertion sort for this gap size
        for (int i = gap; i < n; ++i)
        {
            int temp = arr[i];
            int j;
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap)
            {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }
}

int main()
{
    std::vector<int> arr = {23, 12, 1, 8, 34, 54, 2, 3, 99, 17, 45, 67, 89, 21, 4, 6, 10, 5, 77, 32};

    std::cout << "Original array:\n";
    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";

    shellSort(arr);

    std::cout << "Sorted array:\n";
    for (int num : arr)
    {
        std::cout << num << " ";
    }
    std::cout << "\n";

    return 0;
}

/*
Time Complexity:
- Worst case: O(n^2) (for some gap sequences), but typically much better in practice.
- Average case: Between O(n^1.3) and O(n^1.5) depending on gap sequence.
- Best case: O(n log n) for some gap sequences.

Space Complexity:
- O(1) extra space (in-place sorting).
*/