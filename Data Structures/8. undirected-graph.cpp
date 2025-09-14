/**
 * 8. Undirected Graph Data Structure
 * 
 * Simple implementation of an undirected graph using adjacency list representation.
 * Focuses on the data structure itself - adding vertices, edges, and basic operations
 * without complex algorithms.
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Simple Undirected Graph implementation using adjacency list
 */
class UndirectedGraph
{
private:
    int numVertices;
    std::vector<std::vector<int>> adjacencyList;
    
public:
    /**
     * Constructor - initialize graph with given number of vertices
     */
    UndirectedGraph(int vertices) : numVertices(vertices), adjacencyList(vertices) {}
    
    /**
     * Add an undirected edge between two vertices
     */
    void addEdge(int vertex1, int vertex2)
    {
        // Validate vertices
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << vertex1 << " and " << vertex2 << std::endl;
            return;
        }
        
        // Avoid self-loops and duplicate edges
        if (vertex1 == vertex2)
        {
            std::cout << "Warning: Self-loop from " << vertex1 << " to itself ignored" << std::endl;
            return;
        }
        
        // Check if edge already exists
        if (hasEdge(vertex1, vertex2))
        {
            std::cout << "Warning: Edge " << vertex1 << " - " << vertex2 << " already exists" << std::endl;
            return;
        }
        
        // Add edge in both directions (undirected)
        adjacencyList[vertex1].push_back(vertex2);
        adjacencyList[vertex2].push_back(vertex1);
        
        std::cout << "Added edge: " << vertex1 << " - " << vertex2 << std::endl;
    }
    
    /**
     * Remove an edge between two vertices
     */
    void removeEdge(int vertex1, int vertex2)
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << vertex1 << " and " << vertex2 << std::endl;
            return;
        }
        
        // Remove vertex2 from vertex1's adjacency list
        auto it1 = std::find(adjacencyList[vertex1].begin(), adjacencyList[vertex1].end(), vertex2);
        if (it1 != adjacencyList[vertex1].end())
        {
            adjacencyList[vertex1].erase(it1);
        }
        
        // Remove vertex1 from vertex2's adjacency list
        auto it2 = std::find(adjacencyList[vertex2].begin(), adjacencyList[vertex2].end(), vertex1);
        if (it2 != adjacencyList[vertex2].end())
        {
            adjacencyList[vertex2].erase(it2);
        }
        
        std::cout << "Removed edge: " << vertex1 << " - " << vertex2 << std::endl;
    }
    
    /**
     * Check if an edge exists between two vertices
     */
    bool hasEdge(int vertex1, int vertex2) const
    {
        if (vertex1 < 0 || vertex1 >= numVertices || vertex2 < 0 || vertex2 >= numVertices)
        {
            return false;
        }
        
        // Check if vertex2 is in vertex1's adjacency list
        return std::find(adjacencyList[vertex1].begin(), adjacencyList[vertex1].end(), vertex2) 
               != adjacencyList[vertex1].end();
    }
    
    /**
     * Get all neighbors of a vertex
     */
    std::vector<int> getNeighbors(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return adjacencyList[vertex];
    }
    
    /**
     * Get degree of a vertex (number of edges connected to it)
     */
    int getDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return adjacencyList[vertex].size();
    }
    
    /**
     * Get total number of edges in the graph
     */
    int getEdgeCount() const
    {
        int edgeCount = 0;
        for (int i = 0; i < numVertices; i++)
        {
            edgeCount += adjacencyList[i].size();
        }
        // Divide by 2 since each edge is counted twice (undirected)
        return edgeCount / 2;
    }
    
    /**
     * Get number of vertices
     */
    int getVertexCount() const
    {
        return numVertices;
    }
    
    /**
     * Print the graph representation
     */
    void printGraph() const
    {
        std::cout << "\n=== UNDIRECTED GRAPH ===" << std::endl;
        std::cout << "Vertices: " << numVertices << std::endl;
        std::cout << "Edges: " << getEdgeCount() << std::endl;
        
        std::cout << "\nAdjacency List Representation:" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << ": ";
            
            if (adjacencyList[i].empty())
            {
                std::cout << "(no connections)";
            }
            else
            {
                for (size_t j = 0; j < adjacencyList[i].size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << adjacencyList[i][j];
                }
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Print all edges in the graph
     */
    void printAllEdges() const
    {
        std::cout << "\n=== ALL EDGES ===" << std::endl;
        
        std::unordered_set<std::string> printedEdges;
        int edgeNum = 1;
        
        for (int i = 0; i < numVertices; i++)
        {
            for (int neighbor : adjacencyList[i])
            {
                // Create edge representation (smaller vertex first to avoid duplicates)
                std::string edge = (i < neighbor) ? 
                    std::to_string(i) + "-" + std::to_string(neighbor) :
                    std::to_string(neighbor) + "-" + std::to_string(i);
                
                // Only print if we haven't seen this edge before
                if (printedEdges.find(edge) == printedEdges.end())
                {
                    std::cout << "Edge " << edgeNum++ << ": " << edge << std::endl;
                    printedEdges.insert(edge);
                }
            }
        }
        
        if (edgeNum == 1)
        {
            std::cout << "No edges in the graph." << std::endl;
        }
    }
    
    /**
     * Print vertex information
     */
    void printVertexInfo() const
    {
        std::cout << "\n=== VERTEX INFORMATION ===" << std::endl;
        std::cout << "Vertex\tDegree\tNeighbors" << std::endl;
        std::cout << "------\t------\t---------" << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << "\t" << getDegree(i) << "\t";
            
            const auto& neighbors = getNeighbors(i);
            if (neighbors.empty())
            {
                std::cout << "none";
            }
            else
            {
                for (size_t j = 0; j < neighbors.size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << neighbors[j];
                }
            }
            std::cout << std::endl;
        }
    }
    
    /**
     * Check if the graph is empty (no edges)
     */
    bool isEmpty() const
    {
        return getEdgeCount() == 0;
    }
    
    /**
     * Clear all edges from the graph
     */
    void clear()
    {
        for (int i = 0; i < numVertices; i++)
        {
            adjacencyList[i].clear();
        }
        std::cout << "Graph cleared - all edges removed." << std::endl;
    }
};

