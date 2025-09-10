# Valid Parentheses - LeetCode #20

## Problem Statement
Given a string `s` containing just the characters `'('`, `')'`, `'{'`, `'}'`, `'['` and `']'`, determine if the input string is valid.

An input string is valid if:
1. Open brackets must be closed by the same type of brackets
2. Open brackets must be closed in the correct order
3. Every close bracket has a corresponding open bracket of the same type

## Approach
**Stack-Based Solution**
- Use a stack to keep track of opening brackets
- When we encounter a closing bracket, check if it matches the most recent opening bracket
- Use a hash map to quickly lookup which opening bracket corresponds to each closing bracket

## Algorithm Steps
1. Create an empty stack for opening brackets
2. Create a hash map mapping closing brackets to opening brackets
3. Iterate through each character in the string:
   - If it's a closing bracket:
     - Check if stack is empty (no matching opening bracket)
     - Check if top of stack matches the required opening bracket
     - If either check fails, return false
     - Otherwise, pop from stack
   - If it's an opening bracket:
     - Push it onto the stack
4. Return true if stack is empty (all brackets matched)

## Time & Space Complexity
- **Time Complexity**: O(n) - single pass through string
- **Space Complexity**: O(n) - stack can hold up to n/2 opening brackets

## Key Points for Revision
1. **Stack Usage**: LIFO property perfectly matches bracket nesting
2. **Hash Map Lookup**: Quick way to check bracket correspondence
3. **Early Termination**: Return false immediately on mismatch
4. **Empty Stack Check**: Important for unmatched closing brackets
5. **Final Stack State**: Must be empty for valid string

## Common Patterns
- **Parentheses Matching**: Core stack application
- **Nested Structures**: HTML tags, function calls, etc.
- **Expression Parsing**: Mathematical expressions, code parsing

## Edge Cases to Consider
- Empty string (valid)
- Only opening brackets: "((("
- Only closing brackets: ")))"
- Wrong order: "([)]"
- Single character: "(" or ")"
- Mixed valid/invalid: "()[]{}("

## Alternative Approaches
1. **Counter Method**: For single bracket type, just count opens/closes
2. **Recursive Descent**: For more complex parsing
3. **Two-Pass**: First pass count, second pass validate (less efficient)

## Interview Tips
1. **Clarify Input**: Ask about input constraints and edge cases
2. **Start Simple**: Begin with single bracket type, then extend
3. **Explain Stack Choice**: Why LIFO is perfect for this problem
4. **Handle Edge Cases**: Empty string, unmatched brackets
5. **Optimize Space**: Discuss when counter method might be better
6. **Related Problems**: Mention similar parsing problems
