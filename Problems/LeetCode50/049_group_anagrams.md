# 049. Group Anagrams

## Problem Statement (paraphrased, detailed)
Given an array of strings, group the anagrams together. Two strings are anagrams if one can be rearranged to form the other (same character counts).

### Examples
- `["eat","tea","tan","ate","nat","bat"]` → groups like `["eat","tea","ate"]`, `["tan","nat"]`, `["bat"]`
- `[""]` → `[[""]]`

## Intuition
Anagrams share the same character multiset. If we map each word to a canonical representation of its multiset, equal keys identify anagrams.

## Approach (Counting Signature)
1. For each string, compute a 26-length count of letters `a..z`.
2. Convert the count array into a stable string key (e.g., `"#1#0#0..."`).
3. Insert the original string into a hash map from signature to vector of words.
4. The map’s values are the groups.

## Complexity
- Time: `O(n * L)` where `n` is number of strings and `L` is average length (counting signature is linear in `L`).
- Space: `O(n * L)` for storing results and keys.

## Edge Cases
- Empty strings
- Single-character strings
- Mixed uppercase/lowercase: normalize or expand the signature space.

## Alternatives & Follow-ups
- Sorted-string key: simpler but `O(L log L)` per word.
- Unicode: use a hash map for counts or sort graphemes.

## Related Concepts
- Hash Maps
- String Normalization / Canonicalization
