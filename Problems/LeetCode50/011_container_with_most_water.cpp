/**
 * 011. Container With Most Water
 *
 * Problem (paraphrased, detailed):
 * Given an array heights where heights[i] is the height of a vertical line at x = i, choose two lines
 * that together with the x-axis form a container holding the maximal amount of water.
 * The area is min(heights[l], heights[r]) * (r - l). Return the maximal area.
 *
 * Examples:
 * - [1,8,6,2,5,4,8,3,7] -> 49 (between indices 1 and 8: min(8,7)*(8-1)=7*7=49)
 * - [1,1] -> 1
 *
 * Intuition:
 * - A brute force O(n^2) approach checks all pairs (l, r). We can do better with two pointers.
 * - Start with l=0 and r=n-1. The current area depends on the shorter side; moving the taller side
 *   cannot help because width shrinks and min height cannot increase from that side. Move the shorter side inward
 *   to potentially find a taller line that compensates for the reduced width.
 *
 * Algorithm (Two Pointers):
 * - Initialize l=0, r=n-1, best=0.
 * - While l<r:
 *     best = max(best, min(heights[l], heights[r]) * (r - l))
 *     If heights[l] < heights[r], l++ else r--
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int maxArea(const vector<int>& h)
{
    int l = 0, r = (int)h.size() - 1;
    int best = 0;

    while (l < r)
    {
        // Compute current area using the limiting (shorter) side and the span (r-l)
        int height = min(h[l], h[r]);
        int width  = r - l;
        best = max(best, height * width);

        // Move the pointer at the shorter line inward to try to find a taller line
        if (h[l] < h[r])
            l++;
        else
            r--;
    }
    return best;
}

int main()
{
    cout << maxArea({1,8,6,2,5,4,8,3,7}) << "\n"; // 49
    cout << maxArea({1,1}) << "\n";                 // 1
    cout << maxArea({4,3,2,1,4}) << "\n";           // 16
    cout << maxArea({1,2,1}) << "\n";               // 2
    return 0;
}
