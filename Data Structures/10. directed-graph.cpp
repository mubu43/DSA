/**
 * 9. Directed Graph Data Structure
 * 
 * Simple implementation of a directed graph using adjacency list representation.
 * Focuses on the data structure itself - adding vertices, edges, and basic operations
 * without complex algorithms. Demonstrates key differences from undirected graphs.
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Simple Directed Graph implementation using adjacency list
 */
class DirectedGraph
{
private:
    int numVertices;
    std::vector<std::vector<int>> adjacencyList;  // Outgoing edges
    std::vector<std::vector<int>> reverseAdjacencyList;  // Incoming edges (for efficiency)
    
public:
    /**
     * Constructor - initialize graph with given number of vertices
     */
    DirectedGraph(int vertices) : numVertices(vertices), adjacencyList(vertices), reverseAdjacencyList(vertices) {}
    
    /**
     * Add a directed edge from source to destination
     */
    void addEdge(int source, int destination)
    {
        // Validate vertices
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << source << " and " << destination << std::endl;
            return;
        }
        
        // Check if edge already exists
        if (hasEdge(source, destination))
        {
            std::cout << "Warning: Edge " << source << " -> " << destination << " already exists" << std::endl;
            return;
        }
        
        // Add directed edge (only one direction)
        adjacencyList[source].push_back(destination);
        reverseAdjacencyList[destination].push_back(source);
        
