# 042. Trapping Rain Water

## Problem
Given `height` array representing elevation map, compute how much water can be trapped after raining.

## Examples
- `[0,1,0,2,1,0,1,3,2,1,2,1]` → 6
- `[4,2,0,3,2,5]` → 9

## Intuition
Water above a bar depends on the max height to its left and right: `water[i] = max(0, min(leftMax[i], rightMax[i]) - height[i])`. Two pointers can compute this on the fly with O(1) space by moving the side with the smaller current height.

## Approach
- Two pointers with running `leftMax` and `rightMax`.
- Always move the side with the smaller current height, accumulating water relative to its max.

## Algorithm (Two Pointers)
1. Initialize `l=0`, `r=n-1`, `leftMax=0`, `rightMax=0`, `water=0`.
2. While `l < r`:
   - If `height[l] < height[r]`:
     - If `height[l] >= leftMax`, update `leftMax`.
     - Else, add `leftMax - height[l]` to `water`.
     - Move `l++`.
   - Else:
     - If `height[r] >= rightMax`, update `rightMax`.
     - Else, add `rightMax - height[r]` to `water`.
     - Move `r--`.
3. Return `water`.

## Correctness
- When `height[l] < height[r]`, the trapped water at `l` is bounded by `leftMax`, since `rightMax` is at least `height[r]`, hence `min(leftMax, rightMax) = leftMax`.
- Symmetric reasoning applies when moving `r`.

## Complexity
- Time: O(n)
- Space: O(1)

## Edge Cases
- Monotonic heights → 0 water.
- Single bar or empty array → 0 water.

## Alternatives
- Precompute arrays `leftMax[i]` and `rightMax[i]` and sum: O(n) time, O(n) space.

## References
- Two-pointer technique
- Boundary reasoning
