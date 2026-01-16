/**
 * 206. Reverse Linked List
 *
 * Problem:
 * Reverse a singly linked list and return the new head.
 *
 * Intuition:
 * Iteratively reverse pointers: maintain prev, cur, next.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
using namespace std;

struct ListNode { int val; ListNode* next; ListNode(int v):val(v),next(nullptr){} };

ListNode* reverseList(ListNode* head) {
    ListNode* prev = nullptr; ListNode* cur = head;
    while (cur) {
        ListNode* nxt = cur->next;
        cur->next = prev;
        prev = cur;
        cur = nxt;
    }
    return prev;
}

int main() {
    ListNode* a = new ListNode(1); a->next = new ListNode(2); a->next->next = new ListNode(3);
    ListNode* r = reverseList(a);
    for (ListNode* p = r; p; p = p->next) cout << p->val << (p->next?"->":"\n");
    // 3->2->1
    return 0;
}
