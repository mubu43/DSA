# 062. Unique Paths

## Problem Statement (paraphrased, detailed)
In an `m x n` grid, start at the top-left `(0,0)` and move only right or down until you reach the bottom-right `(m-1,n-1)`. Return the number of distinct paths.

### Example
- `m=3, n=7` → `28`

## Intuition
Each path is a sequence of `(m-1)` moves down and `(n-1)` moves right, in some order. We can either count these with combinatorics or compute them with dynamic programming.

## Approach A: Dynamic Programming (1D rolling array)
1. Let `dp[c]` be the number of ways to reach the current row’s column `c`.
2. Initialize the first row as all ones (only rights).
3. For each subsequent row `r` and column `c>0`, update `dp[c] += dp[c-1]`.
4. Answer is `dp[n-1]`.

## Approach B: Combinatorics (optional)
- Total steps = `m+n-2`, choose where the `(m-1)` downs go: `C(m+n-2, m-1)`.
- Beware of overflow; use 64-bit or big integers for large `m, n`.

## Correctness
- DP recurrence captures the only two ways to arrive at a cell: from top or from left.
- Base conditions (first row and first column) are correctly initialized to 1.

## Complexity (DP)
- Time: `O(m*n)`
- Space: `O(n)`

## Related Concepts
- Dynamic Programming
- Pascal’s Triangle / Combinatorics
