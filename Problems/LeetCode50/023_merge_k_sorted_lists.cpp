/**
 * 023. Merge k Sorted Lists
 *
 * Problem (paraphrased, detailed):
 * Merge k sorted linked lists into a single sorted list and return its head.
 *
 * Examples:
 * - [[1,4,5],[1,3,4],[2,6]] -> [1,1,2,3,4,4,5,6]
 *
 * Intuition:
 * - Always pick the smallest head among the k lists. A min-heap efficiently tracks the current minimum.
 *
 * Algorithm (Min-Heap):
 * - Push non-null list heads into a min-heap.
 * - Pop top (smallest), append to output, and push its next if exists.
 * - Repeat until heap is empty.
 *
 * Complexity:
 * - Time: O(N log k) where N is total number of nodes
 * - Space: O(k) for the heap
 */

#include <iostream>
#include <vector>
#include <queue>
using namespace std;

struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x) : val(x), next(nullptr) {}
};

// Helpers
ListNode* build(const vector<int>& v) {
    ListNode dummy(0);
    ListNode* cur = &dummy;
    for (int x : v) { cur->next = new ListNode(x); cur = cur->next; }
    return dummy.next;
}

void print(ListNode* head) {
    for (ListNode* p = head; p; p = p->next) cout << p->val << (p->next?"->":"\n");
}

ListNode* mergeKLists(vector<ListNode*>& lists) {
    // Custom comparator for a min-heap of ListNode*
    struct Cmp { bool operator()(ListNode* a, ListNode* b) const { return a->val > b->val; } };
    priority_queue<ListNode*, vector<ListNode*>, Cmp> pq;

    // Initialize heap with all non-null heads
    for (auto* l : lists) if (l) pq.push(l);

    ListNode dummy(0), *tail = &dummy;

    // Extract-min loop
    while (!pq.empty()) {
        ListNode* node = pq.top(); pq.pop();
        tail->next = node; tail = tail->next;
        if (node->next) pq.push(node->next);
    }
    return dummy.next;
}

int main() {
    vector<ListNode*> lists;
    lists.push_back(build({1,4,5}));
    lists.push_back(build({1,3,4}));
    lists.push_back(build({2,6}));
    ListNode* merged = mergeKLists(lists);
    print(merged); // 1->1->2->3->4->4->5->6
    return 0;
}
