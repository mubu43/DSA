/**
 * 057. Insert Interval
 *
 * Problem (paraphrased, detailed):
 * Given a list of non-overlapping intervals sorted by start time, insert a new interval and
 * merge if necessary to produce a list of non-overlapping intervals covering all ranges.
 *
 * Examples:
 * - intervals=[[1,3],[6,9]], new=[2,5] -> [[1,5],[6,9]]
 * - intervals=[[1,2],[3,5],[6,7],[8,10],[12,16]], new=[4,8] -> [[1,2],[3,10],[12,16]]
 *
 * Approach:
 * - Append all intervals that end strictly before newInterval starts.
 * - Merge overlaps with newInterval (expand newInterval bounds as needed).
 * - Append the rest.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

vector<vector<int>> insert(vector<vector<int>> intervals, vector<int> newInterval)
{
    vector<vector<int>> res;
    size_t i = 0, n = intervals.size();

    // 1) Add all intervals that end before newInterval starts (no overlap)
    while (i < n && intervals[i][1] < newInterval[0])
    {
        res.push_back(intervals[i++]);
    }

    // 2) Merge all intervals that overlap with newInterval
    while (i < n && intervals[i][0] <= newInterval[1])
    {
        newInterval[0] = min(newInterval[0], intervals[i][0]);
        newInterval[1] = max(newInterval[1], intervals[i][1]);
        i++;
    }
    res.push_back(newInterval);

    // 3) Append the remaining intervals
    while (i < n)
    {
        res.push_back(intervals[i++]);
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

    print(insert({{1,3},{6,9}}, {2,5}));                      // [[1,5],[6,9]]
    print(insert({{1,2},{3,5},{6,7},{8,10},{12,16}}, {4,8})); // [[1,2],[3,10],[12,16]]
    print(insert({}, {5,7}));                                  // [[5,7]]
    return 0;
}
