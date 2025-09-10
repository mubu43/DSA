/*
 * LeetCode #242: Valid Anagram
 * Difficulty: Easy
 * 
 * Problem: Given two strings s and t, return true if t is an anagram of s, and false otherwise.
 * An Anagram is a word or phrase formed by rearranging the letters of a different word or phrase,
 * typically using all the original letters exactly once.
 * 
 * Time Complexity: O(n)
 * Space Complexity: O(1) for character array, O(k) for hash map where k is unique characters
 */

#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

class Solution {
public:
    // Approach 1: Character Frequency Array (Optimal for lowercase letters)
    bool isAnagram(string s, string t) {
        // Early return if lengths differ
        if (s.length() != t.length()) {
            return false;
        }
        
        // Count frequency of each character
        vector<int> charCount(26, 0);
        
        // Process both strings simultaneously
        for (int i = 0; i < s.length(); i++) {
            charCount[s[i] - 'a']++;  // Increment for string s
            charCount[t[i] - 'a']--;  // Decrement for string t
        }
        
        // Check if all counts are zero
        for (int count : charCount) {
            if (count != 0) {
                return false;
            }
        }
        
        return true;
    }
    
    // Approach 2: Hash Map (Works for any characters)
    bool isAnagramHashMap(string s, string t) {
        // Early return if lengths differ
        if (s.length() != t.length()) {
            return false;
        }
        
        unordered_map<char, int> charMap;
        
        // Count characters in string s
        for (char c : s) {
            charMap[c]++;
        }
        
        // Decrement count for characters in string t
        for (char c : t) {
            charMap[c]--;
            // If count becomes negative, t has more of this character
            if (charMap[c] < 0) {
                return false;
            }
        }
        
        // Check if all counts are zero
        for (auto& pair : charMap) {
            if (pair.second != 0) {
                return false;
            }
        }
        
        return true;
    }
    
    // Approach 3: Sorting (Less efficient but simple)
    bool isAnagramSorting(string s, string t) {
        // Early return if lengths differ
        if (s.length() != t.length()) {
            return false;
        }
        
        // Sort both strings and compare
        sort(s.begin(), s.end());
        sort(t.begin(), t.end());
        
        return s == t;
    }
    
    // Approach 4: Two separate frequency maps
    bool isAnagramTwoMaps(string s, string t) {
        if (s.length() != t.length()) {
            return false;
        }
        
        unordered_map<char, int> sMap, tMap;
        
        // Count frequencies in both strings
        for (char c : s) {
            sMap[c]++;
        }
        
        for (char c : t) {
            tMap[c]++;
        }
        
        // Compare the maps
        return sMap == tMap;
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<pair<string, string>> testCases = {
        {"anagram", "nagaram"},     // true
        {"rat", "car"},            // false
        {"listen", "silent"},      // true
        {"hello", "bello"},        // false
        {"a", "ab"},               // false (different lengths)
        {"", ""},                  // true (both empty)
        {"aab", "aba"},            // true
        {"abc", "def"},            // false
        {"aabbcc", "abcabc"}       // true
    };
    
    vector<bool> expected = {true, false, true, false, false, true, true, false, true};
    
    cout << "Testing Valid Anagram (Character Array):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        bool result = solution.isAnagram(testCases[i].first, testCases[i].second);
        cout << "Test " << (i + 1) << ": ";
        cout << "\"" << testCases[i].first << "\" & \"" << testCases[i].second << "\" -> ";
        cout << (result ? "true" : "false") << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Valid Anagram (Hash Map):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        bool result = solution.isAnagramHashMap(testCases[i].first, testCases[i].second);
        cout << "Test " << (i + 1) << ": ";
        cout << (result ? "true" : "false") << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    cout << "\nTesting Valid Anagram (Sorting):" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        bool result = solution.isAnagramSorting(testCases[i].first, testCases[i].second);
        cout << "Test " << (i + 1) << ": ";
        cout << (result ? "true" : "false") << ", Expected: " << (expected[i] ? "true" : "false");
        cout << (result == expected[i] ? " ✓" : " ✗") << endl;
    }
    
    return 0;
}
