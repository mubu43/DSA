# 283. Move Zeroes

## Problem
Move all zeros to the end of the array while maintaining the relative order of non-zero elements (in-place).

## Examples
- [0,1,0,3,12] → [1,3,12,0,0]

## Intuition
Write non-zero elements in order to the front and fill remaining positions with zeros.

## Algorithm (Two-pointer write)
1. `write=0`.
2. For each `x` in `nums`, if `x!=0`, set `nums[write++]=x`.
3. Fill `nums[write..end]` with zeros.

## Correctness
- Stability is preserved since non-zero elements are written in original order.

## Complexity
- Time: O(n)
- Space: O(1)

## References
- Stable compaction patterns
