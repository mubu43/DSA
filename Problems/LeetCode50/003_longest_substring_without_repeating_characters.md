# 003. Longest Substring Without Repeating Characters

## Problem Statement (paraphrased, detailed)
Given a string `s`, return the length of the longest contiguous substring that contains no repeated characters. Substrings are contiguous slices of the original string (subsequences do not qualify). If `s` is empty, return `0`.

### Examples
- `s = "abcabcbb"` → `3` (longest unique substring is `"abc"`)
- `s = "bbbbb"` → `1` (any single `"b"`)
- `s = "pwwkew"` → `3` (longest unique substring is `"wke"`; note `"pwke"` is not contiguous)

### Constraints (typical)
- `0 <= s.length`
- `s` may contain any visible ASCII characters (approach generalizes to Unicode with a different map)

## Intuition
We need the longest window of characters with no duplicates. Sliding window is a natural fit: expand the right boundary; when a duplicate enters the window, move the left boundary just past the previous occurrence of that character to restore uniqueness.

## Approach (Sliding Window with last-seen index)
- Maintain an array/map `last` that records the last index where each character appeared (initialize to `-1`).
- Keep a left boundary `left = 0` and iterate `right` over indices of `s`.
- For the current character `c = s[right]`:
  - If `last[c] >= left`, then `c` already exists in the window; set `left = last[c] + 1` to skip the previous `c`.
  - Update `last[c] = right`.
  - Update the best answer with `right - left + 1`.

## Correctness
- The window `[left..right]` always contains unique characters by construction. When a repeated character is found, moving `left` to `last[c] + 1` removes the previous copy of `c` from the window.
- Each index moves forward monotonically; the algorithm never backtracks, ensuring linear time.

## Complexity
- Time: `O(n)` — each character index is processed at most twice (entering the window and possibly causing a jump of `left`).
- Space: `O(min(n, K))` where `K` is the alphabet size (e.g., 256 for extended ASCII). Using a hash map supports larger alphabets/Unicode.

## Edge Cases
- Empty string → `0`.
- String with all identical characters → `1`.
- String with all unique characters → `n`.
- Mixed whitespace and punctuation — no change to logic.

## Alternatives & Follow-ups
- Use `unordered_map<char,int>` instead of a fixed-size array if inputs can be large Unicode sets.
- To return the actual substring, track the best window’s start and length while computing.
- If case-insensitive behavior is desired, normalize input (e.g., convert to lowercase) before processing.

## Related Concepts
- Sliding Window
- Hash Map / Index Map
- Two Pointers
