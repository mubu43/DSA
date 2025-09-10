/*
 * LeetCode #70: Climbing Stairs
 * Difficulty: Easy
 * 
 * Problem: You are climbing a staircase. It takes n steps to reach the top.
 * Each time you can either climb 1 or 2 steps. In how many distinct ways 
 * can you climb to the top?
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1) for optimized version
 */

#include <vector>
#include <iostream>

using namespace std;

class Solution {
public:
    // Approach 1: Dynamic Programming (Bottom-up)
    int climbStairs(int n) {
        // Base cases
        if (n <= 2) {
            return n;
        }
        
        // DP array where dp[i] = number of ways to reach step i
        vector<int> dp(n + 1);
        dp[1] = 1;  // 1 way to reach step 1
        dp[2] = 2;  // 2 ways to reach step 2
        
        // Fill dp array
        for (int i = 3; i <= n; i++) {
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        
        return dp[n];
    }
    
    // Approach 2: Space-Optimized DP (Fibonacci pattern)
    int climbStairsOptimized(int n) {
        // Base cases
        if (n <= 2) {
            return n;
        }
        
        // Only need to track last two values
        int prev2 = 1;  // ways to reach step (i-2)
        int prev1 = 2;  // ways to reach step (i-1)
        int current;
        
        // Calculate for steps 3 to n
        for (int i = 3; i <= n; i++) {
            current = prev1 + prev2;
            prev2 = prev1;
            prev1 = current;
        }
        
        return prev1;
    }
    
    // Approach 3: Recursive with Memoization (Top-down DP)
    int climbStairsRecursive(int n) {
        vector<int> memo(n + 1, -1);
        return climbStairsHelper(n, memo);
    }
    
private:
    int climbStairsHelper(int n, vector<int>& memo) {
        // Base cases
        if (n <= 2) {
            return n;
        }
        
        // Check if already computed
        if (memo[n] != -1) {
            return memo[n];
        }
        
        // Compute and store result
        memo[n] = climbStairsHelper(n - 1, memo) + climbStairsHelper(n - 2, memo);
        return memo[n];
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<int> testCases = {1, 2, 3, 4, 5, 6, 10, 15};
    vector<int> expected = {1, 2, 3, 5, 8, 13, 89, 987};
    
    cout << "Testing Climbing Stairs (DP Bottom-up):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        int result = solution.climbStairs(testCases[i]);
        cout << "n = " << testCases[i] << ": ";
        cout << "Result = " << result << ", Expected = " << expected[i];
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Climbing Stairs (Space Optimized):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        int result = solution.climbStairsOptimized(testCases[i]);
        cout << "n = " << testCases[i] << ": ";
        cout << "Result = " << result << ", Expected = " << expected[i];
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Climbing Stairs (Recursive + Memoization):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        int result = solution.climbStairsRecursive(testCases[i]);
        cout << "n = " << testCases[i] << ": ";
        cout << "Result = " << result << ", Expected = " << expected[i];
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    // Show the Fibonacci pattern
    cout << "\nFibonacci Pattern:" << endl;
    cout << "Steps: 1, 2, 3, 4, 5,  6,  7,  8,  9, 10" << endl;
    cout << "Ways:  1, 2, 3, 5, 8, 13, 21, 34, 55, 89" << endl;
    
    return 0;
}
