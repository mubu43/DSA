/**
 * 033. Search in Rotated Sorted Array
 *
 * Problem (paraphrased, detailed):
 * You are given a rotated sorted array of unique integers and a target. Return the index of target if found; otherwise return -1.
 * Array is formed by taking an increasing array and rotating it at some pivot unknown to you beforehand.
 *
 * Examples:
 * - nums = [4,5,6,7,0,1,2], target = 0 -> 4
 * - nums = [4,5,6,7,0,1,2], target = 3 -> -1
 * - nums = [1], target = 0 -> -1
 *
 * Intuition:
 * - In a rotated array, at least one half of the current search range is still sorted.
 * - We can do binary search while determining which half is sorted and whether the target lies in that half.
 *
 * Algorithm (Modified Binary Search):
 * - While l <= r:
 *     - m = (l + r) / 2. If nums[m] == target, return m.
 *     - If nums[l] <= nums[m], the left half is sorted.
 *         - If nums[l] <= target < nums[m], move r = m - 1; else l = m + 1.
 *     - Else, the right half is sorted.
 *         - If nums[m] < target <= nums[r], move l = m + 1; else r = m - 1.
 *
 * Complexity:
 * - Time: O(log n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

int search(vector<int> nums, int target)
{
    int l = 0, r = (int)nums.size() - 1;
    while (l <= r)
    {
        int m = l + (r - l) / 2;
        if (nums[m] == target) return m;

        if (nums[l] <= nums[m])
        {
            // Left half [l..m] is sorted
            if (nums[l] <= target && target < nums[m])
                r = m - 1;
            else
                l = m + 1;
        }
        else
        {
            // Right half [m..r] is sorted
            if (nums[m] < target && target <= nums[r])
                l = m + 1;
            else
                r = m - 1;
        }
    }
    return -1;
}

int main()
{
    cout << search({4,5,6,7,0,1,2}, 0) << "\n";  // 4
    cout << search({4,5,6,7,0,1,2}, 3) << "\n";  // -1
    cout << search({1}, 0) << "\n";             // -1
    cout << search({1,3}, 3) << "\n";            // 1
    return 0;
}
