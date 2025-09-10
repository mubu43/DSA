# Climbing Stairs - LeetCode #70

## Problem Statement
You are climbing a staircase. It takes `n` steps to reach the top. Each time you can either climb 1 or 2 steps. In how many distinct ways can you climb to the top?

## Key Insight
This is a **Fibonacci sequence in disguise**! 
- To reach step `n`, you can come from step `n-1` (take 1 step) or step `n-2` (take 2 steps)
- So: `ways(n) = ways(n-1) + ways(n-2)`

## Approach 1: Dynamic Programming (Bottom-up)
**Build solution from base cases upward**
- Start with known values: `dp[1] = 1`, `dp[2] = 2`
- For each step `i`, calculate `dp[i] = dp[i-1] + dp[i-2]`
- Return `dp[n]`

## Approach 2: Space-Optimized DP
**Fibonacci pattern optimization**
- Only need previous two values, not entire array
- Use two variables to track `prev1` and `prev2`
- Roll forward: `current = prev1 + prev2`

## Approach 3: Recursive + Memoization
**Top-down approach with caching**
- Recursive formula: `f(n) = f(n-1) + f(n-2)`
- Use memoization to avoid recomputing subproblems
- Cache results in array/map

## Algorithm Steps (Space-Optimized)
1. Handle base cases: if `n ≤ 2`, return `n`
2. Initialize: `prev2 = 1`, `prev1 = 2`
3. For `i` from 3 to `n`:
   - `current = prev1 + prev2`
   - `prev2 = prev1`
   - `prev1 = current`
4. Return `prev1`

## Time & Space Complexity
### DP Bottom-up:
- **Time**: O(n) - single loop
- **Space**: O(n) - dp array

### Space-Optimized:
- **Time**: O(n) - single loop  
- **Space**: O(1) - only variables

### Recursive + Memo:
- **Time**: O(n) - each subproblem solved once
- **Space**: O(n) - memoization array + recursion stack

## Key Points for Revision
1. **Fibonacci Recognition**: Pattern `f(n) = f(n-1) + f(n-2)`
2. **Base Cases**: `f(1) = 1`, `f(2) = 2`
3. **Space Optimization**: Only need last two values
4. **Memoization**: Top-down vs bottom-up DP
5. **Overflow Handling**: For large n, consider modular arithmetic

## Pattern Recognition
This problem teaches the fundamental DP pattern:
- **Optimal Substructure**: Solution depends on solutions to subproblems
- **Overlapping Subproblems**: Same subproblems appear multiple times
- **State Transition**: Clear recurrence relation

## Common Variations
- **Min Cost Climbing Stairs**: Each step has a cost
- **Climbing Stairs with k steps**: Can take 1, 2, ..., k steps
- **House Robber**: Similar DP pattern but with constraints
- **Decode Ways**: Another Fibonacci-like DP problem

## Edge Cases to Consider
- `n = 0`: Usually undefined or 1 by convention
- `n = 1`: Only one way (single step)
- `n = 2`: Two ways (1+1 or 2)
- Large `n`: Integer overflow considerations

## Visual Example (n=4)
```
Step 4: Can reach from step 2 (take 2) or step 3 (take 1)
Step 3: Can reach from step 1 (take 2) or step 2 (take 1)  
Step 2: Can reach from step 0 (take 2) or step 1 (take 1)
Step 1: Can reach from step 0 (take 1)

Ways to reach step 4 = Ways to step 3 + Ways to step 2 = 3 + 2 = 5
```

## Interview Tips
1. **Recognize the Pattern**: Point out it's Fibonacci sequence
2. **Start Simple**: Begin with recursive solution, then optimize
3. **Space Optimization**: Show how to reduce from O(n) to O(1) space
4. **Handle Base Cases**: Clarify behavior for small inputs
5. **Discuss Extensions**: Be ready for variations like min cost or k steps
6. **Time/Space Trade-offs**: Compare different approaches
