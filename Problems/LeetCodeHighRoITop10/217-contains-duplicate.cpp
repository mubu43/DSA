/*
 * LeetCode #217: Contains Duplicate
 * Difficulty: Easy
 * 
 * Problem: Given an integer array nums, return true if any value appears 
 * at least twice in the array, and return false if every element is distinct.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 */

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <iostream>

using namespace std;

class Solution {
public:
    // Approach 1: Hash Set (Optimal)
    bool containsDuplicate(vector<int>& nums) {
        unordered_set<int> seen;
        
        for (int num : nums) {
            // If number is already in set, we found a duplicate
            if (seen.find(num) != seen.end()) {
                return true;
            }
            // Add number to set
            seen.insert(num);
        }
        
        // No duplicates found
        return false;
    }
    
    // Approach 2: Hash Set with insert return value
    bool containsDuplicateInsert(vector<int>& nums) {
        unordered_set<int> seen;
        
        for (int num : nums) {
            // insert().second returns false if element already exists
            if (!seen.insert(num).second) {
                return true;
            }
        }
        
        return false;
    }
    
    // Approach 3: Sorting (Less space, more time)
    bool containsDuplicateSorting(vector<int>& nums) {
        if (nums.size() <= 1) {
            return false;
        }
        
        // Sort the array
        sort(nums.begin(), nums.end());
        
        // Check adjacent elements
        for (int i = 1; i < nums.size(); i++) {
            if (nums[i] == nums[i - 1]) {
                return true;
            }
        }
        
        return false;
    }
    
    // Approach 4: Brute Force (Not recommended)
    bool containsDuplicateBruteForce(vector<int>& nums) {
        for (int i = 0; i < nums.size(); i++) {
            for (int j = i + 1; j < nums.size(); j++) {
                if (nums[i] == nums[j]) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Approach 5: Using set size comparison
    bool containsDuplicateSetSize(vector<int>& nums) {
        unordered_set<int> uniqueNums(nums.begin(), nums.end());
        
        // If set size is less than array size, there were duplicates
        return uniqueNums.size() < nums.size();
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<vector<int>> testCases = {
        {1, 2, 3, 1},           // true - 1 appears twice
        {1, 2, 3, 4},           // false - all distinct
        {1, 1, 1, 3, 3, 4, 3, 2, 4, 2}, // true - multiple duplicates
        {},                     // false - empty array
        {1},                    // false - single element
        {2, 14, 18, 22, 22},    // true - 22 appears twice
        {-1, -1},               // true - negative duplicates
        {0, 0},                 // true - zero duplicates
        {1000000, 1000000}      // true - large number duplicates
    };
    
    vector<bool> expected = {true, false, true, false, false, true, true, true, true};
    
    cout << "Testing Contains Duplicate (Hash Set):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        vector<int> nums = testCases[i]; // Copy to avoid modification
        bool result = solution.containsDuplicate(nums);
        
        cout << "Test " << (i + 1) << ": [";
        for (int j = 0; j < nums.size(); j++) {
            cout << nums[j];
            if (j < nums.size() - 1) cout << ", ";
        }
        cout << "] -> " << (result ? "true" : "false");
        cout << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Contains Duplicate (Insert Method):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        vector<int> nums = testCases[i];
        bool result = solution.containsDuplicateInsert(nums);
        cout << "Test " << (i + 1) << ": " << (result ? "true" : "false");
        cout << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Contains Duplicate (Sorting):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        vector<int> nums = testCases[i]; // Copy since sorting modifies array
        bool result = solution.containsDuplicateSorting(nums);
        cout << "Test " << (i + 1) << ": " << (result ? "true" : "false");
        cout << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Contains Duplicate (Set Size):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        vector<int> nums = testCases[i];
        bool result = solution.containsDuplicateSetSize(nums);
        cout << "Test " << (i + 1) << ": " << (result ? "true" : "false");
        cout << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    // Performance test
    cout << "\nPerformance Test:" << endl;
    vector<int> largeArray;
    for (int i = 0; i < 10000; i++) {
        largeArray.push_back(i);
    }
    largeArray.push_back(5000); // Add one duplicate
    
    bool perfResult = solution.containsDuplicate(largeArray);
    cout << "Large array with one duplicate: " << (perfResult ? "true" : "false") << endl;
    
    return 0;
}
