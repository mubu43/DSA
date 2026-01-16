# 057. Insert Interval

## Problem Statement (paraphrased, detailed)
You are given a sorted list of non-overlapping intervals and a new interval. Insert the new interval into the list and merge any overlapping intervals to return a new sorted, non-overlapping list that covers all ranges.

### Examples
- `intervals=[[1,3],[6,9]], new=[2,5]` → `[[1,5],[6,9]]`
- `intervals=[[1,2],[3,5],[6,7],[8,10],[12,16]], new=[4,8]` → `[[1,2],[3,10],[12,16]]`

## Intuition
Because the input is already sorted and non-overlapping, we can do a single pass: output intervals that are guaranteed non-overlapping, merge those that overlap with the new interval, then append the remainder.

## Approach
1. Add all intervals that end before `new.start`.
2. Merge all intervals that overlap with `[new.start, new.end]` by expanding `new`’s bounds.
3. Append the remaining intervals after `new.end`.

## Correctness
- Phases partition the input into three sets relative to `new`: strictly before, overlapping, strictly after. Each phase preserves ordering and correctness by construction.

## Complexity
- Time: `O(n)`
- Space: `O(n)` for the result (in-place approaches exist but are less straightforward).

## Edge Cases
- Empty input
- New interval completely before or after all intervals
- New interval subsumed by an existing interval or subsumes multiple intervals

## Related Concepts
- Intervals
- Greedy merging
