/**
 * 283. Move Zeroes
 *
 * Problem:
 * Move all zeros to the end while maintaining relative order of non-zero elements. Do it in-place.
 *
 * Intuition:
 * Two-pointer write: compact non-zeros to the front, then fill the rest with zeros.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

void moveZeroes(vector<int>& nums) {
    int write = 0;
    for (int x : nums) if (x != 0) nums[write++] = x;
    while (write < (int)nums.size()) nums[write++] = 0;
}

int main() {
    vector<int> a{0,1,0,3,12};
    moveZeroes(a);
    for (int x : a) cout << x << ' '; cout << '\n'; // 1 3 12 0 0
    return 0;
}
