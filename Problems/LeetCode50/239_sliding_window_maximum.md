# 239. Sliding Window Maximum

## Problem
Given `nums` and window size `k`, return maximum for each sliding window.

## Examples
- nums = [1,3,-1,-3,5,3,6,7], k=3 → [3,3,5,5,6,7]

## Intuition
Use a deque storing indices of elements in decreasing order. The front is the maximum for the current window.

## Algorithm (Monotonic Deque)
1. For each index `i`:
   - Pop front if it’s outside the window (`<= i-k`).
   - Pop back while `nums[back] <= nums[i]`.
   - Push `i`.
   - If `i >= k-1`, append `nums[dq.front()]` to result.

## Correctness
- Deque invariant keeps potential maxima; front always valid and maximal.

## Complexity
- Time: O(n)
- Space: O(k)

## References
- Monotonic queue/deque
