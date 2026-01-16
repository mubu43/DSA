# 033. Search in Rotated Sorted Array

## Problem Statement (paraphrased, detailed)
Given a rotated sorted array of unique integers `nums` and an integer `target`, return the index of `target` if it exists, otherwise return `-1`.

A rotated sorted array is produced by taking an increasing array and rotating it at some pivot, e.g., `[0,1,2,4,5,6,7]` → `[4,5,6,7,0,1,2]`.

### Examples
- `nums=[4,5,6,7,0,1,2], target=0` → `4`
- `nums=[4,5,6,7,0,1,2], target=3` → `-1`
- `nums=[1], target=0` → `-1`

## Intuition
In a rotated array, at least one half of any subarray is still sorted. During binary search, we detect which half is sorted and decide if the target lies in it.

## Approach (Modified Binary Search)
1. Initialize `l=0`, `r=n-1`.
2. While `l <= r`:
   - `m = (l + r) / 2`. If `nums[m] == target`, return `m`.
   - If `nums[l] <= nums[m]`, left half `[l..m]` is sorted:
     - If `nums[l] <= target < nums[m]`, set `r = m - 1`; else `l = m + 1`.
   - Else, right half `[m..r]` is sorted:
     - If `nums[m] < target <= nums[r]`, set `l = m + 1`; else `r = m - 1`.

## Correctness (Sketch)
- At each iteration, one half is sorted. If the target falls within the sorted half’s numeric range, restricting to that half cannot lose the target. Otherwise, the target (if present) must lie in the other half. Binary search convergence ensures logarithmic steps.

## Complexity
- Time: `O(log n)`
- Space: `O(1)`

## Edge Cases
- Single-element arrays
- Target not present
- Already sorted (no rotation) — reduces to standard binary search logic

## Related Concepts
- Binary Search
- Array Partitioning
