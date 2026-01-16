# 084. Largest Rectangle in Histogram

## Problem
Given an array `heights` representing bar heights, compute the largest rectangle area in the histogram.

## Examples
- [2,1,5,6,2,3] → 10
- [2,4] → 4

## Intuition
Use a monotonic increasing stack of indices. When encountering a height smaller than the stack top, pop indices and compute areas using the popped height as the limiting bar and the current index as the right boundary.

## Algorithm (Monotonic Stack)
1. Iterate `i = 0..n` (with a sentinel height 0 at `i=n`).
2. While stack non-empty and `heights[i] < heights[stack.top()]`:
   - Pop `top` as height index, compute width from `stack.top()` (new top) to `i-1`.
   - Update `best = max(best, height * width)`.
3. Push `i` each iteration.

## Correctness
- Each bar is popped exactly once, computing maximal area where that bar is the minimum height.

## Complexity
- Time: O(n)
- Space: O(n)

## Edge Cases
- All equal heights.
- Strictly increasing or decreasing sequences.

## References
- Monotonic stacks
