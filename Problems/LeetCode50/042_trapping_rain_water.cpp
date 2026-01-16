/**
 * 042. Trapping Rain Water
 *
 * Problem:
 * Given non-negative integers representing an elevation map, compute how much water it can trap after raining.
 *
 * Intuition:
 * For any index i, trapped water is min(max_left, max_right) - height[i]. Two-pointer technique computes this on the fly with O(1) space by
 * moving the side with the smaller height.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

int trap(const vector<int>& h) {
    int n = h.size();
    int l = 0, r = n - 1;
    int leftMax = 0, rightMax = 0, water = 0;
    while (l < r) {
        if (h[l] < h[r]) {
            // left side bounded by leftMax
            if (h[l] >= leftMax) leftMax = h[l];
            else water += leftMax - h[l];
            l++;
        } else {
            // right side bounded by rightMax
            if (h[r] >= rightMax) rightMax = h[r];
            else water += rightMax - h[r];
            r--;
        }
    }
    return water;
}

int main() {
    cout << trap({0,1,0,2,1,0,1,3,2,1,2,1}) << "\n"; // 6
    cout << trap({4,2,0,3,2,5}) << "\n";             // 9
    cout << trap({}) << "\n";                        // 0
    return 0;
}
