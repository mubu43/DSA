# 208. Implement Trie (Prefix Tree)

## Problem
Implement a Trie with methods `insert`, `search`, and `startsWith`.

## Examples
- insert "apple", search "apple" → true
- search "app" → false; startsWith "app" → true
- insert "app", search "app" → true

## Intuition
Each node stores links to children for letters 'a'..'z' and a boolean marking end-of-word.

## API / Algorithm
- `insert(word)`: traverse/allocate nodes for each character and set end.
- `search(word)`: traverse; return true only if final node has `end=true`.
- `startsWith(prefix)`: traverse; return true if traversal succeeds.

## Complexity
- Time: O(L) per operation (L = word length)
- Space: O(total inserted characters)

## Edge Cases
- Empty string insert/search (define behavior as needed).
- Non-lowercase input requires mapping (this implementation assumes 'a'..'z').

## References
- Trie / prefix trees
