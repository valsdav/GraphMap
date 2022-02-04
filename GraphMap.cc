#include "GraphMap.h"
#include <iostream>
#include <iomanip>

using namespace reco;

GraphMap::GraphMap(uint nNodes, const std::vector<uint> &categories) : nNodes_(nNodes) {
  // One entry for node in the edges_ list
  edgesIn_.resize(nNodes);
  edgesOut_.resize(nNodes);
}

void GraphMap::addNode(const uint &index, const uint &category) {
  nodesCategories_[category].push_back(index);
  nodesCount_[category] += 1;
}

void GraphMap::addNodes(const std::vector<uint> &indeces, const std::vector<uint> &categories) {
  for (size_t i = 0; i < indeces.size(); i++) {
    addNode(indeces.at(i), categories.at(i));
  }
}

void GraphMap::addEdge(const uint &i, const uint &j) {
  // The first index is the starting node of the outcoming edge.
  edgesOut_.at(i).push_back(j);
  edgesIn_.at(j).push_back(i);
  adjMatrix_[{i, j}] = 1.;
}

void GraphMap::setAdjMatrix(const uint &i, const uint &j, const float &score) {
  adjMatrix_[{i, j}] = score;
};

void GraphMap::setAdjMatrixSym(const uint &i, const uint &j, const float &score) {
  adjMatrix_[{i, j}] = score;
  adjMatrix_[{j, i}] = score;
};

//Getters
const std::vector<uint> & GraphMap::getOutEdges(const uint &i) const {
  return edgesOut_.at(i);
};

const std::vector<uint> & GraphMap::getInEdges(const uint &i) const {
  return edgesIn_.at(i);
};

uint GraphMap::getAdjMatrix(const uint &i, const uint &j) const {
  return adjMatrix_.at({i, j});
};

std::vector<float> GraphMap::getAdjMatrixRow(const uint &i) const {
  std::vector<float> out;
  for (const auto &j : getOutEdges(i)) {
    out.push_back(adjMatrix_.at({i, j}));
  }
  return out;
};

std::vector<float> GraphMap::getAdjMatrixCol(const uint &j) const {
  std::vector<float> out;
  for (const auto &i : getInEdges(j)) {
    out.push_back(adjMatrix_.at({i, j}));
  }
  return out;
};

void GraphMap::printGraphMap(){
  std::cout << "OUT edges" << std::endl;
  uint seed = 0;
  for (const auto & s: edgesOut_){
    std::cout << "cl: " << seed << " --> ";
    for(const auto & e: s){
      std::cout << e << " (" << adjMatrix_[{seed,e}] << ") ";
    }
    std::cout << std::endl;
    seed++;
  }
  std::cout << std::endl << "IN edges" << std::endl;
  seed =0;
  for (const auto & s : edgesIn_){
    std::cout << "cl: " << seed << " <-- ";
    for(const auto & e: s){
      std::cout << e << " (" << adjMatrix_[{e,seed}] << ") ";
    }
    std::cout << std::endl;
    seed++;
  }
  std::cout << std::endl << "AdjMatrix" << std::endl;
  for (const auto & s : nodesCategories_[1]){
    for(size_t n=0; n<nNodes_; n++){
      std::cout << std::setprecision(2) <<  adjMatrix_[{s,n}] << " ";
    }
    std::cout << std::endl;
  }
}


//--------------------------------------------------------------

