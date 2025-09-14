#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <string>
#include <chrono>

/**
 * Depth-First Search (DFS) in Directed Graph Implementation
 * 
 * A comprehensive demonstration of DFS algorithm and its applications in directed graphs.
 * DFS in directed graphs enables powerful algorithms like topological sorting,
 * strongly connected components detection, and cycle detection in DAGs.
 * 
 * Time Complexity: O(V + E) where V = vertices, E = edges
 * Space Complexity: O(V) for visited array and recursion stack
 * 
 * Applications demonstrated:
 * - Basic directed graph traversal
 * - Topological sorting (DFS-based)
 * - Strongly Connected Components (Kosaraju's algorithm)
 * - Cycle detection in directed graphs
 * - Path finding in directed graphs
 * - Reachability analysis
 * - Transpose graph operations
 * - Finish time calculation
 */

class DirectedGraph
{
private:
    int numVertices;
    std::vector<std::list<int>> adjacencyList;
    
    // Helper variables for advanced algorithms
    std::vector<bool> visited;
    std::vector<int> discoveryTime;
    std::vector<int> finishTime;
    std::vector<bool> inRecursionStack;  // For cycle detection
    int timer;
    
    /**
     * Recursive DFS traversal helper
     */
    void dfsRecursiveHelper(int vertex, std::vector<bool>& visited, std::vector<int>& traversalOrder)
    {
        visited[vertex] = true;
        traversalOrder.push_back(vertex);
        
        std::cout << "Visiting vertex " << vertex << std::endl;
        
        // Visit all adjacent vertices (following directed edges)
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                std::cout << "  Moving from " << vertex << " to " << neighbor << std::endl;
                dfsRecursiveHelper(neighbor, visited, traversalOrder);
            }
            else
            {
                std::cout << "  Vertex " << neighbor << " already visited (from " << vertex << ")" << std::endl;
            }
        }
        
        std::cout << "Finished exploring from vertex " << vertex << std::endl;
    }
    
    /**
     * DFS for topological sorting with finish time tracking
     */
    void dfsTopologicalHelper(int vertex, std::vector<bool>& visited, std::stack<int>& topologicalStack)
    {
        visited[vertex] = true;
        
        // Visit all adjacent vertices first
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                dfsTopologicalHelper(neighbor, visited, topologicalStack);
            }
        }
        
        // Add current vertex to stack after all descendants are processed
        topologicalStack.push(vertex);
        std::cout << "Added vertex " << vertex << " to topological order" << std::endl;
    }
    
    /**
     * DFS for cycle detection using recursion stack
     */
    bool dfsCycleHelper(int vertex, std::vector<bool>& visited, std::vector<bool>& recStack, std::vector<int>& path)
    {
        visited[vertex] = true;
        recStack[vertex] = true;
        path.push_back(vertex);
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                if (dfsCycleHelper(neighbor, visited, recStack, path))
                {
                    return true;
                }
            }
            else if (recStack[neighbor])
            {
                // Found back edge - cycle detected
                std::cout << "Cycle detected! Back edge from " << vertex << " to " << neighbor << std::endl;
                std::cout << "Cycle path: ";
                
                // Find the start of cycle in path
                auto cycleStart = std::find(path.begin(), path.end(), neighbor);
                for (auto it = cycleStart; it != path.end(); ++it)
                {
                    std::cout << *it << " -> ";
                }
                std::cout << neighbor << std::endl;
                
                return true;
            }
        }
        
        recStack[vertex] = false;
        path.pop_back();
        return false;
    }
    
    /**
     * DFS for path finding in directed graph
     */
    bool dfsPathHelper(int current, int destination, std::vector<bool>& visited, std::vector<int>& path)
    {
        visited[current] = true;
        path.push_back(current);
        
        if (current == destination)
        {
            return true;  // Found destination
        }
        
        for (int neighbor : adjacencyList[current])
        {
            if (!visited[neighbor])
            {
                if (dfsPathHelper(neighbor, destination, visited, path))
                {
                    return true;
                }
            }
        }
        
        path.pop_back();  // Backtrack
        return false;
    }
    
    /**
     * DFS for strongly connected components (first pass)
     */
    void dfsFinishTimeHelper(int vertex, std::vector<bool>& visited, std::stack<int>& finishStack)
    {
        visited[vertex] = true;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                dfsFinishTimeHelper(neighbor, visited, finishStack);
            }
        }
        
        finishStack.push(vertex);  // Add to finish stack when done
    }
    
    /**
     * DFS for strongly connected components (second pass on transpose)
     */
    void dfsSCCHelper(int vertex, std::vector<bool>& visited, std::vector<int>& component, 
                      const std::vector<std::list<int>>& transposeAdj)
    {
        visited[vertex] = true;
        component.push_back(vertex);
        
        for (int neighbor : transposeAdj[vertex])
        {
            if (!visited[neighbor])
            {
                dfsSCCHelper(neighbor, visited, component, transposeAdj);
            }
        }
    }
    
    /**
     * DFS for reachability analysis
     */
    void dfsReachabilityHelper(int vertex, std::vector<bool>& visited, std::vector<int>& reachable)
    {
        visited[vertex] = true;
        reachable.push_back(vertex);
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                dfsReachabilityHelper(neighbor, visited, reachable);
            }
        }
    }
    
    /**
     * DFS with timestamp tracking
     */
    void dfsTimestampHelper(int vertex, std::vector<bool>& visited)
    {
        visited[vertex] = true;
        discoveryTime[vertex] = ++timer;
        
        std::cout << "Discovered vertex " << vertex << " at time " << discoveryTime[vertex] << std::endl;
        
        for (int neighbor : adjacencyList[vertex])
        {
            if (!visited[neighbor])
            {
                dfsTimestampHelper(neighbor, visited);
            }
        }
        
        finishTime[vertex] = ++timer;
        std::cout << "Finished vertex " << vertex << " at time " << finishTime[vertex] << std::endl;
    }