/**
 * Demonstrate undirected graph construction and operations
 */
void demonstrateUndirectedGraph()
{
    std::cout << "================================================" << std::endl;
    std::cout << "       UNDIRECTED GRAPH DEMONSTRATION         " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Create a graph with 6 vertices (0 to 5)
    std::cout << "\n--- Creating Undirected Graph with 6 vertices ---" << std::endl;
    UndirectedGraph graph(6);
    
    // Initially empty graph
    std::cout << "\nInitial empty graph:" << std::endl;
    graph.printGraph();
    
    // Add edges to construct a sample graph
    std::cout << "\n--- Adding Edges ---" << std::endl;
    graph.addEdge(0, 1);  // Connect vertex 0 and 1
    graph.addEdge(0, 2);  // Connect vertex 0 and 2
    graph.addEdge(1, 3);  // Connect vertex 1 and 3
    graph.addEdge(2, 3);  // Connect vertex 2 and 3
    graph.addEdge(2, 4);  // Connect vertex 2 and 4
    graph.addEdge(3, 5);  // Connect vertex 3 and 5
    graph.addEdge(4, 5);  // Connect vertex 4 and 5
    
    // Display the constructed graph
    graph.printGraph();
    graph.printAllEdges();
    graph.printVertexInfo();
    
    // Test edge operations
    std::cout << "\n--- Testing Edge Operations ---" << std::endl;
    
    // Check if specific edges exist
    std::cout << "Edge 0-1 exists: " << (graph.hasEdge(0, 1) ? "Yes" : "No") << std::endl;
    std::cout << "Edge 0-3 exists: " << (graph.hasEdge(0, 3) ? "Yes" : "No") << std::endl;
    std::cout << "Edge 1-5 exists: " << (graph.hasEdge(1, 5) ? "Yes" : "No") << std::endl;
    
    // Try to add duplicate edge
    std::cout << "\nTrying to add duplicate edge 0-1:" << std::endl;
    graph.addEdge(0, 1);
    
    // Try to add self-loop
    std::cout << "\nTrying to add self-loop 2-2:" << std::endl;
    graph.addEdge(2, 2);
    
    // Remove an edge
    std::cout << "\nRemoving edge 2-4:" << std::endl;
    graph.removeEdge(2, 4);
    graph.printGraph();
    
    // Add a new edge
    std::cout << "\nAdding new edge 1-4:" << std::endl;
    graph.addEdge(1, 4);
    graph.printGraph();
    graph.printVertexInfo();
    
    // Test individual vertex operations
    std::cout << "\n--- Individual Vertex Analysis ---" << std::endl;
    for (int i = 0; i < graph.getVertexCount(); i++)
    {
        std::cout << "Vertex " << i << " has degree " << graph.getDegree(i) << " with neighbors: ";
        auto neighbors = graph.getNeighbors(i);
        
        if (neighbors.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (size_t j = 0; j < neighbors.size(); j++)
            {
                if (j > 0) std::cout << ", ";
                std::cout << neighbors[j];
            }
        }
        std::cout << std::endl;
    }
    
    // Final graph statistics
    std::cout << "\n--- Final Graph Statistics ---" << std::endl;
    std::cout << "Total vertices: " << graph.getVertexCount() << std::endl;
    std::cout << "Total edges: " << graph.getEdgeCount() << std::endl;
    std::cout << "Graph is empty: " << (graph.isEmpty() ? "Yes" : "No") << std::endl;
}

