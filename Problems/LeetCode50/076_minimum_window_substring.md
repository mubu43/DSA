# 076. Minimum Window Substring

## Problem
Given `s` and `t`, find the minimum window in `s` that contains all characters of `t` (including multiplicity). Return empty string if none.

## Examples
- s = "ADOBECODEBANC", t = "ABC" → "BANC"
- s = "a", t = "a" → "a"
- s = "a", t = "aa" → ""

## Intuition
Maintain counts of needed characters and a sliding window that grows until it satisfies all counts, then shrink greedily to find the minimal window.

## Algorithm (Sliding Window)
1. Count `need[c]` from `t` and compute number of distinct required characters.
2. Expand right pointer `r`, updating `have` counts and `formed` when a character meets its requirement.
3. While `formed == required`, update best window and move left pointer `l`, decreasing counts and `formed` when necessary.

## Correctness
- Window is only considered valid when all required counts are met; shrinking maintains minimality when possible.

## Complexity
- Time: O(|s| + |t|)
- Space: O(Σ) where Σ is character set size (ASCII usage here).

## Edge Cases
- `t` longer than `s` → empty.
- Repeated characters in `t` handled via counts.

## References
- Sliding window, frequency counting
