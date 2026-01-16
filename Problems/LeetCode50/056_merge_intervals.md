# 056. Merge Intervals

## Problem Statement (paraphrased, detailed)
Given a list of intervals `[start, end]`, merge all overlapping intervals and return a list of non-overlapping intervals that cover all the intervals in the input.

### Examples
- `[[1,3],[2,6],[8,10],[15,18]]` → `[[1,6],[8,10],[15,18]]`
- `[[1,4],[4,5]]` → `[[1,5]]`

## Intuition
Sorting by start time allows a single left-to-right sweep to merge overlaps. Once sorted, any new overlap can only extend the current merged interval’s end.

## Approach
1. Sort intervals by `start` (and then by `end`).
2. Initialize `res` with the first interval.
3. For each subsequent interval `cur`:
   - If `cur.start <= last.end`, set `last.end = max(last.end, cur.end)`.
   - Else, append `cur` as a new merged interval.

## Correctness
- Sorting ensures that when we see an interval, any potential overlaps must involve the last merged interval. The greedy extension is optimal because it always maintains the largest merged coverage seen so far.

## Complexity
- Time: `O(n log n)` for sorting.
- Space: `O(n)` for the result (in-place variants are also common).

## Edge Cases
- Empty input → empty output.
- Single interval → same interval.
- Intervals that only touch at endpoints (problem typically treats as overlapping).

## Related Concepts
- Sorting
- Greedy Sweep
