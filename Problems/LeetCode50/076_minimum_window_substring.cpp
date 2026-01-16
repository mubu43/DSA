/**
 * 076. Minimum Window Substring
 *
 * Problem:
 * Given strings s and t, return the minimum window in s which contains all the characters of t. If no such window, return empty string.
 *
 * Intuition:
 * Sliding window with frequency counts: expand right to satisfy counts, then contract left greedily.
 *
 * Complexity:
 * - Time: O(|s| + |t|)
 * - Space: O(1) for ASCII (or O(Σ) for charset size)
 */

#include <iostream>
#include <string>
#include <vector>
using namespace std;

string minWindow(const string& s, const string& t) {
    if (t.empty() || s.size() < t.size()) return "";
    vector<int> need(128, 0), have(128, 0);
    int required = 0; // number of distinct chars needed
    for (char c : t) { if (need[c]++ == 0) required++; }

    int formed = 0; // number of chars meeting required count
    int bestLen = (int)s.size() + 1, bestL = 0;
    int l = 0;
    for (int r = 0; r < (int)s.size(); r++) {
        char cr = s[r];
        have[cr]++;
        if (need[cr] > 0 && have[cr] == need[cr]) formed++;
        // try to shrink
        while (l <= r && formed == required) {
            if (r - l + 1 < bestLen) { bestLen = r - l + 1; bestL = l; }
            char cl = s[l++];
            have[cl]--;
            if (need[cl] > 0 && have[cl] < need[cl]) formed--;
        }
    }
    return bestLen > (int)s.size() ? "" : s.substr(bestL, bestLen);
}

int main() {
    cout << minWindow("ADOBECODEBANC", "ABC") << "\n"; // BANC
    cout << minWindow("a", "a") << "\n";               // a
    cout << minWindow("a", "aa") << "\n";              // (empty)
    return 0;
}
