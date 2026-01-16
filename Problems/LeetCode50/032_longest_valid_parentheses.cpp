/**
 * 032. Longest Valid Parentheses
 *
 * Problem:
 * Given a string containing just '(' and ')', return the length of the longest valid (well-formed) parentheses substring.
 *
 * Intuition:
 * Use a stack of indices to track boundaries of valid substrings. Push '(' indices; for ')', pop. A base index (initially -1) marks the position
 * before a potential valid substring starts. When stack isn't empty after popping, the current valid length is i - stack.top(). If it becomes empty,
 * reset the base by pushing current i.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(n)
 */

#include <iostream>
#include <stack>
#include <string>
#include <algorithm> // for std::max
using namespace std;

int longestValidParentheses(const string& s) {
    stack<int> st;
    st.push(-1); // base index before the start of a valid substring
    int best = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] == '(') {
            st.push(i); // mark position of potential opener
        } else {
            st.pop(); // match this ')' with the last '('
            if (st.empty()) {
                // no matching '(', set new base for future valid substrings
                st.push(i);
            } else {
                // valid substring length from last unmatched index to i
                best = max(best, i - st.top());
            }
        }
    }
    return best;
}

int main() {
    cout << longestValidParentheses("(()") << "\n";      // 2
    cout << longestValidParentheses(")()())") << "\n";  // 4
    cout << longestValidParentheses("") << "\n";        // 0
    cout << longestValidParentheses("()(()") << "\n";    // 2
    return 0;
}