const GraphMap::GraphOutput & GraphMap::collectNodes(GraphMap::CollectionStrategy strategy, float threshold){
  // Clear any stored graph output
  graphOutput_.clear();  
  
  if (strategy == GraphMap::CollectionStrategy::A){
    // Starting from the highest energy seed (cat1), collect all the nodes. 
    // Other seeds collected by higher energy seeds (cat1) are ignored 
    collectCascading(threshold);
  }
  else if (strategy == GraphMap::CollectionStrategy::B){
    // First, for each cat0 node keep only the edge with the highest score.
    // Then proceed as strategy A, from the first cat1 node cascading to the others.
    // Secondary cat1 nodes linked are absorbed and ignored in the next iteration:
    // this implies that nodes connected to these cat1 nodes are lost.
    assignHighestScoreEdge();
    collectCascading(threshold);      
  }
  else if(strategy == GraphMap::CollectionStrategy::C){
    // First, for each cat0 node keep only the edge with the highest score.
    // Then collect all the cat0 nodes around the cat1 seeds.
    // Edges between the cat1 nodes are ignored.
    // Finally, starting from the first cat1 node, look for linked cat1 secondary
    // nodes and if they pass the threshold, merge their noded.
    assignHighestScoreEdge();
    const auto & [cat1graph,cat0map] = collectSeparately(threshold);
    mergeSubGraphs(threshold, cat1graph, cat0map);
  }
  else if(strategy == GraphMap::CollectionStrategy::D){
    // First starting from the first cat1 node resolve all the edges between cat1 nodes/
    // If the score > threshold, disable the creation of a subgraph around the secondary seed.
    // Then proceed as in strategy A
    resolveSuperNodesEdges(threshold);
    collectCascading(threshold);
    
  }
  else if(strategy == GraphMap::CollectionStrategy::E){
    // Like strategy D, but after solving the edges bet
    resolveSuperNodesEdges(threshold);
    assignHighestScoreEdge();
    collectCascading(threshold);
    
  }
  
  
  return graphOutput_;
}

void GraphMap::collectCascading(float threshold){
  // Starting from the highest energy seed (cat1), collect all the nodes. 
  // Other seeds collected by higher energy seeds (cat1) are ignored 
  const auto & superNodes = nodesCategories_[1];
  // superNodes are already included in order
  std::cout << "Cascading..." << std::endl;
  for(const auto & s : superNodes){
    std::cout << "seed: " << s << std::endl;
    std::vector<uint> collectedNodes;
    // Check if the seed if still available
    if (adjMatrix_[{s,s}] < threshold) continue;
    // Loop on the out-coming edges 
    for (const auto & out : edgesOut_[s]){
      // Check the threshold for association
      if (adjMatrix_[{s, out}] >= threshold){
        std::cout << "\tOut edge: " << s << " --> " << out<< std::endl;
        // Save the node
        collectedNodes.push_back(out);
        // Remove all incoming edges to the selected node
        // So that it cannot be taken from other SuperNodes
        for (const auto & out_in : edgesIn_[out] ){
          std::cout << "\t\t Deleted edge: " <<  out << " <-- " << out_in << std::endl;
          // Since there is a self loop on the superNode
          // this will select also other superNodes pointing to the
          // current superNode.
          // Also the self-loop on the correct superNodes is removed. 
          // The edge is removed in the adjacency matrix score
          adjMatrix_[{out_in, out}] = 0.;
        }
        // Remove also any self-loop so that if the out node is superNode,
        // so it cannot be used in the next iteration
        // If out is a cat1 node (seed) this avoids to clean
        // all the edges from seed "out" to others 
        adjMatrix_[{out,out}] = 0.;
      }
    }
    graphOutput_.push_back({s, collectedNodes});
  }
}

void GraphMap::assignHighestScoreEdge(){
  // First for each cat0 node (no seed) keep only the highest score link
  // Then perform strategy A.
  std::cout << "Keep only highest score edge" << std::endl;
  for(const auto & cl : nodesCategories_[0]){
    std::pair<uint, float> maxPair {0,0};
    for(const auto & seed: edgesIn_[cl]){
      float score = adjMatrix_[{seed, cl}];
      if (score > maxPair.second){
        maxPair = {seed, score};
      }
    }
    std::cout << "cluster: " << cl << " highest score edge from " <<
      maxPair.first << "| remove seeds: " ;
    // Second loop to remove all the edges apart from the max
    for(const auto & seed: edgesIn_[cl]){
      if (seed != maxPair.first)
        std::cout << seed << " ";
        adjMatrix_[{seed,cl}] = 0.;
    }
    std::cout << std::endl;
  }
}

