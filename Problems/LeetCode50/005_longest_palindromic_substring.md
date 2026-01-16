# 005. Longest Palindromic Substring

## Problem Statement (paraphrased, detailed)
Given a string `s`, return the longest contiguous substring of `s` that is a palindrome. If multiple answers exist, any one is acceptable.

### Examples
- `"babad"` → `"bab"` (or `"aba"`)
- `"cbbd"` → `"bb"`
- `"a"` → `"a"`
- `"ac"` → `"a"` (or `"c"`)

## Intuition
Every palindrome mirrors around a center. We can test all centers and expand outward as long as the characters match. There are `2n-1` such centers in a string of length `n` (n odd centers, n-1 even centers).

## Approach (Expand Around Center)
1. For each index `i`:
   - Expand around `(i, i)` for odd-length palindromes.
   - Expand around `(i, i+1)` for even-length palindromes.
2. Track the best `[start, end]` window across all expansions.
3. Return the substring defined by this window.

## Correctness
- Expanding outward from a center enumerates all maximal palindromes with that center. As we test all possible centers, we must encounter the global longest palindrome.

## Complexity
- Time: `O(n^2)` worst-case (e.g., uniform strings like `"aaaa"`). Each expansion may traverse `O(n)` and there are `O(n)` centers.
- Space: `O(1)` extra.

## Edge Cases
- Empty string → return empty string.
- Single character → the character itself.
- All characters identical → entire string.

## Alternatives & Follow-ups
- Manacher’s algorithm: `O(n)` time with more complex implementation.
- Dynamic Programming: `O(n^2)` time and `O(n^2)` space (or `O(n)` with optimization), easier to reason about but slower than center expansion in practice.

## Related Concepts
- Two Pointers
- String Algorithms
- Expand Around Center
