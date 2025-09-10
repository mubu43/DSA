/*
 * LeetCode #21: Merge Two Sorted Lists
 * Difficulty: Easy
 * 
 * Problem: You are given the heads of two sorted linked lists list1 and list2.
 * Merge the two lists in a one sorted list. The list should be made by splicing 
 * together the nodes of the first two lists.
 * 
 * Time Complexity: O(m + n)
 * Space Complexity: O(1)
 */

#include <iostream>

using namespace std;

// Definition for singly-linked list
struct ListNode {
    int val;
    ListNode *next;
    
    // Constructor
    ListNode() {
        val = 0;
        next = nullptr;
    }
    
    ListNode(int x) {
        val = x;
        next = nullptr;
    }
    
    ListNode(int x, ListNode *next) {
        val = x;
        this->next = next;
    }
};

class Solution {
public:
    ListNode* mergeTwoLists(ListNode* list1, ListNode* list2) {
        // Create a dummy head to simplify edge cases
        ListNode* dummy = new ListNode(0);
        ListNode* current = dummy;
        
        // Merge while both lists have nodes
        while (list1 != nullptr && list2 != nullptr) {
            if (list1->val <= list2->val) {
                current->next = list1;
                list1 = list1->next;
            } else {
                current->next = list2;
                list2 = list2->next;
            }
            current = current->next;
        }
        
        // Append remaining nodes from either list
        if (list1 != nullptr) {
            current->next = list1;
        } else {
            current->next = list2;
        }
        
        // Store result and clean up dummy node
        ListNode* result = dummy->next;
        delete dummy;
        return result;
    }
};

// Helper functions for testing
ListNode* createList(vector<int> vals) {
    if (vals.empty()) return nullptr;
    
    ListNode* head = new ListNode(vals[0]);
    ListNode* current = head;
    
    for (int i = 1; i < vals.size(); i++) {
        current->next = new ListNode(vals[i]);
        current = current->next;
    }
    
    return head;
}

void printList(ListNode* head) {
    while (head != nullptr) {
        cout << head->val;
        if (head->next != nullptr) cout << " -> ";
        head = head->next;
    }
    cout << endl;
}

void deleteList(ListNode* head) {
    while (head != nullptr) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// Test function
int main() {
    Solution solution;
    
    // Test case 1: [1,2,4] and [1,3,4]
    ListNode* list1 = createList({1, 2, 4});
    ListNode* list2 = createList({1, 3, 4});
    
    cout << "List 1: ";
    printList(list1);
    cout << "List 2: ";
    printList(list2);
    
    ListNode* merged = solution.mergeTwoLists(list1, list2);
    cout << "Merged: ";
    printList(merged);
    
    deleteList(merged);
    
    // Test case 2: [] and []
    ListNode* list3 = nullptr;
    ListNode* list4 = nullptr;
    ListNode* merged2 = solution.mergeTwoLists(list3, list4);
    cout << "\nEmpty lists merged: ";
    printList(merged2);
    
    // Test case 3: [] and [0]
    ListNode* list5 = nullptr;
    ListNode* list6 = createList({0});
    ListNode* merged3 = solution.mergeTwoLists(list5, list6);
    cout << "Empty and [0] merged: ";
    printList(merged3);
    
    deleteList(merged3);
    
    return 0;
}
