/**
 * 015. 3Sum
 *
 * Problem (paraphrased, detailed):
 * Given an integer array nums, return all unique triplets [a, b, c] such that a + b + c = 0.
 * Triplets are unordered and should be unique (no duplicate triplets in the output).
 *
 * Examples:
 * - [-1,0,1,2,-1,-4] -> [[-1,-1,2], [-1,0,1]]
 * - [0,1,1] -> []
 * - [0,0,0] -> [[0,0,0]]
 *
 * Intuition:
 * - Sorting helps avoid duplicates and enables two-pointer sum search.
 * - For each index i, reduce to a two-sum on the subarray (i+1..n-1) targeting -nums[i].
 * - Skip duplicates at i and while moving pointers.
 *
 * Complexity:
 * - Time: O(n^2) after sorting.
 * - Space: O(1) extra (excluding output).
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<vector<int>> threeSum(vector<int> nums)
{
    vector<vector<int>> res;

    sort(nums.begin(), nums.end());
    int n = nums.size();

    for (int i = 0; i < n; i++)
    {
        // Skip repeated anchors to avoid duplicate triplets
        if (i > 0 && nums[i] == nums[i-1]) continue;

        int target = -nums[i];
        int l = i + 1, r = n - 1;

        while (l < r)
        {
            int sum = nums[l] + nums[r];

            if (sum == target)
            {
                res.push_back({nums[i], nums[l], nums[r]});
                // Move both pointers and skip duplicates to avoid repeated triplets
                l++; r--;
                while (l < r && nums[l] == nums[l-1]) l++;
                while (l < r && nums[r] == nums[r+1]) r--;
            }
            else if (sum < target)
            {
                l++;
            }
            else
            {
                r--;
            }
        }
    }

    return res;
}

int main()
{
    auto print = [](const vector<vector<int>>& v)
    {
        for (auto& t : v)
        {
            cout << "[" << t[0] << "," << t[1] << "," << t[2] << "] ";
        }
        cout << "\n";
    };

    // Basic tests
    print(threeSum({-1,0,1,2,-1,-4}));   // [[-1,-1,2],[-1,0,1]]
    print(threeSum({0,1,1}));            // []
    print(threeSum({0,0,0}));            // [[0,0,0]]
    return 0;
}
