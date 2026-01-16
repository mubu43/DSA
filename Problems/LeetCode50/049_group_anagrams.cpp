/**
 * 049. Group Anagrams
 *
 * Problem (paraphrased, detailed):
 * Given an array of strings, group them so that each group contains strings that are anagrams of each other.
 * Two strings are anagrams if they contain the same characters with the same multiplicities (order irrelevant).
 *
 * Examples:
 * - ["eat","tea","tan","ate","nat","bat"] -> groups like ["eat","tea","ate"], ["tan","nat"], ["bat"].
 * - [""] -> [[""]]
 *
 * Intuition & Approach:
 * - Anagrams share the same multiset of characters. Map each word to a canonical key representing its multiset.
 * - Options for key:
 *   1) Sorted string (simple, O(L log L) per word)
 *   2) 26-count signature for lowercase letters (O(L)) — used here.
 *
 * Complexity:
 * - Time: O(n * L) with counting signature (where L is average string length).
 * - Space: O(n * L) for storing groups and keys (amortized).
 */

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
using namespace std;

// Build a frequency-based signature for lowercase 'a'..'z' strings.
static string signature(const string& s)
{
    array<int,26> cnt{}; // zero-initialized counts
    for (char ch : s) cnt[ch - 'a']++;

    // Serialize counts into a compact key; separators prevent ambiguity
    string key; key.reserve(26 * 3);
    for (int i = 0; i < 26; i++)
    {
        key.push_back('#');
        key += to_string(cnt[i]);
    }
    return key;
}

vector<vector<string>> groupAnagrams(vector<string> strs)
{
    unordered_map<string, vector<string>> mp;

    for (auto& s : strs)
    {
        // For general Unicode or case sensitivity, adjust signature accordingly
        mp[signature(s)].push_back(s);
    }

    vector<vector<string>> res;
    res.reserve(mp.size());
    for (auto& p : mp)
        res.push_back(move(p.second));

    return res;
}

int main()
{
    auto groups = groupAnagrams({"eat","tea","tan","ate","nat","bat"});
    for (auto& g : groups)
    {
        for (auto& w : g) cout << w << ' ';
        cout << "\n";
    }

    cout << "----\n";
    auto groups2 = groupAnagrams({""});
    for (auto& g : groups2)
    {
        for (auto& w : g) cout << '"' << w << '"' << ' ';
        cout << "\n";
    }
    return 0;
}
