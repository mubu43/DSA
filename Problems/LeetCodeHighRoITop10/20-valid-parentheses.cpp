/*
 * LeetCode #20: Valid Parentheses
 * Difficulty: Easy
 *
 * Problem:
 * Given a string s containing only the characters '(', ')', '{', '}', '[' and ']',
 * determine if the input string is valid. An input string is valid if:
 *   1. Open brackets are closed by the same type of brackets.
 *   2. Open brackets are closed in the correct order.
 *   3. Every closing bracket has a corresponding open bracket of the same type.
 *
 * Examples:
 *   - "()"      -> true
 *   - "()[]{}"  -> true
 *   - "(]"      -> false
 *   - "([)]"    -> false
 *   - "{[]}"    -> true
 *
 * Time Complexity: O(n), where n is the length of the string.
 * Space Complexity: O(n), for the stack used to track open brackets.
 */

#include <string>
#include <stack>
#include <unordered_map>
#include <iostream>

using namespace std;

class Solution {
public:
    bool isValid(string s) {
        // Stack to keep track of opening brackets
        stack<char> bracketStack;
        
        // Map closing brackets to their corresponding opening brackets
        unordered_map<char, char> bracketMap;
        bracketMap[')'] = '(';
        bracketMap['}'] = '{';
        bracketMap[']'] = '[';
        
        // Process each character in the string
        for (char c : s) {
            // If it's a closing bracket
            if (bracketMap.find(c) != bracketMap.end()) {
                // Check if stack is empty or top doesn't match
                if (bracketStack.empty() || bracketStack.top() != bracketMap[c]) {
                    return false;
                }
                // Pop the matching opening bracket
                bracketStack.pop();
            }
            // If it's an opening bracket
            else {
                // Push onto stack
                bracketStack.push(c);
            }
        }
        
        // Valid if stack is empty (all brackets matched)
        return bracketStack.empty();
    }
};

// Test function
int main() {
    Solution solution;
    
    // Test cases
    vector<string> testCases = {
        "()",        // true
        "()[]{}",    // true
        "(]",        // false
        "([)]",      // false
        "{[]}",      // true
        "",          // true (empty string)
        "(((",       // false
        ")))",       // false
        "({[]})"     // true
    };
    
    cout << "Testing Valid Parentheses:" << endl;
    for (int i = 0; i < testCases.size(); i++) {
        bool result = solution.isValid(testCases[i]);
        cout << "Test " << (i + 1) << ": \"" << testCases[i] 
             << "\" -> " << (result ? "true" : "false") << endl;
    }
    
    return 0;
}
