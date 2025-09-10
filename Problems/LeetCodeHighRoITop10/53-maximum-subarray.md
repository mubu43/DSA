# Maximum Subarray - LeetCode #53

## Problem Statement
Given an integer array `nums`, find the contiguous subarray (containing at least one number) which has the largest sum and return its sum.

A subarray is a contiguous part of an array.

## Approach 1: Kadane's Algorithm (Optimal)
**Dynamic Programming - Local vs Global Maximum**
- Track two values: maximum sum ending at current position, and global maximum
- At each position, decide whether to extend previous subarray or start fresh
- Update global maximum as we go

## Algorithm Steps (Kadane's)
1. Initialize `maxSoFar` and `maxEndingHere` with first element
2. For each element from index 1:
   - `maxEndingHere = max(current_element, maxEndingHere + current_element)`
   - `maxSoFar = max(maxSoFar, maxEndingHere)`
3. Return `maxSoFar`

## Approach 2: Divide and Conquer
**Recursive Solution**
- Divide array into two halves
- Maximum subarray is either:
  - Entirely in left half
  - Entirely in right half  
  - Crosses the middle point
- Return maximum of these three cases

## Time & Space Complexity
### Kadane's Algorithm:
- **Time Complexity**: O(n) - single pass
- **Space Complexity**: O(1) - constant space

### Divide and Conquer:
- **Time Complexity**: O(n log n) - T(n) = 2T(n/2) + O(n)
- **Space Complexity**: O(log n) - recursion stack

## Key Points for Revision
1. **Kadane's Core Insight**: At each position, either extend or restart
2. **Local vs Global**: Track both current subarray sum and best seen so far
3. **Negative Numbers**: Handle all-negative arrays correctly
4. **DP Transition**: `maxEndingHere = max(nums[i], maxEndingHere + nums[i])`
5. **Why It Works**: Negative prefix never helps future subarrays

## Common Variations
- **Maximum Product Subarray**: Handle positive/negative differently
- **Circular Maximum Subarray**: Array wraps around
- **K-Concatenation Maximum Sum**: Repeated arrays
- **Return Indices**: Track start/end positions of optimal subarray

## Edge Cases to Consider
- All negative numbers: return the largest (least negative)
- Single element array
- Empty array (if allowed)
- Array with zeros
- Very large numbers (overflow considerations)

## Interview Tips
1. **Start with Brute Force**: O(n³) or O(n²) approaches first
2. **Optimize Gradually**: Explain how to reach O(n) solution
3. **Handle Negatives**: Clarify behavior with all-negative arrays
4. **Trace Through Example**: Walk through algorithm step-by-step
5. **Discuss Trade-offs**: Kadane's vs Divide & Conquer
6. **Follow-up Questions**: Be ready for variations

## Mathematical Insight
Kadane's algorithm is based on the observation:
- If current sum becomes negative, it's better to start fresh
- Any subarray with negative sum as prefix can be improved by removing that prefix
- This greedy choice leads to optimal solution

## Applications
- **Stock Trading**: Maximum profit from price differences
- **Image Processing**: Finding brightest rectangular region
- **Biology**: Finding gene sequences with maximum score
- **Economics**: Periods of maximum economic growth
