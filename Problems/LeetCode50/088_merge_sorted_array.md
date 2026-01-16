# 088. Merge Sorted Array

## Problem
Merge two sorted arrays `nums1` and `nums2` into `nums1` (in-place). `nums1` has length `m+n` with the last `n` slots reserved for `nums2`.

## Examples
- nums1 = [1,2,3,0,0,0], m = 3; nums2 = [2,5,6], n = 3 → [1,2,2,3,5,6]

## Intuition
Merge from the end so we only write into unused slots, avoiding overwriting data still needed in `nums1`.

## Algorithm (Reverse Merge)
1. Set pointers `i=m-1`, `j=n-1`, `k=m+n-1`.
2. While `j>=0`:
   - If `i>=0 && nums1[i] > nums2[j]`, write `nums1[i]` to `nums1[k]` and decrement `i`.
   - Else, write `nums2[j]` to `nums1[k]` and decrement `j`.
3. The remainder of `nums1` is already in place if `j<0`.

## Correctness
- At each step, the largest remaining element among tails is placed at position `k`, preserving sorted order.

## Complexity
- Time: O(m+n)
- Space: O(1)

## Edge Cases
- One array empty.
- All elements of one array smaller/larger than the other.

## References
- Two-pointer merging
