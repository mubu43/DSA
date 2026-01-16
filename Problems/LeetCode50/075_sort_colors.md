# 075. Sort Colors

## Problem
Sort an array of 0s, 1s, and 2s in-place.

## Examples
- [2,0,2,1,1,0] → [0,0,1,1,2,2]

## Intuition
Use the Dutch National Flag algorithm with three pointers:
- `low`: boundary for next 0 placement
- `mid`: current index
- `high`: boundary for next 2 placement

## Algorithm
- While `mid <= high`:
  - If `a[mid]==0`, swap with `low` and advance both.
  - If `a[mid]==1`, just advance `mid`.
  - If `a[mid]==2`, swap with `high` and decrement `high`.

## Correctness
- Invariants maintain zones: [0..low-1]=0, [low..mid-1]=1, [high+1..end]=2.

## Complexity
- Time: O(n)
- Space: O(1)

## References
- Dutch National Flag problem
