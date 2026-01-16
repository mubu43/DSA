# 032. Longest Valid Parentheses

## Problem
Given a string `s` of '(' and ')', return the length of the longest valid (well-formed) parentheses substring.

## Examples
- "(()" → 2 (substring "()")
- ")()())" → 4 (substring "()()")
- "" → 0

## Intuition
We need to measure valid spans that balance parentheses. A stack of indices helps track boundaries efficiently: pushing indices for '(' and using a base index for unmatched ')'. When we find a matching pair, the current valid length is the distance from the last unmatched boundary to the current index.

## Algorithm (Stack of Indices)
1. Initialize stack with base `-1` to mark the start before the string.
2. For each index `i`:
   - If `s[i] == '('`, push `i`.
   - Else (')'): pop. If the stack is empty, push `i` as the new base (unmatched ')'). Otherwise, update `best = max(best, i - stack.top())`.
3. Return `best`.

## Correctness
- The base index represents the position before a valid substring starts. When a matching ')' is processed, the stack top is the index of the last unmatched '(' (or base), so subtracting gives the current valid length.
- Unmatched ')' resets the base, preventing invalid spans.

## Complexity
- Time: O(n)
- Space: O(n)

## Edge Cases
- All '(' or all ')' → 0.
- Nested and adjacent valid substrings handled naturally by index logic.

## Alternatives
- DP: `dp[i]` as the longest valid substring ending at `i`; careful transitions (O(n), O(n) space).
- Two-pass counters (left-to-right, right-to-left) counting '(' and ')' to capture valid spans without stack (O(n), O(1) space).

## References
- Stack-based parsing
- String scanning techniques
