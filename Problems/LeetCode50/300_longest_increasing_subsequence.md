# 300. Longest Increasing Subsequence

## Problem
Return the length of the longest strictly increasing subsequence (not necessarily contiguous).

## Examples
- [10,9,2,5,3,7,101,18] → 4
- [0,1,0,3,2,3] → 4
- [7,7,7,7,7] → 1

## Intuition
Maintain `tails` where `tails[i]` is the smallest possible tail value of any increasing subsequence of length `i+1`. For each number, replace the first tail that’s ≥ it (lower_bound), or append if it’s larger than all tails.

## Algorithm (Patience Sorting)
1. Initialize empty `tails`.
2. For each `x` in `nums`, find `i = lower_bound(tails, x)`:
   - If `i == tails.size()`, append `x`.
   - Else, set `tails[i] = x`.
3. Return `tails.size()`.

## Correctness
- `tails` maintains optimal tail values; its size equals LIS length.

## Complexity
- Time: O(n log n)
- Space: O(n)

## Edge Cases
- All equal values → LIS length 1.

## References
- Patience sorting technique
