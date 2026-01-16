# 040. Combination Sum II

## Problem
Given `candidates` (which may contain duplicates) and a `target`, return unique combinations that sum to `target`. Each number may be used at most once.

## Examples
- candidates = [10,1,2,7,6,1,5], target = 8 → [[1,1,6],[1,2,5],[1,7],[2,6]]
- candidates = [2,5,2,1,2], target = 5 → [[1,2,2],[5]]

## Intuition
To avoid duplicate combinations, sort the array and, at each recursive level, skip equal-valued candidates beyond the first choice. Prevent reuse by advancing the index after picking.

## Algorithm (Backtracking with Dedup)
1. Sort `candidates`.
2. DFS over `i` from `start` to `n-1`:
   - If `i > start && candidates[i] == candidates[i-1]`, skip to avoid duplicate combinations at the same depth.
   - If `candidates[i] > target`, break (pruning).
   - Pick `candidates[i]`, recurse with `(i+1, target - candidates[i])` (move forward to prevent reuse), then backtrack.

## Correctness
- Sorting and skipping duplicates at the same depth ensure unique combinations.
- Moving to `i+1` after picking enforces “use at most once”.

## Complexity
- Time: Exponential in the number of valid combinations.
- Space: O(depth) recursion.

## Edge Cases
- Many duplicates in input; dedup logic prevents repeated result rows.
- Large single element equal to `target` should still appear.

## Alternatives
- Iterative enumeration with combination generation and a set for dedup (extra memory and overhead).

## References
- Backtracking
- Dedup patterns with sorted arrays
