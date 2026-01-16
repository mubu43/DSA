/**
 * 239. Sliding Window Maximum
 *
 * Problem:
 * Given an array and window size k, return the maximum in each window.
 *
 * Intuition:
 * Deque of indices maintaining decreasing values; front is max for current window.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(k)
 */

#include <iostream>
#include <vector>
#include <deque>
using namespace std;

vector<int> maxSlidingWindow(const vector<int>& nums, int k) {
    deque<int> dq; vector<int> res;
    for (int i = 0; i < (int)nums.size(); i++) {
        // remove indices out of window
        if (!dq.empty() && dq.front() <= i - k) dq.pop_front();
        // maintain decreasing deque
        while (!dq.empty() && nums[dq.back()] <= nums[i]) dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1) res.push_back(nums[dq.front()]);
    }
    return res;
}

int main() {
    auto print = [](const vector<int>& v){ for(int x:v) cout<<x<<' '; cout<<'\n'; };
    print(maxSlidingWindow({1,3,-1,-3,5,3,6,7}, 3)); // 3 3 5 5 6 7
    print(maxSlidingWindow({1}, 1)); // 1
    return 0;
}