public:
    /**
     * Constructor
     */
    DirectedGraph(int vertices)
    {
        numVertices = vertices;
        adjacencyList.resize(vertices);
        
        // Initialize helper arrays
        visited.resize(vertices);
        discoveryTime.resize(vertices);
        finishTime.resize(vertices);
        inRecursionStack.resize(vertices);
        timer = 0;
    }
    
    /**
     * Add a directed edge from u to v
     */
    void addEdge(int u, int v)
    {
        adjacencyList[u].push_back(v);
        std::cout << "Added directed edge: " << u << " -> " << v << std::endl;
    }
    
    /**
     * Remove a directed edge from u to v
     */
    void removeEdge(int u, int v)
    {
        adjacencyList[u].remove(v);
        std::cout << "Removed directed edge: " << u << " -> " << v << std::endl;
    }
    
    /**
     * Print graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== Directed Graph Adjacency List ===" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << " -> ";
            for (int neighbor : adjacencyList[i])
            {
                std::cout << neighbor << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * Basic DFS traversal (recursive)
     */
    std::vector<int> dfsRecursive(int startVertex)
    {
        std::cout << "\n=== DFS Recursive Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> traversalOrder;
        
        dfsRecursiveHelper(startVertex, visited, traversalOrder);
        
        std::cout << "DFS order: ";
        for (int vertex : traversalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return traversalOrder;
    }
    
    /**
     * DFS traversal using explicit stack (iterative)
     */
    std::vector<int> dfsIterative(int startVertex)
    {
        std::cout << "\n=== DFS Iterative Traversal from vertex " << startVertex << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> traversalOrder;
        std::stack<int> stack;
        
        stack.push(startVertex);
        
        while (!stack.empty())
        {
            int current = stack.top();
            stack.pop();
            
            if (!visited[current])
            {
                visited[current] = true;
                traversalOrder.push_back(current);
                std::cout << "Visiting vertex " << current << std::endl;
                
                // Add neighbors to stack (in reverse order for same traversal as recursive)
                std::vector<int> neighbors;
                for (int neighbor : adjacencyList[current])
                {
                    if (!visited[neighbor])
                    {
                        neighbors.push_back(neighbor);
                    }
                }
                
                std::reverse(neighbors.begin(), neighbors.end());
                for (int neighbor : neighbors)
                {
                    stack.push(neighbor);
                }
            }
        }
        
        std::cout << "DFS order: ";
        for (int vertex : traversalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return traversalOrder;
    }
    
    /**
     * Topological sorting using DFS
     */
    std::vector<int> topologicalSort()
    {
        std::cout << "\n=== Topological Sorting using DFS ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::stack<int> topologicalStack;
        
        // Perform DFS from all unvisited vertices
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                std::cout << "Starting DFS from vertex " << i << std::endl;
                dfsTopologicalHelper(i, visited, topologicalStack);
            }
        }
        
        // Convert stack to vector
        std::vector<int> topologicalOrder;
        while (!topologicalStack.empty())
        {
            topologicalOrder.push_back(topologicalStack.top());
            topologicalStack.pop();
        }
        
        std::cout << "Topological order: ";
        for (int vertex : topologicalOrder)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        
        return topologicalOrder;
    }
    
    /**
     * Detect cycle in directed graph using DFS
     */
    bool hasCycle()
    {
        std::cout << "\n=== Cycle Detection in Directed Graph ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<bool> recStack(numVertices, false);
        std::vector<int> path;
        
        // Check each component
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                if (dfsCycleHelper(i, visited, recStack, path))
                {
                    std::cout << "Directed graph contains a cycle." << std::endl;
                    return true;
                }
            }
        }
        
        std::cout << "Directed graph is acyclic (DAG)." << std::endl;
        return false;
    }
    
    /**
     * Find path between two vertices using DFS
     */
    std::vector<int> findPath(int source, int destination)
    {
        std::cout << "\n=== Finding Path from " << source << " to " << destination << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> path;
        
        if (dfsPathHelper(source, destination, visited, path))
        {
            std::cout << "Path found: ";
            for (int i = 0; i < path.size(); i++)
            {
                std::cout << path[i];
                if (i < path.size() - 1) std::cout << " -> ";
            }
            std::cout << std::endl;
            std::cout << "Path length: " << path.size() - 1 << " edges" << std::endl;
        }
        else
        {
            std::cout << "No path exists from " << source << " to " << destination << std::endl;
        }
        
        return path;
    }
    
    /**
     * Create transpose (reverse) of the directed graph
     */
    DirectedGraph createTranspose() const
    {
        std::cout << "\n=== Creating Transpose Graph ===" << std::endl;
        
        DirectedGraph transpose(numVertices);
        
        for (int u = 0; u < numVertices; u++)
        {
            for (int v : adjacencyList[u])
            {
                transpose.adjacencyList[v].push_back(u);  // Reverse the edge
                std::cout << "Reversed edge: " << u << " -> " << v << " becomes " << v << " -> " << u << std::endl;
            }
        }
        
        return transpose;
    }
    
    /**
     * Find strongly connected components using Kosaraju's algorithm
     */
    std::vector<std::vector<int>> findStronglyConnectedComponents()
    {
        std::cout << "\n=== Finding Strongly Connected Components (Kosaraju's Algorithm) ===" << std::endl;
        
        // Step 1: Perform DFS on original graph and store vertices by finish time
        std::vector<bool> visited(numVertices, false);
        std::stack<int> finishStack;
        
        std::cout << "Step 1: DFS on original graph" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            if (!visited[i])
            {
                dfsFinishTimeHelper(i, visited, finishStack);
            }
        }
        
        // Step 2: Create transpose graph
        DirectedGraph transpose = createTranspose();
        
        // Step 3: Perform DFS on transpose graph in order of decreasing finish time
        std::cout << "\nStep 3: DFS on transpose graph" << std::endl;
        std::fill(visited.begin(), visited.end(), false);
        std::vector<std::vector<int>> sccs;
        
        while (!finishStack.empty())
        {
            int vertex = finishStack.top();
            finishStack.pop();
            
            if (!visited[vertex])
            {
                std::vector<int> component;
                dfsSCCHelper(vertex, visited, component, transpose.adjacencyList);
                sccs.push_back(component);
                
                std::cout << "SCC " << sccs.size() << ": ";
                for (int v : component)
                {
                    std::cout << v << " ";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "Total strongly connected components: " << sccs.size() << std::endl;
        return sccs;
    }
    
    /**
     * Find all vertices reachable from a given vertex
     */
    std::vector<int> findReachableVertices(int source)
    {
        std::cout << "\n=== Finding Vertices Reachable from " << source << " ===" << std::endl;
        
        std::vector<bool> visited(numVertices, false);
        std::vector<int> reachable;
        
        dfsReachabilityHelper(source, visited, reachable);
        
        std::cout << "Vertices reachable from " << source << ": ";
        for (int vertex : reachable)
        {
            std::cout << vertex << " ";
        }
        std::cout << std::endl;
        std::cout << "Total reachable vertices: " << reachable.size() << std::endl;
        
        return reachable;
    }
    
    /**
     * DFS with discovery and finish times
     */
    void dfsWithTimestamps(int startVertex)
    {
        std::cout << "\n=== DFS with Discovery and Finish Times from vertex " << startVertex << " ===" << std::endl;
        
        // Reset arrays
        std::fill(visited.begin(), visited.end(), false);
        std::fill(discoveryTime.begin(), discoveryTime.end(), 0);
        std::fill(finishTime.begin(), finishTime.end(), 0);
        timer = 0;
        
        dfsTimestampHelper(startVertex, visited);
        
        // Print timestamp summary
        std::cout << "\nTimestamp Summary:" << std::endl;
        std::cout << "Vertex\tDiscovery\tFinish" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            if (discoveryTime[i] > 0)  // Was visited
            {
                std::cout << i << "\t" << discoveryTime[i] << "\t\t" << finishTime[i] << std::endl;
            }
        }
    }
    
    /**
     * Check if the graph is a DAG (Directed Acyclic Graph)
     */
    bool isDAG()
    {
        std::cout << "\n=== Checking if Graph is a DAG ===" << std::endl;
        
        bool acyclic = !hasCycle();
        
        if (acyclic)
        {
            std::cout << "Graph is a DAG (Directed Acyclic Graph)." << std::endl;
            std::cout << "Topological sorting is possible." << std::endl;
        }
        else
        {
            std::cout << "Graph is not a DAG - contains cycles." << std::endl;
            std::cout << "Topological sorting is not possible." << std::endl;
        }
        
        return acyclic;
    }
    
    /**
     * Count strongly connected components
     */
    int countSCCs()
    {
        auto sccs = findStronglyConnectedComponents();
        return sccs.size();
    }
    
    /**
     * Check if graph is strongly connected
     */
    bool isStronglyConnected()
    {
        std::cout << "\n=== Checking Strong Connectivity ===" << std::endl;
        
        int sccCount = countSCCs();
        bool stronglyConnected = (sccCount == 1);
        
        if (stronglyConnected)
        {
            std::cout << "Graph is strongly connected." << std::endl;
        }
        else
        {
            std::cout << "Graph is not strongly connected (" << sccCount << " SCCs)." << std::endl;
        }
        
        return stronglyConnected;
    }
    
    /**
     * Get graph statistics
     */
    void printStatistics() const
    {
        std::cout << "\n=== Directed Graph Statistics ===" << std::endl;
        
        int totalEdges = 0;
        int maxOutDegree = 0;
        int maxInDegree = 0;
        std::vector<int> inDegree(numVertices, 0);
        
        for (int i = 0; i < numVertices; i++)
        {
            int outDeg = adjacencyList[i].size();
            totalEdges += outDeg;
            maxOutDegree = std::max(maxOutDegree, outDeg);
            
            for (int neighbor : adjacencyList[i])
            {
                inDegree[neighbor]++;
            }
        }
        
        for (int inDeg : inDegree)
        {
            maxInDegree = std::max(maxInDegree, inDeg);
        }
        
        std::cout << "Number of vertices: " << numVertices << std::endl;
        std::cout << "Number of directed edges: " << totalEdges << std::endl;
        std::cout << "Maximum out-degree: " << maxOutDegree << std::endl;
        std::cout << "Maximum in-degree: " << maxInDegree << std::endl;
        
        if (numVertices > 0)
        {
            double avgOutDegree = (double)totalEdges / numVertices;
            std::cout << "Average out-degree: " << std::fixed << std::setprecision(2) << avgOutDegree << std::endl;
            
            double density = (double)totalEdges / (numVertices * (numVertices - 1));
            std::cout << "Graph density: " << std::fixed << std::setprecision(4) << density << std::endl;
        }
    }
};

