/*
 * LeetCode #1: Two Sum
 * Difficulty: Easy
 * 
 * Problem: Given an array of integers nums and an integer target, 
 * return indices of the two numbers such that they add up to target.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 */

#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;

class Solution {
public:
    vector<int> twoSum(vector<int>& nums, int target) {
        // Hash map to store value -> index mapping
        unordered_map<int, int> numMap;
        
        // Iterate through the array
        for (int i = 0; i < nums.size(); i++) {
            int complement = target - nums[i];
            
            // Check if complement exists in hash map
            if (numMap.find(complement) != numMap.end()) {
                // Found the pair, return indices
                return {numMap[complement], i};
            }
            
            // Store current number and its index
            numMap[nums[i]] = i;
        }
        
        // No solution found (shouldn't happen according to problem statement)
        return {};
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test case 1: [2,7,11,15], target = 9
    vector<int> nums1 = {2, 7, 11, 15};
    int target1 = 9;
    vector<int> result1 = solution.twoSum(nums1, target1);
    cout << "Test 1: [" << result1[0] << ", " << result1[1] << "]" << endl;
    
    // Test case 2: [3,2,4], target = 6
    vector<int> nums2 = {3, 2, 4};
    int target2 = 6;
    vector<int> result2 = solution.twoSum(nums2, target2);
    cout << "Test 2: [" << result2[0] << ", " << result2[1] << "]" << endl;
    
    // Test case 3: [3,3], target = 6
    vector<int> nums3 = {3, 3};
    int target3 = 6;
    vector<int> result3 = solution.twoSum(nums3, target3);
    cout << "Test 3: [" << result3[0] << ", " << result3[1] << "]" << endl;
    
    return 0;
}
