# Contains Duplicate - LeetCode #217

## Problem Statement
Given an integer array `nums`, return `true` if any value appears **at least twice** in the array, and return `false` if every element is distinct.

## Key Insight
We need to detect if any element appears more than once. This is a classic application of hash sets for O(1) lookup.

## Approach 1: Hash Set (Optimal)
**Track seen elements as we iterate**
- Use hash set to store elements we've already seen
- For each element, check if it's already in the set
- If yes, we found a duplicate; if no, add it to set
- Continue until duplicate found or array exhausted

## Approach 2: Sorting
**Sort then check adjacent elements**
- Sort the array first
- Check consecutive elements for equality
- Duplicates will be adjacent after sorting
- Space-efficient but slower due to sorting

## Approach 3: Set Size Comparison
**Compare original vs unique element counts**
- Create set from entire array (removes duplicates)
- If set size < array size, duplicates existed
- Simple but processes entire array

## Algorithm Steps (Hash Set)
1. Create empty hash set
2. For each number in array:
   - If number exists in set, return true
   - Otherwise, add number to set
3. If loop completes, return false (no duplicates)

## Time & Space Complexity
### Hash Set:
- **Time**: O(n) - single pass through array
- **Space**: O(n) - hash set storage

### Sorting:
- **Time**: O(n log n) - due to sorting step
- **Space**: O(1) - in-place sorting

### Set Size:
- **Time**: O(n) - create set from array
- **Space**: O(n) - set storage

### Brute Force:
- **Time**: O(n²) - nested loops
- **Space**: O(1) - no extra storage

## Key Points for Revision
1. **Hash Set Benefits**: O(1) average lookup time
2. **Early Termination**: Return immediately upon finding duplicate
3. **Space-Time Trade-off**: Hash set uses space for speed
4. **Insert Return Value**: `set.insert().second` indicates if insertion happened
5. **Alternative Methods**: Multiple valid approaches with different trade-offs

## Common Patterns
- **Duplicate Detection**: Hash set for O(n) solution
- **Seen Before**: Common pattern in array problems
- **Set Operations**: Using set properties for problem solving

## Edge Cases to Consider
- Empty array (no duplicates possible)
- Single element array (no duplicates possible)
- All elements are the same
- Array with negative numbers
- Array with zeros
- Very large arrays (performance considerations)

## Related Problems
- **Contains Duplicate II**: Duplicates within k distance
- **Contains Duplicate III**: Values within range
- **Find All Duplicates**: Return all duplicate elements
- **Single Number**: Find element that appears once

## Interview Tips
1. **Start with Hash Set**: Most optimal for general case
2. **Discuss Trade-offs**: Time vs space complexity
3. **Consider Constraints**: Array size, memory limitations
4. **Handle Edge Cases**: Empty arrays, single elements
5. **Alternative Solutions**: Mention sorting approach
6. **Follow-up Questions**: Be ready for variations

## Implementation Variations
```cpp
// Method 1: Explicit check
if (seen.find(num) != seen.end()) return true;
seen.insert(num);

// Method 2: Using insert return value
if (!seen.insert(num).second) return true;

// Method 3: Count vs insert
if (seen.count(num)) return true;
seen.insert(num);
```

## When to Use Each Approach
### Hash Set:
- General case, optimal time complexity needed
- Memory is available
- Array can be very large

### Sorting:
- Memory is limited
- Array modification is allowed
- Slightly slower time is acceptable

### Set Size:
- When you need all unique elements anyway
- Simple one-liner solution
- Less efficient but very readable

## Mathematical Properties
- **Pigeonhole Principle**: If array size > range of possible values, duplicates must exist
- **Birthday Paradox**: Related probability concept for collision detection
- **Load Factor**: Hash set performance depends on load factor
