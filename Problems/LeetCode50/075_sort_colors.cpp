/**
 * 075. Sort Colors
 *
 * Problem:
 * Sort an array of 0s, 1s, and 2s in-place (Dutch National Flag problem).
 *
 * Intuition:
 * Three pointers: low (next 0 position), mid (current), high (next 2 position). Swap and move accordingly.
 *
 * Complexity:
 * - Time: O(n)
 * - Space: O(1)
 */

#include <iostream>
#include <vector>
using namespace std;

void sortColors(vector<int>& a) {
    int low = 0, mid = 0, high = (int)a.size() - 1;
    while (mid <= high) {
        if (a[mid] == 0) {
            swap(a[low++], a[mid++]);
        } else if (a[mid] == 1) {
            mid++;
        } else { // 2
            swap(a[mid], a[high--]);
        }
    }
}

int main() {
    vector<int> a{2,0,2,1,1,0};
    sortColors(a);
    for (int x : a) cout << x << ' '; cout << '\n'; // 0 0 1 1 2 2
    return 0;
}
