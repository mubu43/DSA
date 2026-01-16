/**
 * 005. Longest Palindromic Substring
 *
 * Problem (paraphrased, detailed):
 * Given a string s, return the longest substring of s that is a palindrome. If multiple answers
 * exist, any one is acceptable. A palindrome reads the same forward and backward.
 *
 * Examples:
 * - s = "babad" -> "bab" (or "aba")
 * - s = "cbbd"  -> "bb"
 * - s = "a"     -> "a"
 * - s = "ac"    -> "a" (or "c")
 *
 * Intuition:
 * - Every palindrome can be described by its center. There are 2n-1 centers in a string of length n:
 *   n centers for odd-length palindromes (center at a character) and n-1 for even-length palindromes
 *   (center between two characters).
 *
 * Algorithm (Expand Around Center):
 * - For each index i, expand from (i, i) for odd and (i, i+1) for even palindromes while the
 *   characters match and bounds are valid. Track the best [start,end] window found.
 *
 * Complexity:
 * - Time: O(n^2) in the worst case (e.g., "aaaa...") because each expansion can scan across the string.
 * - Space: O(1) extra.
 */

#include <iostream>
#include <string>
#include <utility>
using namespace std;

// Expand from the given left/right center and return the final inclusive bounds of the palindrome.
static pair<int,int> expand(const string& s, int l, int r)
{
    while (l >= 0 && r < (int)s.size() && s[l] == s[r])
    {
        l--; r++;
    }
    // When the while-loop exits, l and r are one step beyond the valid palindrome bounds.
    return {l + 1, r - 1};
}

string longestPalindrome(const string& s)
{
    if (s.empty()) return "";

    int start = 0, end = 0; // best window [start..end]

    for (int i = 0; i < (int)s.size(); i++)
    {
        // Try odd-length center at i
        auto odd = expand(s, i, i);

        // Try even-length center between i and i+1
        auto even = expand(s, i, i + 1);

        // Update best with odd
        if (odd.second - odd.first > end - start)
        {
            start = odd.first; end = odd.second;
        }
        // Update best with even
        if (even.second - even.first > end - start)
        {
            start = even.first; end = even.second;
        }
    }

    return s.substr(start, end - start + 1);
}

int main()
{
    // Basic examples
    cout << longestPalindrome("babad") << "\n";   // "bab" or "aba"
    cout << longestPalindrome("cbbd") << "\n";    // "bb"

    // Edge cases
    cout << longestPalindrome("a") << "\n";       // "a"
    cout << longestPalindrome("ac") << "\n";      // "a" or "c"
    cout << longestPalindrome("aaaa") << "\n";    // "aaaa"
    return 0;
}
