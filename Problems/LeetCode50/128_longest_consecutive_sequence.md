# 128. Longest Consecutive Sequence

## Problem
Given an unsorted array of integers, find the length of the longest consecutive elements sequence in O(n) time.

## Examples
- [100,4,200,1,3,2] → 4 (sequence 1,2,3,4)
- [0,3,7,2,5,8,4,6,0,1] → 9 (0..8)

## Intuition
A hash set supports O(1) average checks. Start only at numbers that have no predecessor (`x-1` absent) and extend forward.

## Algorithm (Hash Set)
1. Insert all numbers into a set.
2. For each `x` in set:
   - If `x-1` not in set, count `len` by advancing `x+1, x+2, ...` while present.
   - Update `best`.

## Correctness
- Starting only at sequence heads ensures each sequence is processed once, guaranteeing O(n) behavior.

## Complexity
- Time: O(n)
- Space: O(n)

## Edge Cases
- Duplicates are harmless due to set usage.
- Single element or empty array.

## References
- Hash set techniques