// Demonstration and Testing
int main()
{
    std::cout << "=== DEPTH-FIRST SEARCH IN DIRECTED GRAPH DEMONSTRATION ===" << std::endl;
    
    // Test Case 1: Basic DFS Traversal
    std::cout << "\n1. BASIC DFS TRAVERSAL" << std::endl;
    
    DirectedGraph graph1(6);
    graph1.addEdge(0, 1);
    graph1.addEdge(0, 2);
    graph1.addEdge(1, 3);
    graph1.addEdge(2, 4);
    graph1.addEdge(3, 5);
    graph1.addEdge(4, 5);
    
    graph1.printGraph();
    
    // Compare recursive and iterative DFS
    auto recursiveOrder = graph1.dfsRecursive(0);
    auto iterativeOrder = graph1.dfsIterative(0);
    
    graph1.dfsWithTimestamps(0);
    graph1.printStatistics();
    
    // Test Case 2: Topological Sorting
    std::cout << "\n\n2. TOPOLOGICAL SORTING" << std::endl;
    
    DirectedGraph dag(6);
    dag.addEdge(5, 2);
    dag.addEdge(5, 0);
    dag.addEdge(4, 0);
    dag.addEdge(4, 1);
    dag.addEdge(2, 3);
    dag.addEdge(3, 1);
    
    std::cout << "Directed Acyclic Graph (DAG):" << std::endl;
    dag.printGraph();
    dag.isDAG();
    auto topOrder = dag.topologicalSort();
    
    // Test Case 3: Cycle Detection
    std::cout << "\n\n3. CYCLE DETECTION" << std::endl;
    
    // Acyclic directed graph
    DirectedGraph acyclic(4);
    acyclic.addEdge(0, 1);
    acyclic.addEdge(1, 2);
    acyclic.addEdge(2, 3);
    
    std::cout << "Acyclic directed graph:" << std::endl;
    acyclic.printGraph();
    acyclic.hasCycle();
    
    // Cyclic directed graph
    DirectedGraph cyclic(3);
    cyclic.addEdge(0, 1);
    cyclic.addEdge(1, 2);
    cyclic.addEdge(2, 0);  // Creates cycle
    
    std::cout << "\nCyclic directed graph:" << std::endl;
    cyclic.printGraph();
    cyclic.hasCycle();
    
    // Test Case 4: Path Finding
    std::cout << "\n\n4. PATH FINDING" << std::endl;
    
    DirectedGraph pathGraph(5);
    pathGraph.addEdge(0, 1);
    pathGraph.addEdge(1, 2);
    pathGraph.addEdge(0, 3);
    pathGraph.addEdge(3, 4);
    pathGraph.addEdge(2, 4);
    
    pathGraph.printGraph();
    pathGraph.findPath(0, 4);
    pathGraph.findPath(1, 3);  // No path
    pathGraph.findPath(0, 2);
    
    // Test Case 5: Strongly Connected Components
    std::cout << "\n\n5. STRONGLY CONNECTED COMPONENTS" << std::endl;
    
    DirectedGraph sccGraph(8);
    // SCC 1: {0, 1, 2}
    sccGraph.addEdge(0, 1);
    sccGraph.addEdge(1, 2);
    sccGraph.addEdge(2, 0);
    
    // SCC 2: {3, 4}
    sccGraph.addEdge(3, 4);
    sccGraph.addEdge(4, 3);
    
    // SCC 3: {5}
    // SCC 4: {6, 7}
    sccGraph.addEdge(6, 7);
    sccGraph.addEdge(7, 6);
    
    // Connections between SCCs
    sccGraph.addEdge(2, 3);
    sccGraph.addEdge(4, 5);
    sccGraph.addEdge(5, 6);
    
    sccGraph.printGraph();
    auto sccs = sccGraph.findStronglyConnectedComponents();
    sccGraph.isStronglyConnected();
    
    // Test Case 6: Reachability Analysis
    std::cout << "\n\n6. REACHABILITY ANALYSIS" << std::endl;
    
    DirectedGraph reachGraph(6);
    reachGraph.addEdge(0, 1);
    reachGraph.addEdge(0, 2);
    reachGraph.addEdge(1, 3);
    reachGraph.addEdge(2, 4);
    reachGraph.addEdge(4, 5);
    
    reachGraph.printGraph();
    reachGraph.findReachableVertices(0);
    reachGraph.findReachableVertices(1);
    reachGraph.findReachableVertices(4);
    
    // Test Case 7: Transpose Graph
    std::cout << "\n\n7. TRANSPOSE GRAPH CREATION" << std::endl;
    
    DirectedGraph original(4);
    original.addEdge(0, 1);
    original.addEdge(1, 2);
    original.addEdge(2, 3);
    original.addEdge(3, 0);
    
    std::cout << "Original graph:" << std::endl;
    original.printGraph();
    
    DirectedGraph transpose = original.createTranspose();
    std::cout << "Transpose graph:" << std::endl;
    transpose.printGraph();
    
    // Test Case 8: Complex DAG for Topological Sorting
    std::cout << "\n\n8. COMPLEX DAG ANALYSIS" << std::endl;
    
    DirectedGraph complexDAG(7);
    // Dependencies: think of this as a project dependency graph
    complexDAG.addEdge(0, 1);  // Task 0 -> Task 1
    complexDAG.addEdge(0, 2);  // Task 0 -> Task 2
    complexDAG.addEdge(1, 3);  // Task 1 -> Task 3
    complexDAG.addEdge(2, 3);  // Task 2 -> Task 3
    complexDAG.addEdge(3, 4);  // Task 3 -> Task 4
    complexDAG.addEdge(1, 5);  // Task 1 -> Task 5
    complexDAG.addEdge(5, 6);  // Task 5 -> Task 6
    complexDAG.addEdge(4, 6);  // Task 4 -> Task 6
    
    std::cout << "Complex DAG (Project Dependencies):" << std::endl;
    complexDAG.printGraph();
    complexDAG.isDAG();
    auto complexTopOrder = complexDAG.topologicalSort();
    complexDAG.printStatistics();
    
    // Test Case 9: Strongly Connected Graph
    std::cout << "\n\n9. STRONGLY CONNECTED GRAPH" << std::endl;
    
    DirectedGraph strongGraph(4);
    // Make it strongly connected
    strongGraph.addEdge(0, 1);
    strongGraph.addEdge(1, 2);
    strongGraph.addEdge(2, 3);
    strongGraph.addEdge(3, 0);
    strongGraph.addEdge(0, 2);  // Additional edge
    strongGraph.addEdge(1, 3);  // Additional edge
    
    strongGraph.printGraph();
    strongGraph.findStronglyConnectedComponents();
    strongGraph.isStronglyConnected();
    
    // Test Case 10: Performance Test
    std::cout << "\n\n10. LARGE DIRECTED GRAPH PERFORMANCE TEST" << std::endl;
    
    DirectedGraph largeGraph(100);
    
    // Create a directed graph with some structure
    for (int i = 0; i < 100; i++)
    {
        int numOutEdges = rand() % 5 + 1;  // 1-5 outgoing edges per vertex
        for (int j = 0; j < numOutEdges; j++)
        {
            int target = (i + rand() % 50 + 1) % 100;  // Directed to later vertices mostly
            largeGraph.addEdge(i, target);
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    largeGraph.dfsRecursive(0);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "DFS on 100-vertex directed graph completed in " << duration.count() << " microseconds" << std::endl;
    
    // Test topological sort performance
    start = std::chrono::high_resolution_clock::now();
    largeGraph.hasCycle();
    end = std::chrono::high_resolution_clock::now();
    
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Cycle detection completed in " << duration.count() << " microseconds" << std::endl;
    
    largeGraph.printStatistics();
    
    std::cout << "\n=== DFS in Directed Graph Demonstration Complete ===" << std::endl;
    std::cout << "Key Insight: DFS in directed graphs enables powerful algorithms like topological" << std::endl;
    std::cout << "sorting, strongly connected components detection, and cycle detection." << std::endl;
    std::cout << "These algorithms are fundamental for dependency analysis, scheduling problems," << std::endl;
    std::cout << "and understanding the structure of directed relationships." << std::endl;
    
    return 0;
}
