# 031. Next Permutation

## Problem
Rearrange numbers into the lexicographically next greater permutation. If no such permutation exists (already highest), rearrange into the lowest possible order (sorted ascending).

## Examples
- [1,2,3] → [1,3,2]
- [3,2,1] → [1,2,3]
- [1,1,5] → [1,5,1]

## Intuition
Find the longest non-increasing suffix; it's already the highest ordering. The pivot just before this suffix can be increased by swapping with the smallest greater element within the suffix, then sorting (reversing) the suffix gives the next lexicographic arrangement.

## Algorithm
1. Scan from right to find the first index `i` with `nums[i] < nums[i+1]` (the pivot). If none, reverse the entire array and return.
2. From the right, find the smallest element greater than `nums[i]` (call index `j`) and swap `nums[i]` and `nums[j]`.
3. Reverse the suffix starting at `i+1` to get the minimal arrangement of that suffix.

## Correctness
- The pivot ensures we can form a strictly larger permutation.
- Swapping with the smallest greater element keeps the increase minimal.
- Reversing the suffix yields the minimal ordering for that suffix, ensuring the next lexicographic permutation.

## Complexity
- Time: O(n) (two linear scans plus a reverse)
- Space: O(1)

## Edge Cases
- Entire array non-increasing (e.g., [3,2,1]) → reverse whole array.
- Duplicates → logic still holds; comparisons handle ties correctly.
- Single element → unchanged.

## Alternatives
- Generate permutations until next appears (inefficient).
- Use STL `next_permutation` (C++) which implements this algorithm.

## References
- Lexicographic order
- Array manipulation
