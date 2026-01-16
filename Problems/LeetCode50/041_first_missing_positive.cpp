/**
 * 041. First Missing Positive
 *
 * Problem:
 * Given an unsorted integer array, return the smallest missing positive integer.
 *
 * Intuition:
 * Place each value v in [1..n] to index v-1 using cyclic swaps. Then scan for the first mismatch.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
#include <algorithm> // for std::swap
using namespace std;

int firstMissingPositive(vector<int>& nums) {
    int n = nums.size();
    for (int i = 0; i < n; i++) {
        // place nums[i] to its correct position if possible
        while (nums[i] >= 1 && nums[i] <= n && nums[nums[i]-1] != nums[i]) {
            int idx = nums[i] - 1;
            swap(nums[i], nums[idx]);
        }
    }
    // the first index where nums[i] != i+1 is the answer
    for (int i = 0; i < n; i++) {
        if (nums[i] != i + 1) return i + 1;
    }
    return n + 1;
}

int main() {
    vector<int> a{1,2,0}; cout << firstMissingPositive(a) << "\n"; // 3
    vector<int> b{3,4,-1,1}; cout << firstMissingPositive(b) << "\n"; // 2
    vector<int> c{7,8,9,11,12}; cout << firstMissingPositive(c) << "\n"; // 1
    return 0;
}
