#ifndef RecoEcal_EgammaCoreTools_GraphMap_h
#define RecoEcal_EgammaCoreTools_GraphMap_h

#include <vector>
#include <array>
#include <map>
#include <algorithm>

/*
 * Class handling a sparse graph of clusters.
 *
 */

namespace reco {
  
  class GraphMap {
    
  public:
    GraphMap(uint nNodes, const std::vector<uint> &categories);
    ~GraphMap(){};

    void printGraphMap();
    void addNode(const uint &index, const uint &category);
    void addNodes(const std::vector<uint> &indices, const std::vector<uint> &categories);
    void addEdge(const uint &i, const uint &j);
    void setAdjMatrix(const uint &i, const uint &j, const float &score);
    void setAdjMatrixSym(const uint &i, const uint &j, const float &score);

    //Getters
    const std::vector<uint> &getOutEdges(const uint &i) const;
    const std::vector<uint> &getInEdges(const uint &i) const;
    uint getAdjMatrix(const uint &i, const uint &j) const;
    std::vector<float> getAdjMatrixRow(const uint &i) const;
    std::vector<float> getAdjMatrixCol(const uint &j) const;

    enum CollectionStrategy{
      A, // Collect edgesOut ordered by cat1 index
      B, // First order the edgesIn by score, keeping only the highest one
         // Then collect the edgesOut ordered by cat1 index
      C,  // Like B, but while collecting cat1 nodes in other cat1 nodes,
         // Collect also all the nodes connected to the secondary cat1 node.
      D,
      E
    };

    // Output of the collection  [{seed, [list of clusters]}]
    typedef std::vector<std::pair<uint, std::vector<uint>>> GraphOutput;
    typedef std::map<uint, std::vector<uint>> GraphOutputMap;    
    // Apply the collection algorithms
    const GraphOutput & collectNodes(GraphMap::CollectionStrategy strategy, float threshold);
    
  private:
    uint nNodes_;
    // Map with list of indices of nodes for each category
    std::map<uint, std::vector<uint>> nodesCategories_;
    // Count of nodes for each category
    std::map<uint, uint> nodesCount_;
    // Incoming edges, one list for each node (no distinction between type)
    std::vector<std::vector<uint>> edgesIn_;
    // Outcoming edges, one list for each node
    std::vector<std::vector<uint>> edgesOut_;
    // Adjacency matrix (i,j) --> score
    // Rows are interpreted as OUT edges
    // Columns are interpreted as IN edges
    std::map<std::pair<uint, uint>, float> adjMatrix_;

    // Store for the graph collection result
    GraphOutput graphOutput_;
    
    // Functions for the collection strategies
    void collectCascading(float threshold);
    void assignHighestScoreEdge();
    // Return both the output graph with only cat1 nodes and a GraphOutputMap
    // of the collected cat0 nodes from each cat1 one.
    std::pair<GraphOutput, GraphOutputMap> collectSeparately(float threshold);
    void mergeSubGraphs(float threshold, GraphOutput cat1NodesGraph, GraphOutputMap cat0GraphMap);
    void resolveSuperNodesEdges(float threshold);

  };

}  // namespace reco

#endif
