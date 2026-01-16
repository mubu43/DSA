/**
 * 238. Product of Array Except Self
 *
 * Problem:
 * Return an array `ans` where `ans[i]` is the product of all elements of `nums` except `nums[i]`, without using division.
 *
 * Intuition:
 * Prefix products from left and suffix products from right; multiply for each index.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1) extra (output excluded)
 */

#include <iostream>
#include <vector>
using namespace std;

vector<int> productExceptSelf(const vector<int>& nums) {
    int n = nums.size();
    vector<int> ans(n, 1);
    int pref = 1;
    for (int i = 0; i < n; i++) { ans[i] = pref; pref *= nums[i]; }
    int suff = 1;
    for (int i = n - 1; i >= 0; i--) { ans[i] *= suff; suff *= nums[i]; }
    return ans;
}

int main() {
    auto print = [](const vector<int>& v){ for(int x:v) cout<<x<<' '; cout<<'\n'; };
    print(productExceptSelf({1,2,3,4})); // 24 12 8 6
    print(productExceptSelf({-1,1,0,-3,3})); // 0 0 9 0 0
    return 0;
}
