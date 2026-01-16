/**
 * 268. Missing Number
 *
 * Problem:
 * Given an array containing n distinct numbers taken from 0..n, find the one that is missing.
 *
 * Intuition:
 * XOR from 0..n then XOR all nums; pairs cancel leaving the missing one. Alternatively, use sum formula.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

int missingNumber(const vector<int>& nums) {
    int n = nums.size();
    int x = 0;
    for (int i = 0; i <= n; i++) x ^= i;
    for (int v : nums) x ^= v;
    return x;
}

int main() {
    cout << missingNumber({3,0,1}) << "\n"; // 2
    cout << missingNumber({0,1}) << "\n";   // 2
    cout << missingNumber({9,6,4,2,3,5,7,0,1}) << "\n"; // 8
    return 0;
}
