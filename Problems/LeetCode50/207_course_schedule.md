# 207. Course Schedule

## Problem
Given `numCourses` and prerequisites `prerequisites[i] = [a, b]` meaning you must take `b` before `a`, determine if it's possible to finish all courses.

## Examples
- num=2, prereq=[[1,0]] → true
- num=2, prereq=[[1,0],[0,1]] → false

## Intuition
Build a directed graph and detect cycles using Kahn's algorithm (BFS topological sort). If we can process all nodes (in-degree becomes zero), the graph is acyclic.

## Algorithm (Kahn's BFS)
1. Build adjacency list and in-degree array.
2. Push all nodes with in-degree 0 into the queue.
3. Pop nodes, decrement in-degree of neighbors, push newly zero in-degree nodes.
4. If processed count equals `numCourses`, return true; else false.

## Correctness
- In-degree elimination simulates removing edges. Presence of a cycle leaves some nodes with positive in-degree.

## Complexity
- Time: O(V+E)
- Space: O(V+E)

## Edge Cases
- No prerequisites → always true.
- Multiple independent chains.

## References
- Topological sorting
