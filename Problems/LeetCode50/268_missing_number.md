# 268. Missing Number

## Problem
Given `nums` containing `n` distinct numbers from `0..n`, return the missing number.

## Examples
- [3,0,1] → 2
- [0,1] → 2
- [9,6,4,2,3,5,7,0,1] → 8

## Intuition
Use XOR: XOR all numbers from `0..n`, then XOR all elements of `nums`; duplicates cancel and the missing number remains. Alternatively, use sum formula: `n(n+1)/2 - sum(nums)`.

## Algorithm (XOR)
1. `x = XOR(0..n)`.
2. For each `v` in `nums`, `x ^= v`.
3. Return `x`.

## Complexity
- Time: O(n)
- Space: O(1)

## Edge Cases
- Missing `0` or `n` handled naturally.

## References
- XOR properties