/**
 * Demonstrate different graph patterns
 */
void demonstrateGraphPatterns()
{
    std::cout << "\n================================================" << std::endl;
    std::cout << "          DIFFERENT GRAPH PATTERNS            " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Pattern 1: Complete Graph (K4)
    std::cout << "\n--- Pattern 1: Complete Graph K4 ---" << std::endl;
    UndirectedGraph completeGraph(4);
    
    // Connect every vertex to every other vertex
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            completeGraph.addEdge(i, j);
        }
    }
    
    completeGraph.printGraph();
    completeGraph.printVertexInfo();
    
    // Pattern 2: Star Graph
    std::cout << "\n--- Pattern 2: Star Graph ---" << std::endl;
    UndirectedGraph starGraph(5);
    
    // Connect vertex 0 to all other vertices
    for (int i = 1; i < 5; i++)
    {
        starGraph.addEdge(0, i);
    }
    
    starGraph.printGraph();
    starGraph.printVertexInfo();
    
    // Pattern 3: Path Graph
    std::cout << "\n--- Pattern 3: Path Graph ---" << std::endl;
    UndirectedGraph pathGraph(5);
    
    // Connect vertices in sequence: 0-1-2-3-4
    for (int i = 0; i < 4; i++)
    {
        pathGraph.addEdge(i, i + 1);
    }
    
    pathGraph.printGraph();
    pathGraph.printVertexInfo();
    
    // Pattern 4: Cycle Graph
    std::cout << "\n--- Pattern 4: Cycle Graph ---" << std::endl;
    UndirectedGraph cycleGraph(5);
    
    // Connect vertices in cycle: 0-1-2-3-4-0
    for (int i = 0; i < 4; i++)
    {
        cycleGraph.addEdge(i, i + 1);
    }
    cycleGraph.addEdge(4, 0);  // Close the cycle
    
    cycleGraph.printGraph();
    cycleGraph.printVertexInfo();
}

/**
 * Main function - demonstrates undirected graph data structure
 */
int main()
{
    try 
    {
        demonstrateUndirectedGraph();
        demonstrateGraphPatterns();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
