/*
 * LeetCode #121: Best Time to Buy and Sell Stock
 * Difficulty: Easy
 * 
 * Problem: You are given an array prices where prices[i] is the price of a given stock on the ith day.
 * You want to maximize your profit by choosing a single day to buy one stock and choosing 
 * a different day in the future to sell that stock.
 * Return the maximum profit you can achieve from this transaction. If you cannot achieve any profit, return 0.
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
    // Approach 1: Single Pass - Track minimum price
    int maxProfit(vector<int>& prices) {
        if (prices.empty()) {
            return 0;
        }
        
        int minPrice = prices[0];   // Minimum price seen so far
        int maxProfit = 0;         // Maximum profit achievable
        
        for (int i = 1; i < prices.size(); i++) {
            // Update minimum price if current price is lower
            minPrice = min(minPrice, prices[i]);
            
            // Calculate profit if we sell at current price
            int currentProfit = prices[i] - minPrice;
            
            // Update maximum profit
            maxProfit = max(maxProfit, currentProfit);
        }
        
        return maxProfit;
    }
    
    // Approach 2: Kadane's Algorithm variant (Maximum Subarray)
    int maxProfitKadane(vector<int>& prices) {
        if (prices.size() < 2) {
            return 0;
        }
        
        int maxProfit = 0;
        int maxEndingHere = 0;
        
        // Calculate profit differences between consecutive days
        for (int i = 1; i < prices.size(); i++) {
            int dailyProfit = prices[i] - prices[i - 1];
            
            // Either extend previous profit or start new
            maxEndingHere = max(0, maxEndingHere + dailyProfit);
            
            // Update global maximum
            maxProfit = max(maxProfit, maxEndingHere);
        }
        
        return maxProfit;
    }
    
    // Approach 3: Two Pointer technique (less efficient but educational)
    int maxProfitTwoPointer(vector<int>& prices) {
        if (prices.size() < 2) {
            return 0;
        }
        
        int maxProfit = 0;
        
        for (int buy = 0; buy < prices.size() - 1; buy++) {
            for (int sell = buy + 1; sell < prices.size(); sell++) {
                int profit = prices[sell] - prices[buy];
                maxProfit = max(maxProfit, profit);
            }
        }
        
        return maxProfit;
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<vector<int>> testCases = {
        {7, 1, 5, 3, 6, 4},    // Expected: 5 (buy at 1, sell at 6)
        {7, 6, 4, 3, 1},       // Expected: 0 (prices only decrease)
        {1, 2, 3, 4, 5},       // Expected: 4 (buy at 1, sell at 5)
        {2, 4, 1},             // Expected: 2 (buy at 2, sell at 4)
        {3, 2, 6, 5, 0, 3},    // Expected: 4 (buy at 2, sell at 6)
        {1},                   // Expected: 0 (single day)
        {}                     // Expected: 0 (empty array)
    };
    
    vector<int> expected = {5, 0, 4, 2, 4, 0, 0};
    
    cout << "Testing Best Time to Buy and Sell Stock:" << endl;
    
    for (int i = 0; i < testCases.size(); i++) {
        int result1 = solution.maxProfit(testCases[i]);
        int result2 = solution.maxProfitKadane(testCases[i]);
        
        cout << "Test " << (i + 1) << ": ";
        cout << "Prices = [";
        for (int j = 0; j < testCases[i].size(); j++) {
            cout << testCases[i][j];
            if (j < testCases[i].size() - 1) cout << ", ";
        }
        cout << "]" << endl;
        
        cout << "  Single Pass: " << result1;
        cout << ", Kadane's: " << result2;
        cout << ", Expected: " << expected[i];
        cout << (result1 == expected[i] && result2 == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    // Performance comparison for large input
    cout << "\nPerformance Test (O(n²) vs O(n)):" << endl;
    vector<int> largePrices;
    for (int i = 1000; i >= 1; i--) {
        largePrices.push_back(i);
    }
    largePrices.push_back(2000); // Maximum profit = 1999
    
    int efficientResult = solution.maxProfit(largePrices);
    cout << "Efficient O(n) result: " << efficientResult << endl;
    
    return 0;
}
