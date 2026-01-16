# 022. Generate Parentheses

## Problem Statement (paraphrased, detailed)
Given `n` pairs of parentheses, return all combinations of well-formed parentheses.

### Examples
- `n=3` → `["((()))","(()())","(())()","()(())","()()()"]`
- `n=1` → `["()"]`

## Intuition
We construct the string by deciding at each step whether to add `(` or `)`. The only valid states are those where we never close more parentheses than have been opened.

## Approach (Backtracking)
1. Maintain `openUsed` and `closeUsed`.
2. You may add `(` if `openUsed < n`.
3. You may add `)` if `closeUsed < openUsed`.
4. When the string reaches length `2n`, record it.

## Correctness
- The constraints ensure partial strings are always valid prefixes. Exploring all valid choices yields every well-formed sequence exactly once.

## Complexity
- Time: `O(Catalan(n))` due to the number of valid sequences.
- Space: `O(n)` recursion depth.

## Follow-ups
- Generalize to other bracket types.
- Count only (Catalan number) rather than generating strings.

## Related Concepts
- Backtracking
- Combinatorics (Catalan numbers)
