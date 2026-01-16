/**
 * 300. Longest Increasing Subsequence
 *
 * Problem:
 * Given an integer array, return the length of the longest strictly increasing subsequence.
 *
 * Intuition:
 * Patience sorting: maintain tails[], where tails[len] is the minimal tail of an increasing subsequence of length len+1.
 * Use binary search to place each number.
 *
 * Complexity:
 * - Time: O(n log n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int lengthOfLIS(const vector<int>& nums) {
    vector<int> tails;
    for (int x : nums) {
        auto it = lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) tails.push_back(x);
        else *it = x;
    }
    return (int)tails.size();
}

int main() {
    cout << lengthOfLIS({10,9,2,5,3,7,101,18}) << "\n"; // 4
    cout << lengthOfLIS({0,1,0,3,2,3}) << "\n";        // 4
    cout << lengthOfLIS({7,7,7,7,7}) << "\n";          // 1
    return 0;
}
