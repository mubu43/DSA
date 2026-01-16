# 238. Product of Array Except Self

## Problem
Given `nums`, return `ans[i] = product(nums[j] for j!=i)` without using division.

## Examples
- [1,2,3,4] → [24,12,8,6]
- [-1,1,0,-3,3] → [0,0,9,0,0]

## Intuition
Compute prefix products and suffix products; multiply them to get the product of all elements except the current index.

## Algorithm
1. `ans[i]` <- product of elements to the left of `i`.
2. Traverse from right with `suff` and multiply: `ans[i] *= suff`.

## Correctness
- Left and right products exclude `nums[i]` naturally.

## Complexity
- Time: O(n)
- Space: O(1) extra (excluding output)

## Edge Cases
- Zeros in the array handled naturally by prefix/suffix multiplication.

## References
- Prefix/suffix product technique
