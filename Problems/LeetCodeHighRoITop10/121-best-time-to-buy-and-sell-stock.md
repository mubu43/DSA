# Best Time to Buy and Sell Stock - LeetCode #121

## Problem Statement
You are given an array `prices` where `prices[i]` is the price of a given stock on the ith day.

You want to maximize your profit by choosing a **single day** to buy one stock and choosing a **different day in the future** to sell that stock.

Return the maximum profit you can achieve from this transaction. If you cannot achieve any profit, return 0.

## Key Constraints
- You can only hold at most one share at any time
- You must buy before you sell
- Only one complete transaction allowed

## Approach 1: Single Pass (Optimal)
**Track minimum price and maximum profit**
- Keep track of the lowest price seen so far (best buying opportunity)
- For each day, calculate profit if we sell at current price
- Update maximum profit if current profit is better

## Approach 2: Kadane's Algorithm Variant
**Transform to maximum subarray problem**
- Convert price array to daily profit differences
- Find maximum sum of contiguous subarray (consecutive days)
- This gives maximum profit from any buy-sell period

## Algorithm Steps (Single Pass)
1. Initialize `minPrice = prices[0]` and `maxProfit = 0`
2. For each day from day 1:
   - Update `minPrice = min(minPrice, current_price)`
   - Calculate `currentProfit = current_price - minPrice`
   - Update `maxProfit = max(maxProfit, currentProfit)`
3. Return `maxProfit`

## Time & Space Complexity
### Single Pass:
- **Time**: O(n) - one iteration through array
- **Space**: O(1) - only using constant variables

### Kadane's Variant:
- **Time**: O(n) - one iteration through differences
- **Space**: O(1) - constant space

### Brute Force:
- **Time**: O(n²) - check all buy-sell pairs
- **Space**: O(1) - no extra space

## Key Points for Revision
1. **Greedy Insight**: Always buy at lowest price seen so far
2. **Future Selling**: Can only sell after buying (array constraint)
3. **One Transaction**: Unlike multiple transaction variants
4. **Zero Profit**: If no profit possible, return 0
5. **Connection to Maximum Subarray**: Transform problem to familiar pattern

## Problem Transformation
Original: Find max(prices[j] - prices[i]) where j > i
Transform to: Find maximum sum of consecutive daily profits

Example: [7,1,5,3,6,4] → [-6,4,-2,3,-2] → max subarray sum = 5

## Common Variations
- **Best Time II**: Multiple transactions allowed
- **Best Time III**: At most 2 transactions
- **Best Time IV**: At most k transactions
- **With Cooldown**: Must wait one day between transactions
- **With Fee**: Pay transaction fee

## Edge Cases to Consider
- Empty array or single element
- Strictly decreasing prices (no profit possible)
- Strictly increasing prices (buy first, sell last)
- All same prices (no profit)
- Very large price differences

## Interview Tips
1. **Start with Brute Force**: Mention O(n²) approach checking all pairs
2. **Optimize with Insight**: Explain why tracking minimum is sufficient
3. **Draw Examples**: Visualize buying/selling points
4. **Handle Edge Cases**: Empty array, single day, no profit scenarios
5. **Discuss Variations**: Be ready for multiple transaction problems
6. **Mathematical Connection**: Relate to maximum subarray if asked

## Visual Example
```
Prices: [7, 1, 5, 3, 6, 4]
Days:    0  1  2  3  4  5

Day 0: minPrice=7, profit=0
Day 1: minPrice=1, profit=0 (1-1=0)
Day 2: minPrice=1, profit=4 (5-1=4)
Day 3: minPrice=1, profit=4 (3-1=2 < 4)
Day 4: minPrice=1, profit=5 (6-1=5)
Day 5: minPrice=1, profit=5 (4-1=3 < 5)

Best strategy: Buy at day 1 (price=1), sell at day 4 (price=6)
Maximum profit: 6 - 1 = 5
```

## Why This Works
- We want to maximize `sell_price - buy_price`
- For any sell day, the best buy day is the one with minimum price before it
- By tracking running minimum, we always have the best buy price for current sell day
