/**
 * 056. Merge Intervals
 *
 * Problem (paraphrased, detailed):
 * Given a list of intervals [start, end], merge all overlapping intervals and return the result
 * as a list of non-overlapping intervals covering the same ranges.
 *
 * Examples:
 * - [[1,3],[2,6],[8,10],[15,18]] -> [[1,6],[8,10],[15,18]]
 * - [[1,4],[4,5]] -> [[1,5]] (touching intervals count as overlapping here)
 *
 * Approach:
 * - Sort by start ascending. Iterate and maintain a current merged interval.
 * - If the next interval overlaps (next.start <= merged.end), extend merged.end.
 * - Else, push merged and start a new one.
 *
 * Complexity:
 * - Time: O(n log n) for sorting
 * - Space: O(n) for the output
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<vector<int>> merge(vector<vector<int>> intervals)
{
    if (intervals.empty()) return {};

    sort(intervals.begin(), intervals.end()); // sorts by first, then second element

    vector<vector<int>> res;
    res.push_back(intervals[0]); // seed with the first interval

    for (size_t i = 1; i < intervals.size(); i++)
    {
        auto& cur = intervals[i];
        auto& last = res.back();

        // Overlap if current start <= last end
        if (cur[0] <= last[1])
        {
            last[1] = max(last[1], cur[1]); // extend the merged interval
        }
        else
        {
            res.push_back(cur); // no overlap; start a new merged block
        }
    }
    return res;
}

int main()
{
    auto print = [](const vector<vector<int>>& v)
    {
        for (auto& it : v) cout << '[' << it[0] << ',' << it[1] << "] ";
        cout << "\n";
    };

    print(merge({{1,3},{2,6},{8,10},{15,18}})); // [[1,6],[8,10],[15,18]]
    print(merge({{1,4},{4,5}}));                 // [[1,5]]
    print(merge({}));                            // []
    return 0;
}
