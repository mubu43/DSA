/**
 * 084. Largest Rectangle in Histogram
 *
 * Problem:
 * Given heights of bars, find the largest rectangle area in the histogram.
 *
 * Intuition:
 * Monotonic stack of indices. Maintain increasing heights; when a lower height comes, pop and compute area with popped height as the smallest bar.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(n)
 */

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
using namespace std;

int largestRectangleArea(const vector<int>& h) {
    int n = h.size();
    stack<int> st; // indices of increasing heights
    long long best = 0;
    for (int i = 0; i <= n; i++) {
        int cur = (i == n ? 0 : h[i]);
        while (!st.empty() && cur < h[st.top()]) {
            int height = h[st.top()]; st.pop();
            int left = st.empty() ? -1 : st.top();
            long long width = i - left - 1;
            best = max(best, width * height);
        }
        st.push(i);
    }
    return (int)best;
}

int main() {
    cout << largestRectangleArea({2,1,5,6,2,3}) << "\n"; // 10
    cout << largestRectangleArea({2,4}) << "\n";          // 4
    return 0;
}
