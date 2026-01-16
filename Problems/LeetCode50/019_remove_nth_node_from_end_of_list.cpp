/**
 * 019. Remove Nth Node From End of List
 *
 * Problem (paraphrased, detailed):
 * Given the head of a singly linked list, remove the nth node from the end of the list
 * and return the list head.
 *
 * Examples:
 * - [1,2,3,4,5], n=2 -> [1,2,3,5]
 * - [1], n=1 -> []
 * - [1,2], n=2 -> [2]
 *
 * Intuition:
 * - Use two pointers spaced n nodes apart. When the leading pointer reaches the end,
 *   the trailing pointer sits just before the node to delete.
 *
 * Algorithm (Two Pointers with Dummy Head):
 * - Attach a dummy node before head to simplify removing the original head.
 * - Move fast pointer n+1 steps ahead.
 * - Move fast and slow together until fast reaches null.
 * - Delete slow->next and relink.
 *
 * Complexity:
 * - Time: O(L), L = list length
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x) : val(x), next(nullptr) {}
};

// Helper to build a list from a vector
ListNode* build(const vector<int>& v) {
    ListNode dummy(0);
    ListNode* cur = &dummy;
    for (int x : v) {
        cur->next = new ListNode(x);
        cur = cur->next;
    }
    return dummy.next;
}

// Helper to print the list
void print(ListNode* head) {
    for (ListNode* p = head; p; p = p->next) {
        cout << p->val << (p->next ? "->" : "\n");
    }
}

ListNode* removeNthFromEnd(ListNode* head, int n) {
    // Dummy node simplifies edge cases (e.g., removing the head)
    ListNode dummy(0);
    dummy.next = head;
    ListNode* fast = &dummy;
    ListNode* slow = &dummy;

    // Advance fast by n+1 steps so slow lands just before target
    for (int i = 0; i < n + 1; i++) {
        fast = fast->next;
    }

    // March both pointers until fast hits the end
    while (fast) {
        fast = fast->next;
        slow = slow->next;
    }

    // Remove the node after slow
    ListNode* toDelete = slow->next;
    slow->next = slow->next ? slow->next->next : nullptr;
    delete toDelete; // free memory (optional on platforms with GC)

    return dummy.next;
}

int main() {
    // Basic
    ListNode* h1 = build({1,2,3,4,5});
    h1 = removeNthFromEnd(h1, 2); // remove 4
    print(h1); // 1->2->3->5

    // Remove head
    ListNode* h2 = build({1});
    h2 = removeNthFromEnd(h2, 1); // becomes empty
    print(h2); // (prints nothing or newline)

    // Remove first of two
    ListNode* h3 = build({1,2});
    h3 = removeNthFromEnd(h3, 2); // remove 1
    print(h3); // 2

    return 0;
}
