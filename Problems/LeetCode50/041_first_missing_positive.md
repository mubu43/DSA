# 041. First Missing Positive

## Problem
Given an unsorted integer array, return the smallest missing positive integer.

## Examples
- [1,2,0] → 3
- [3,4,-1,1] → 2
- [7,8,9,11,12] → 1

## Intuition
The answer lies in the range `[1..n+1]` where `n` is the array length. Place each number `v` into its correct index `v-1` via cyclic swaps. Then scan for the first position where `nums[i] != i+1`.

## Algorithm (Index Placement / Cyclic Sort)
1. For each index `i`, while `nums[i]` is in `[1..n]` and `nums[nums[i]-1] != nums[i]`, swap `nums[i]` with `nums[nums[i]-1]`.
2. After placement, scan `i = 0..n-1`: if `nums[i] != i+1`, return `i+1`.
3. If all match, return `n+1`.

## Correctness
- After placement, any number `v` in `[1..n]` resides at index `v-1`. The first mismatch identifies the missing integer.
- Numbers outside `[1..n]` are irrelevant and ignored via conditions.

## Complexity
- Time: O(n) (each element is moved at most once into position).
- Space: O(1).

## Edge Cases
- Contains negatives and zeros → ignored.
- Duplicates → placement guard `nums[nums[i]-1] != nums[i]` avoids infinite loops.

## Alternatives
- Hash set to check presence in `[1..n]` (O(n) time, O(n) space).

## References
- Cyclic sort / index placement pattern
