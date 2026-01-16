# 322. Coin Change

## Problem
Given coin denominations and an amount, return the minimum number of coins needed to make up the amount. If impossible, return -1.

## Examples
- coins=[1,2,5], amount=11 → 3 (5+5+1)
- coins=[2], amount=3 → -1

## Intuition
Unbounded knapsack DP: `dp[a]` is the fewest coins to make sum `a`.

## Algorithm (DP)
1. Initialize `dp[0]=0` and `dp[a]=INF` for `a>0`.
2. For each coin `c`, for `a` from `c` to `amount`, update `dp[a] = min(dp[a], dp[a-c] + 1)`.
3. Return `dp[amount]` or -1 if `INF`.

## Correctness
- Transition ensures minimal coins; each coin can be used multiple times.

## Complexity
- Time: O(n·amount)
- Space: O(amount)

## Edge Cases
- amount=0 → 0
- Non-canonical coin sets may require careful ordering; this bottom-up approach is robust.

## References
- Unbounded knapsack DP
