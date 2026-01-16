# 039. Combination Sum

## Problem
Given distinct `candidates` and a `target`, return all unique combinations where the sum is `target`. You may use each number multiple times.

## Examples
- candidates = [2,3,6,7], target = 7 → [[2,2,3],[7]]
- candidates = [2,3,5], target = 8 → [[2,2,2,2],[2,3,3],[3,5]]

## Intuition
Backtracking explores combinations; sorting helps prune branches when the cumulative sum exceeds the target. Allow reuse by staying at the same index when including a candidate.

## Algorithm (Backtracking with Reuse)
1. Sort `candidates`.
2. Define DFS `(idx, target, cur)`:
   - If `target == 0`, push `cur` to results.
   - If `idx == n` or `target < 0`, return.
   - Include `candidates[idx]` and recurse with `(idx, target - candidates[idx])` to allow reuse; then backtrack.
   - Exclude `candidates[idx]` and recurse with `(idx+1, target)`.

## Correctness
- Sorting enables pruning when `candidates[idx] > target`.
- Reuse is allowed by not advancing the index when including the current candidate.
- Backtracking with two choices (take/skip) enumerates all valid combinations without duplicates due to distinct input.

## Complexity
- Time: Exponential in the number of combinations (output-sensitive).
- Space: O(depth), where depth ≤ `target / min(candidates)` for typical pruning.

## Edge Cases
- `target` smaller than all candidates → no solutions.
- Large `target` with small candidates can create many combinations.

## Alternatives
- Iterative DP (unbounded knapsack style) to count or list combinations (requires careful dedup).

## References
- Backtracking
- Pruning with sorted inputs