        std::cout << "Added directed edge: " << source << " -> " << destination << std::endl;
    }
    
    /**
     * Remove a directed edge from source to destination
     */
    void removeEdge(int source, int destination)
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            std::cout << "Error: Invalid vertices " << source << " and " << destination << std::endl;
            return;
        }
        
        // Remove destination from source's outgoing edges
        auto it1 = std::find(adjacencyList[source].begin(), adjacencyList[source].end(), destination);
        if (it1 != adjacencyList[source].end())
        {
            adjacencyList[source].erase(it1);
        }
        
        // Remove source from destination's incoming edges
        auto it2 = std::find(reverseAdjacencyList[destination].begin(), reverseAdjacencyList[destination].end(), source);
        if (it2 != reverseAdjacencyList[destination].end())
        {
            reverseAdjacencyList[destination].erase(it2);
        }
        
        std::cout << "Removed directed edge: " << source << " -> " << destination << std::endl;
    }
    
    /**
     * Check if a directed edge exists from source to destination
     */
    bool hasEdge(int source, int destination) const
    {
        if (source < 0 || source >= numVertices || destination < 0 || destination >= numVertices)
        {
            return false;
        }
        
        // Check if destination is in source's outgoing edges
        return std::find(adjacencyList[source].begin(), adjacencyList[source].end(), destination) 
               != adjacencyList[source].end();
    }
    
    /**
     * Get all outgoing neighbors of a vertex (vertices this vertex points to)
     */
    std::vector<int> getOutgoingNeighbors(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return adjacencyList[vertex];
    }
    
    /**
     * Get all incoming neighbors of a vertex (vertices that point to this vertex)
     */
    std::vector<int> getIncomingNeighbors(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return {};
        }
        
        return reverseAdjacencyList[vertex];
    }
    
    /**
     * Get out-degree of a vertex (number of outgoing edges)
     */
    int getOutDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return adjacencyList[vertex].size();
    }
    
    /**
     * Get in-degree of a vertex (number of incoming edges)
     */
    int getInDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return reverseAdjacencyList[vertex].size();
    }
    
    /**
     * Get total degree of a vertex (in-degree + out-degree)
     */
    int getTotalDegree(int vertex) const
    {
        if (vertex < 0 || vertex >= numVertices)
        {
            std::cout << "Error: Invalid vertex " << vertex << std::endl;
            return -1;
        }
        
        return getInDegree(vertex) + getOutDegree(vertex);
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
        return edgeCount;  // No division by 2 needed (directed edges)
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
        std::cout << "\n=== DIRECTED GRAPH ===" << std::endl;
        std::cout << "Vertices: " << numVertices << std::endl;
        std::cout << "Edges: " << getEdgeCount() << std::endl;
        
        std::cout << "\nAdjacency List (Outgoing Edges):" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << " -> ";
            
            if (adjacencyList[i].empty())
            {
                std::cout << "(no outgoing edges)";
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
     * Print reverse adjacency list (incoming edges)
     */
    void printReverseGraph() const
    {
        std::cout << "\nReverse Adjacency List (Incoming Edges):" << std::endl;
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << "Vertex " << i << " <- ";
            
            if (reverseAdjacencyList[i].empty())
            {
                std::cout << "(no incoming edges)";
            }
            else
            {
                for (size_t j = 0; j < reverseAdjacencyList[i].size(); j++)
                {
                    if (j > 0) std::cout << ", ";
                    std::cout << reverseAdjacencyList[i][j];
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
        std::cout << "\n=== ALL DIRECTED EDGES ===" << std::endl;
        
        int edgeNum = 1;
        for (int i = 0; i < numVertices; i++)
        {
            for (int neighbor : adjacencyList[i])
            {
                std::cout << "Edge " << edgeNum++ << ": " << i << " -> " << neighbor << std::endl;
            }
        }
        
        if (edgeNum == 1)
        {
            std::cout << "No edges in the graph." << std::endl;
        }
    }
    
    /**
     * Print detailed vertex information
     */
    void printVertexInfo() const
    {
        std::cout << "\n=== VERTEX INFORMATION ===" << std::endl;
        std::cout << "Vertex\tOut-Deg\tIn-Deg\tTotal\tOutgoing\t\tIncoming" << std::endl;
        std::cout << "------\t-------\t------\t-----\t--------\t\t--------" << std::endl;
        
        for (int i = 0; i < numVertices; i++)
        {
            std::cout << i << "\t" << getOutDegree(i) << "\t" << getInDegree(i) 
                     << "\t" << getTotalDegree(i) << "\t";
            
            // Print outgoing neighbors
            const auto& outgoing = getOutgoingNeighbors(i);
            if (outgoing.empty())
            {
                std::cout << "none\t\t\t";
            }
            else
            {
                for (size_t j = 0; j < outgoing.size(); j++)
                {
                    if (j > 0) std::cout << ",";
                    std::cout << outgoing[j];
                }
                std::cout << "\t\t\t";
            }
            
            // Print incoming neighbors
            const auto& incoming = getIncomingNeighbors(i);
            if (incoming.empty())
            {
                std::cout << "none";
            }
            else
            {
                for (size_t j = 0; j < incoming.size(); j++)
                {
                    if (j > 0) std::cout << ",";
                    std::cout << incoming[j];
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
            reverseAdjacencyList[i].clear();
        }
        std::cout << "Graph cleared - all edges removed." << std::endl;
    }
    
    /**
     * Find vertices with no incoming edges (sources)
     */
    std::vector<int> findSources() const
    {
        std::vector<int> sources;
        for (int i = 0; i < numVertices; i++)
        {
            if (getInDegree(i) == 0 && getOutDegree(i) > 0)
            {
                sources.push_back(i);
            }
        }
        return sources;
    }
    
    /**
     * Find vertices with no outgoing edges (sinks)
     */
    std::vector<int> findSinks() const
    {
        std::vector<int> sinks;
        for (int i = 0; i < numVertices; i++)
        {
            if (getOutDegree(i) == 0 && getInDegree(i) > 0)
            {
                sinks.push_back(i);
            }
        }
        return sinks;
    }
    
    /**
     * Find isolated vertices (no incoming or outgoing edges)
     */
    std::vector<int> findIsolated() const
    {
        std::vector<int> isolated;
        for (int i = 0; i < numVertices; i++)
        {
            if (getInDegree(i) == 0 && getOutDegree(i) == 0)
            {
                isolated.push_back(i);
            }
        }
        return isolated;
    }
};

/**
 * Demonstrate directed graph construction and operations
 */
void demonstrateDirectedGraph()
{
    std::cout << "================================================" << std::endl;
    std::cout << "        DIRECTED GRAPH DEMONSTRATION          " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Create a graph with 6 vertices (0 to 5)
    std::cout << "\n--- Creating Directed Graph with 6 vertices ---" << std::endl;
    DirectedGraph graph(6);
    
    // Initially empty graph
    std::cout << "\nInitial empty graph:" << std::endl;
    graph.printGraph();
    
    // Add directed edges to construct a sample graph
    std::cout << "\n--- Adding Directed Edges ---" << std::endl;
    graph.addEdge(0, 1);  // 0 points to 1
    graph.addEdge(0, 2);  // 0 points to 2
    graph.addEdge(1, 3);  // 1 points to 3
    graph.addEdge(2, 1);  // 2 points to 1 (creates different path)
    graph.addEdge(2, 4);  // 2 points to 4
    graph.addEdge(3, 5);  // 3 points to 5
    graph.addEdge(4, 5);  // 4 points to 5
    graph.addEdge(5, 3);  // 5 points to 3 (creates cycle)
    
    // Display the constructed graph
    graph.printGraph();
    graph.printReverseGraph();
    graph.printAllEdges();
    graph.printVertexInfo();
    
    // Test edge operations
    std::cout << "\n--- Testing Edge Operations ---" << std::endl;
    
    // Check if specific edges exist
    std::cout << "Edge 0 -> 1 exists: " << (graph.hasEdge(0, 1) ? "Yes" : "No") << std::endl;
    std::cout << "Edge 1 -> 0 exists: " << (graph.hasEdge(1, 0) ? "Yes" : "No") << std::endl;
    std::cout << "Edge 3 -> 5 exists: " << (graph.hasEdge(3, 5) ? "Yes" : "No") << std::endl;
    std::cout << "Edge 5 -> 3 exists: " << (graph.hasEdge(5, 3) ? "Yes" : "No") << std::endl;
    
    // Try to add duplicate edge
    std::cout << "\nTrying to add duplicate edge 0 -> 1:" << std::endl;
    graph.addEdge(0, 1);
    
    // Remove an edge
    std::cout << "\nRemoving edge 5 -> 3:" << std::endl;
    graph.removeEdge(5, 3);
    graph.printGraph();
    graph.printVertexInfo();
    
    // Add a new edge
    std::cout << "\nAdding new edge 1 -> 4:" << std::endl;
    graph.addEdge(1, 4);
    graph.printGraph();
    graph.printVertexInfo();
    
    // Analyze graph structure
    std::cout << "\n--- Graph Structure Analysis ---" << std::endl;
    
    auto sources = graph.findSources();
    std::cout << "Source vertices (in-degree = 0): ";
    if (sources.empty())
    {
        std::cout << "none";
    }
    else
    {
        for (size_t i = 0; i < sources.size(); i++)
        {
            if (i > 0) std::cout << ", ";
            std::cout << sources[i];
        }
    }
    std::cout << std::endl;
    
    auto sinks = graph.findSinks();
    std::cout << "Sink vertices (out-degree = 0): ";
    if (sinks.empty())
    {
        std::cout << "none";
    }
    else
    {
        for (size_t i = 0; i < sinks.size(); i++)
        {
            if (i > 0) std::cout << ", ";
            std::cout << sinks[i];
        }
    }
    std::cout << std::endl;
    
    auto isolated = graph.findIsolated();
    std::cout << "Isolated vertices: ";
    if (isolated.empty())
    {
        std::cout << "none";
    }
    else
    {
        for (size_t i = 0; i < isolated.size(); i++)
        {
            if (i > 0) std::cout << ", ";
            std::cout << isolated[i];
        }
    }
    std::cout << std::endl;
    
    // Final graph statistics
    std::cout << "\n--- Final Graph Statistics ---" << std::endl;
    std::cout << "Total vertices: " << graph.getVertexCount() << std::endl;
    std::cout << "Total edges: " << graph.getEdgeCount() << std::endl;
    std::cout << "Graph is empty: " << (graph.isEmpty() ? "Yes" : "No") << std::endl;
}

/**
 * Demonstrate different directed graph patterns
 */
void demonstrateDirectedGraphPatterns()
{
    std::cout << "\n================================================" << std::endl;
    std::cout << "       DIFFERENT DIRECTED GRAPH PATTERNS      " << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Pattern 1: Tournament (Complete Directed Graph)
    std::cout << "\n--- Pattern 1: Tournament (Complete Directed) ---" << std::endl;
    DirectedGraph tournament(4);
    
    // Add directed edges between every pair (one direction only)
    tournament.addEdge(0, 1);
    tournament.addEdge(0, 2);
    tournament.addEdge(0, 3);
    tournament.addEdge(1, 2);
    tournament.addEdge(1, 3);
    tournament.addEdge(2, 3);
    
    tournament.printGraph();
    tournament.printVertexInfo();
    
    // Pattern 2: Directed Tree (Rooted Tree)
    std::cout << "\n--- Pattern 2: Directed Tree (Root at 0) ---" << std::endl;
    DirectedGraph tree(5);
    
    // Root 0 points to children 1, 2
    tree.addEdge(0, 1);
    tree.addEdge(0, 2);
    // Node 1 points to child 3
    tree.addEdge(1, 3);
    // Node 2 points to child 4
    tree.addEdge(2, 4);
    
    tree.printGraph();
    tree.printVertexInfo();
    
    // Pattern 3: Directed Cycle
    std::cout << "\n--- Pattern 3: Directed Cycle ---" << std::endl;
    DirectedGraph cycle(4);
    
    // Create cycle: 0 -> 1 -> 2 -> 3 -> 0
    cycle.addEdge(0, 1);
    cycle.addEdge(1, 2);
    cycle.addEdge(2, 3);
    cycle.addEdge(3, 0);
    
    cycle.printGraph();
    cycle.printVertexInfo();
    
    // Pattern 4: Directed Acyclic Graph (DAG)
    std::cout << "\n--- Pattern 4: Directed Acyclic Graph (DAG) ---" << std::endl;
    DirectedGraph dag(5);
    
    // Create DAG with multiple paths
    dag.addEdge(0, 1);
    dag.addEdge(0, 2);
    dag.addEdge(1, 3);
    dag.addEdge(2, 3);
    dag.addEdge(2, 4);
    dag.addEdge(3, 4);
    
    dag.printGraph();
    dag.printVertexInfo();
    
    auto sources = dag.findSources();
    auto sinks = dag.findSinks();
    
    std::cout << "DAG Sources: ";
    for (size_t i = 0; i < sources.size(); i++)
    {
        if (i > 0) std::cout << ", ";
        std::cout << sources[i];
    }
    std::cout << std::endl;
    
    std::cout << "DAG Sinks: ";
    for (size_t i = 0; i < sinks.size(); i++)
    {
        if (i > 0) std::cout << ", ";
        std::cout << sinks[i];
    }
    std::cout << std::endl;
}

/**
 * Main function - demonstrates directed graph data structure
 */
int main()
{
    try 
    {
        demonstrateDirectedGraph();
        demonstrateDirectedGraphPatterns();
    }
    catch (const std::exception& e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
