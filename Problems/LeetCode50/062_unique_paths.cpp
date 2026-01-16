/**
 * 062. Unique Paths
 *
 * Problem (paraphrased, detailed):
 * In an m x n grid, you start at the top-left cell and want to reach the bottom-right cell by
 * moving only right or down. Count the number of distinct paths.
 *
 * Example:
 * - m=3, n=7 -> 28
 *
 * Approaches:
 * - Combinatorics: choose (m-1) downs (or (n-1) rights) among (m+n-2) steps: C(m+n-2, m-1).
 * - DP: dp[r][c] = dp[r-1][c] + dp[r][c-1]. Use 1D rolling array for O(n) space.
 *
 * Complexity (DP):
 * - Time: O(m*n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
using namespace std;

int uniquePaths(int m, int n)
{
    // dp[c] is number of ways to reach current row at column c
    vector<int> dp(n, 1); // first row: only one way (all rights)

    for (int r = 1; r < m; r++)
    {
        for (int c = 1; c < n; c++)
        {
            // Ways to reach (r,c) = from top (dp[c]) + from left (dp[c-1])
            dp[c] += dp[c-1];
        }
    }
    return dp[n-1];
}

int main()
{
    cout << uniquePaths(3, 7) << "\n"; // 28
    cout << uniquePaths(3, 2) << "\n"; // 3
    cout << uniquePaths(1, 1) << "\n"; // 1
    cout << uniquePaths(10, 10) << "\n"; // large
    return 0;
}
