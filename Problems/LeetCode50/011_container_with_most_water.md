# 011. Container With Most Water

## Problem Statement (paraphrased, detailed)
You are given an array `heights` where `heights[i]` is the height of a vertical line at position `i`. Pick two lines at indices `l < r` to form a container with the x-axis. The container’s area is `min(heights[l], heights[r]) * (r - l)`. Return the maximum possible area.

### Examples
- `[1,8,6,2,5,4,8,3,7]` → `49` (lines at indices `1` and `8`)  
- `[1,1]` → `1`

## Intuition
Starting with the widest container (`l=0`, `r=n-1`), the area is limited by the shorter side. If we move the taller side, width decreases but the limiting height remains the same or better only if the new side is taller—however, moving the taller side cannot increase the minimum of the two heights. Therefore, we should move the shorter side to search for a potentially taller line that might offset the narrower width.

## Approach (Two Pointers)
1. Initialize `l = 0`, `r = n-1`, `best = 0`.
2. While `l < r`:
   - Compute `area = min(h[l], h[r]) * (r - l)` and update `best`.
   - If `h[l] < h[r]`, increment `l` (seek a taller left line);
     otherwise, decrement `r`.
3. Return `best`.

## Correctness (Sketch)
- At each step, moving the taller pointer cannot improve the current minimum height, while width shrinks. So only moving the shorter pointer can possibly increase the limiting height enough to compensate for lost width. Exhausting all such moves considers all optimal candidates.

## Complexity
- Time: `O(n)`
- Space: `O(1)`

## Edge Cases
- Very small arrays: `n < 2` → `0` by definition (no container).  
- Monotonic sequences: logic still holds, best often near extremes.

## Alternatives
- Brute force `O(n^2)`: check all pairs — too slow for large `n`.

## Related Concepts
- Two Pointers
- Greedy reasoning