std::pair<GraphMap::GraphOutput, GraphMap::GraphOutputMap> GraphMap::collectSeparately(float threshold){
  // Collect all the nodes around cat1, but not other cat1 nodes
  GraphOutputMap cat0GraphMap;
  // Save a subgraph of only cat1 nodes, without self-loops
  GraphOutput  cat1NodesGraph;

  // superNodes are already included in order
  for(const auto & s : nodesCategories_[1]){
    std::cout << "seed: " << s << std::endl;
    std::vector<uint> collectedNodes;
    std::vector<uint> collectedCat1Nodes;
    // Check if the seed if still available
    if (adjMatrix_[{s,s}] < threshold) continue;
    // Loop on the out-coming edges 
    for (const auto & out : edgesOut_[s]){
      if (out != s && adjMatrix_[{out,out}] > 0){
        // Check if it is another cat1 node
        // DO NOT CHECK the score, it will be checked during the merging
        collectedCat1Nodes.push_back(out);
        // No self-loops in this represetnation. 
        // Then continue and do not work on this edgeOut
        continue;
      }
      // Check the threshold for association
      if (adjMatrix_[{s, out}] >= threshold){
        std::cout << "\tOut edge: " << s << " --> " << out<< " (" << adjMatrix_[{s, out}] << " )"<< std::endl;
        // Save the node
        collectedNodes.push_back(out);
        // The links of the node to other cat1 nodes are not touched
        // IF the function is called after assignHighestScoreEdge
        // the other links have been already remove.
        // IF not: the same node can be assigned to more subgraphs.
        // The self-loop is included in this case in order to save the cat1 node
        // in its own sub-graph.
      }
    }
    cat0GraphMap[s] = collectedNodes;
    cat1NodesGraph.push_back({s, collectedCat1Nodes});
  }
  return std::make_pair(cat1NodesGraph, cat0GraphMap);
}


void GraphMap::mergeSubGraphs(float threshold, GraphOutput cat1NodesGraph, GraphOutputMap cat0GraphMap){
  // We have the graph between the cat1 nodes and a map of
  // cat0 nodes connected to each cat1 separately. 
  // Now we link them and build superGraphs starting from the first seed
  std::cout << "Starting merging" << std::endl;
  for (const auto & [s, other_seeds]: cat1NodesGraph){
    std::cout << "seed: " << s <<std::endl;
    // Check if the seed is still available
    if (adjMatrix_[{s,s}] < threshold) continue;
    // If it is, we collect the final list of nodes
    std::vector<uint> collectedNodes;
    // Take the previously connected cat0 nodes
    const auto & cat0nodes = cat0GraphMap[s];
    collectedNodes.insert(std::end(collectedNodes), std::begin(cat0nodes), std::end(cat0nodes));
    // Check connected cat1 nodes
    for (const auto & out_s: other_seeds){
      // Check the score of the edge for the merging 
      if (adjMatrix_[{s, out_s}] > threshold){
        std::cout <<"\tMerging nodes from seed: " << out_s << std::endl;
        // Take the nodes already linked to this cat1 node
        const auto & otherNodes = cat0GraphMap[out_s];
        // We don't check for duplicates because assignHighestScoreEdge() should
        // have been called already
        collectedNodes.insert(std::end(collectedNodes), std::begin(otherNodes), std::end(otherNodes));
        // Now let's disable the secondary seed
        adjMatrix_[{out_s, out_s}] = 0.;
        // This makes the strategy  NOT RECURSIVE
        // Other seeds linked to the disable seed won't be collected, but analyzed independently.
      }
    }
    graphOutput_.push_back({s, collectedNodes});
  } 
}


void GraphMap::resolveSuperNodesEdges(float threshold){
  // superNodes are already included in order
  for(const auto & s : nodesCategories_[1]){
    std::cout << "seed: " << s << std::endl;
    // Check if the seed if still available
    if (adjMatrix_[{s,s}] < threshold) continue;
    // Loop on the out-coming edges 
    for (const auto & out : edgesOut_[s]){
      if (out != s && adjMatrix_[{out,out}] > 0){
        // This is a link to another cat1 node
        // If the edge score is good the other cat1 node is disabled
        if (adjMatrix_[{s, out}] > threshold){
          std::cout << "\tdisable seed: " << out << std::endl;
          adjMatrix_[{out,out}] = 0.;
        }
      }
    }
  }
}
