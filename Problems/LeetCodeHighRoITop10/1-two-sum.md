# Two Sum - LeetCode #1

## Problem Statement
Given an array of integers `nums` and an integer `target`, return indices of the two numbers such that they add up to target.

You may assume that each input would have exactly one solution, and you may not use the same element twice.

## Approach
**Hash Map Technique**
- Use a hash map to store each number and its index as we iterate
- For each element, calculate the complement (target - current number)
- Check if the complement exists in our hash map
- If yes, we found our pair; if no, add current number to hash map

## Algorithm Steps
1. Create an empty hash map to store `value -> index` mappings
2. Iterate through the array with index `i`
3. Calculate `complement = target - nums[i]`
4. Check if `complement` exists in hash map
5. If exists, return `[map[complement], i]`
6. Otherwise, store `nums[i] -> i` in hash map
7. Continue to next element

## Time & Space Complexity
- **Time Complexity**: O(n) - single pass through array
- **Space Complexity**: O(n) - hash map storage

## Key Points for Revision
1. **Hash Map Pattern**: This is a classic "complement lookup" pattern
2. **One Pass Solution**: We don't need to check all pairs (O(n²))
3. **Index Handling**: We return indices, not the actual values
4. **Early Return**: As soon as we find a match, we return immediately
5. **No Duplicate Usage**: Each element is used only once

## Common Variations
- **Two Sum II**: Array is sorted (use two pointers)
- **3Sum**: Find triplets that sum to zero
- **4Sum**: Find quadruplets with target sum
- **Two Sum in BST**: Use tree traversal

## Edge Cases to Consider
- Empty array (not possible per constraints)
- Array with 2 elements (minimum valid input)
- Negative numbers in array
- Target is 0
- Same number appears multiple times

## Interview Tips
1. **Start with brute force**: Mention O(n²) approach first
2. **Optimize with hash map**: Explain the complement lookup technique
3. **Discuss trade-offs**: Time vs space complexity
4. **Handle edge cases**: Ask about constraints and edge cases
5. **Test with examples**: Walk through the algorithm with given examples
