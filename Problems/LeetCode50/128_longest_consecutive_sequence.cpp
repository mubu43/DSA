/**
 * 128. Longest Consecutive Sequence
 *
 * Problem:
 * Given an unsorted array of integers, find the length of the longest consecutive sequence. Must be O(n).
 *
 * Intuition:
 * Use a hash set. Start sequences at numbers with no predecessor (x-1 not present), then count forward.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
using namespace std;

int longestConsecutive(const vector<int>& nums) {
    unordered_set<int> s(nums.begin(), nums.end());
    int best = 0;
    for (int x : s) {
        if (!s.count(x - 1)) { // start of a sequence
            int y = x; int len = 1;
            while (s.count(y + 1)) { y++; len++; }
            best = max(best, len);
        }
    }
    return best;
}

int main() {
    cout << longestConsecutive({100,4,200,1,3,2}) << "\n"; // 4 (1,2,3,4)
    cout << longestConsecutive({0,3,7,2,5,8,4,6,0,1}) << "\n"; // 9 (0..8)
    return 0;
}
