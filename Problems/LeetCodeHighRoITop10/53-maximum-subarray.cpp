/*
 * LeetCode #53: Maximum Subarray
 * Difficulty: Medium
 * 
 * Problem: Given an integer array nums, find the contiguous subarray 
 * (containing at least one number) which has the largest sum and return its sum.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1)
 */

#include <vector>
#include <algorithm>
#include <iostream>
#include <climits>

using namespace std;

class Solution {
public:
    // Kadane's Algorithm - Dynamic Programming Approach
    int maxSubArray(vector<int>& nums) {
        // Initialize with first element
        int maxSoFar = nums[0];      // Global maximum
        int maxEndingHere = nums[0]; // Maximum ending at current position
        
        // Process remaining elements
        for (int i = 1; i < nums.size(); i++) {
            // Either extend previous subarray or start new one
            maxEndingHere = max(nums[i], maxEndingHere + nums[i]);
            
            // Update global maximum
            maxSoFar = max(maxSoFar, maxEndingHere);
        }
        
        return maxSoFar;
    }
    
    // Alternative: Divide and Conquer Approach
    int maxSubArrayDivideConquer(vector<int>& nums) {
        return maxSubArrayHelper(nums, 0, nums.size() - 1);
    }
    
private:
    int maxSubArrayHelper(vector<int>& nums, int left, int right) {
        // Base case: single element
        if (left == right) {
            return nums[left];
        }
        
        int mid = left + (right - left) / 2;
        
        // Maximum subarray in left half
        int leftMax = maxSubArrayHelper(nums, left, mid);
        
        // Maximum subarray in right half
        int rightMax = maxSubArrayHelper(nums, mid + 1, right);
        
        // Maximum subarray crossing the middle
        int crossMax = maxCrossingSum(nums, left, mid, right);
        
        // Return maximum of all three
        return max({leftMax, rightMax, crossMax});
    }
    
    int maxCrossingSum(vector<int>& nums, int left, int mid, int right) {
        // Maximum sum ending at mid (going left)
        int leftSum = INT_MIN;
        int sum = 0;
        for (int i = mid; i >= left; i--) {
            sum += nums[i];
            leftSum = max(leftSum, sum);
        }
        
        // Maximum sum starting at mid+1 (going right)
        int rightSum = INT_MIN;
        sum = 0;
        for (int i = mid + 1; i <= right; i++) {
            sum += nums[i];
            rightSum = max(rightSum, sum);
        }
        
        return leftSum + rightSum;
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<vector<int>> testCases = {
        {-2, 1, -3, 4, -1, 2, 1, -5, 4},  // Expected: 6 ([4,-1,2,1])
        {1},                               // Expected: 1
        {5, 4, -1, 7, 8},                // Expected: 23
        {-2, -1, -3, -4},                // Expected: -1
        {-1, -2},                         // Expected: -1
        {2, -3, 4, -1, 2, 1}             // Expected: 6
    };
    
    vector<int> expected = {6, 1, 23, -1, -1, 6};
    
    cout << "Testing Maximum Subarray (Kadane's Algorithm):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        int result = solution.maxSubArray(testCases[i]);
        cout << "Test " << (i + 1) << ": ";
        cout << "Result = " << result << ", Expected = " << expected[i];
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Maximum Subarray (Divide & Conquer):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        int result = solution.maxSubArrayDivideConquer(testCases[i]);
        cout << "Test " << (i + 1) << ": ";
        cout << "Result = " << result << ", Expected = " << expected[i];
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    return 0;
}
