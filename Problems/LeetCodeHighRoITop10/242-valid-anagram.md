# Valid Anagram - LeetCode #242

## Problem Statement
Given two strings `s` and `t`, return `true` if `t` is an anagram of `s`, and `false` otherwise.

An **anagram** is a word or phrase formed by rearranging the letters of a different word or phrase, typically using all the original letters exactly once.

## Key Insight
Two strings are anagrams if they contain exactly the same characters with the same frequencies.

## Approach 1: Character Frequency Array (Optimal)
**Use fixed-size array for character counting**
- Since we're dealing with lowercase English letters (26 characters)
- Use array of size 26 to count character frequencies
- Increment for first string, decrement for second string
- All counts should be zero if they're anagrams

## Approach 2: Hash Map
**Universal solution for any character set**
- Count frequency of each character in both strings
- Compare the frequency maps
- Works for Unicode, uppercase, special characters

## Approach 3: Sorting
**Simple but less efficient**
- Sort both strings
- Compare if sorted strings are equal
- Easy to understand but O(n log n) time complexity

## Algorithm Steps (Character Array)
1. Check if lengths are equal (early return if not)
2. Create frequency array of size 26 (for 'a' to 'z')
3. For each character in both strings:
   - Increment count for character in string `s`
   - Decrement count for character in string `t`
4. Check if all counts in array are zero
5. Return true if all zeros, false otherwise

## Time & Space Complexity
### Character Array:
- **Time**: O(n) - single pass through both strings
- **Space**: O(1) - fixed array of size 26

### Hash Map:
- **Time**: O(n) - single pass through both strings
- **Space**: O(k) - where k is number of unique characters

### Sorting:
- **Time**: O(n log n) - due to sorting
- **Space**: O(1) - in-place sorting (or O(n) if creating copies)

## Key Points for Revision
1. **Length Check**: First optimization - different lengths can't be anagrams
2. **Character Frequency**: Core concept - same characters, same counts
3. **Array vs HashMap**: Choose based on character set constraints
4. **Simultaneous Processing**: Count both strings in single loop
5. **Early Termination**: Return false as soon as mismatch found

## Common Patterns
- **Frequency Counting**: Fundamental technique for string problems
- **Character Mapping**: 'a' → 0, 'b' → 1, etc. using `char - 'a'`
- **Hash Table Usage**: When character set is large or unknown

## Edge Cases to Consider
- Empty strings (both empty = anagram)
- Single character strings
- Strings of different lengths
- Identical strings (always anagrams)
- Case sensitivity (if applicable)
- Unicode characters (if applicable)

## Related Problems
- **Group Anagrams**: Group strings that are anagrams of each other
- **Find All Anagrams**: Find all anagram substrings in a string
- **Anagram Mapping**: Create character mapping between anagrams
- **Palindrome Permutation**: Check if string can form palindrome

## Optimizations
1. **Early Length Check**: O(1) operation that can save O(n) work
2. **Single Pass**: Count both strings simultaneously
3. **Immediate False Return**: Stop as soon as frequency goes negative

## Interview Tips
1. **Clarify Constraints**: Ask about character set (lowercase only? Unicode?)
2. **Start Simple**: Begin with sorting approach, then optimize
3. **Discuss Trade-offs**: Time vs space, array vs hash map
4. **Handle Edge Cases**: Empty strings, different lengths
5. **Consider Follow-ups**: Be ready for group anagrams variant
6. **Code Cleanly**: Show different approaches if time permits

## Visual Example
```
s = "listen", t = "silent"

Character frequency counting:
l: +1 (from s), 0 (net: +1)
i: +1 (from s), -1 (from t), (net: 0)
s: +1 (from s), -1 (from t), (net: 0)
t: +1 (from s), -1 (from t), (net: 0)
e: +1 (from s), -1 (from t), (net: 0)
n: +1 (from s), -1 (from t), (net: 0)

All net counts are 0 → Valid anagram!
```

## Alternative Implementations
- **XOR Approach**: Won't work (doesn't handle duplicates)
- **Prime Numbers**: Assign prime to each character, multiply (risk of overflow)
- **Bit Manipulation**: Only works if each character appears once
