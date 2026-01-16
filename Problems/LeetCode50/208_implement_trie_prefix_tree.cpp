/**
 * 208. Implement Trie (Prefix Tree)
 *
 * Problem:
 * Design a Trie with insert, search, and startsWith.
 *
 * Intuition:
 * Each node has up to 26 children and a flag for end-of-word.
 *
 * Complexity:
 * - Insert/Search/Prefix: O(L) where L is word length
 * - Space: O(total characters inserted)
 */

#include <iostream>
#include <string>
#include <array>
using namespace std;

struct Node {
    array<Node*,26> ch{}; bool end = false;
    Node(){ ch.fill(nullptr); }
};

class Trie {
    Node* root;
public:
    Trie(): root(new Node()) {}
    void insert(const string& word) {
        Node* p = root;
        for (char c : word) {
            int i = c - 'a'; if (!p->ch[i]) p->ch[i] = new Node();
            p = p->ch[i];
        }
        p->end = true;
    }
    bool search(const string& word) const {
        const Node* p = root;
        for (char c : word) {
            int i = c - 'a'; if (!p->ch[i]) return false; p = p->ch[i];
        }
        return p->end;
    }
    bool startsWith(const string& prefix) const {
        const Node* p = root;
        for (char c : prefix) {
            int i = c - 'a'; if (!p->ch[i]) return false; p = p->ch[i];
        }
        return true;
    }
};

int main() {
    Trie t; t.insert("apple");
    cout << boolalpha << t.search("apple") << "\n";    // true
    cout << boolalpha << t.search("app") << "\n";      // false
    cout << boolalpha << t.startsWith("app") << "\n";  // true
    t.insert("app");
    cout << boolalpha << t.search("app") << "\n";      // true
    return 0;
}
