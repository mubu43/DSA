# 046. Permutations

## Problem
Given a list of distinct integers, return all possible permutations.

## Examples
- [1,2,3] → 6 permutations
- [0,1] → [[0,1],[1,0]]

## Intuition
Backtracking explores all arrangements. In-place swap-based generation is efficient and avoids the need for additional `used[]` tracking for distinct inputs.

## Algorithm (Swap-based Backtracking)
1. Function `backtrack(nums, start)`:
   - If `start == n`, append `nums` to results.
   - For `i` from `start` to `n-1`:
     - Swap `nums[start]` and `nums[i]`.
     - Recurse with `start+1`.
     - Swap back to restore state.

## Correctness
- At depth `start`, choosing which element to place there enumerates all possible assignments across branches.
- Swapping back maintains correctness for subsequent branches.

## Complexity
- Time: O(n · n!) to generate all permutations.
- Space: O(n) recursion depth (excluding output storage).

## Edge Cases
- Single element → one permutation.
- Empty input → one empty permutation (depending on definition; typically return `[[]]` or `[]`).

## Alternatives
- Backtracking with `used[]` array and building current path incrementally.

## References
- Backtracking patterns
- Permutation generation
