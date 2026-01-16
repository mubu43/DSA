/**
 * 031. Next Permutation
 *
 * Problem (paraphrased):
 * Rearrange numbers into the lexicographically next greater permutation.
 * If not possible (already highest), rearrange to the lowest possible order.
 *
 * Intuition:
 * - The longest non-increasing suffix from the right is at its highest ordering.
 * - A pivot just before that suffix can be increased by swapping with the smallest greater element in the suffix.
 * - Reversing the suffix gives the minimal arrangement, yielding the next permutation.
 *
 * Algorithm:
 * - Scan from right to find pivot i where nums[i] < nums[i+1]. If not found, reverse all.
 * - Scan from right to find j where nums[j] > nums[i]. Swap.
 * - Reverse the suffix from i+1..end.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

void nextPermutation(vector<int>& nums) {
    int n = nums.size();
    int i = n - 2;
    // 1) Find pivot: first index from right where nums[i] < nums[i+1]
    while (i >= 0 && nums[i] >= nums[i+1]) i--;
    if (i >= 0) {
        // 2) Find the smallest element greater than nums[i] to the right
        int j = n - 1;
        while (nums[j] <= nums[i]) j--;
        swap(nums[i], nums[j]);
    }
    // 3) Reverse the suffix to obtain the minimal ordering
    reverse(nums.begin() + i + 1, nums.end());
}

int main() {
    auto print = [](const vector<int>& v){ for(int x:v) cout<<x<<' '; cout<<'\n'; };
    vector<int> a{1,2,3}; nextPermutation(a); print(a); // 1 3 2
    vector<int> b{3,2,1}; nextPermutation(b); print(b); // 1 2 3
    vector<int> c{1,1,5}; nextPermutation(c); print(c); // 1 5 1
    vector<int> d{1}; nextPermutation(d); print(d);     // 1
    return 0;
}
