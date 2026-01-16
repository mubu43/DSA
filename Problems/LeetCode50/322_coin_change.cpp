/**
 * 322. Coin Change
 *
 * Problem:
 * Given coins and amount, return fewest coins needed to make up the amount; return -1 if not possible.
 *
 * Intuition:
 * Classic DP: unbounded knapsack minimizing coin counts.
 *
 * Complexity:
 * - Time: O(n*amount) where n=coins.size()
 * - Space: O(amount)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int coinChange(const vector<int>& coins, int amount) {
    const int INF = amount + 1;
    vector<int> dp(amount + 1, INF);
    dp[0] = 0;
    for (int c : coins) {
        for (int a = c; a <= amount; a++) {
            dp[a] = min(dp[a], dp[a - c] + 1);
        }
    }
    return dp[amount] > amount ? -1 : dp[amount];
}

int main() {
    cout << coinChange({1,2,5}, 11) << "\n"; // 3 (5+5+1)
    cout << coinChange({2}, 3) << "\n";       // -1
    cout << coinChange({1}, 0) << "\n";       // 0
    return 0;
}
