/**
 * 003. Longest Substring Without Repeating Characters
 *
 * Problem (detailed, paraphrased):
 * Given a string s, return the length of the longest substring that contains no repeated
 * characters. A substring is contiguous and order matters. If s is empty, return 0.
 *
 * Examples:
 * - s = "abcabcbb" -> 3 ("abc"): the longest contiguous run with all unique characters.
 * - s = "bbbbb"   -> 1 ("b"): any single character is the best we can do.
 * - s = "pwwkew"  -> 3 ("wke"): note "pwke" isn't contiguous; only substrings count.
 *
 * Constraints (typical):
 * - 0 <= s.length <= 5e4 (or higher on some platforms)
 * - s may contain ASCII or Unicode. For ASCII solutions, we can use a 256-length table.
 *
 * Intuition:
 * - We want the longest window (contiguous range) with all unique characters.
 * - Sliding window fits: expand right pointer; if a duplicate appears, shrink from the left
 *   just enough to remove that duplicate from the window.
 *
 * Algorithm (last seen index variant):
 * - Maintain an array last[256] initialized to -1 that stores last index where each character appeared.
 * - Maintain left boundary of the window and iterate right from 0..n-1.
 * - For character c at position right:
 *     If last[c] >= left, it means c is already inside the current window; move left to last[c] + 1.
 *     Update last[c] = right.
 *     Update answer = max(answer, right - left + 1).
 *
 * Correctness:
 * - The window [left..right] always has unique characters by construction. Whenever we see a repeat
 *   of c within the window, we move left just past the previous c to restore uniqueness.
 * - Each index moves forward at most once, so the algorithm is linear.
 *
 * Complexity:
 * - Time: O(n). Each character index is processed at most twice (enter window, potentially causes left to jump).
 * - Space: O(min(n, alphabet)). With ASCII, O(256).
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// Returns the length of the longest substring of s with all unique characters.
int lengthOfLongestSubstring(const string& s)
{
    // last[pos] = most recent index where character with code pos was seen; -1 if never
    vector<int> last(256, -1); // If input can be Unicode, consider unordered_map<char32_t,int>

    int best = 0;   // best window length so far
    int left = 0;   // current window left boundary

    for (int right = 0; right < (int)s.size(); right++)
    {
        unsigned char c = s[right];

        // If we have seen c at index last[c], and that index lies inside the current window
        // (i.e., >= left), then the window is no longer valid. Move left to just after that
        // previous occurrence to make the window valid again.
        if (last[c] >= left)
        {
            left = last[c] + 1;
        }

        // Record the most recent occurrence of c
        last[c] = right;

        // Update the best length with the current window size [left..right]
        best = max(best, right - left + 1);
    }

    return best;
}

int main()
{
    // Basic sanity checks with expected outputs in comments
    cout << lengthOfLongestSubstring("abcabcbb") << "\n"; // 3 ("abc")
    cout << lengthOfLongestSubstring("bbbbb") << "\n";    // 1 ("b")
    cout << lengthOfLongestSubstring("pwwkew") << "\n";   // 3 ("wke")

    // Edge cases
    cout << lengthOfLongestSubstring("") << "\n";         // 0: empty string
    cout << lengthOfLongestSubstring(" ") << "\n";        // 1: single whitespace
    cout << lengthOfLongestSubstring("au") << "\n";       // 2: both unique
    cout << lengthOfLongestSubstring("dvdf") << "\n";     // 3: "vdf"

    return 0;
}
