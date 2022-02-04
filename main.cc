#include "GraphMap.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <cstdlib>

using namespace std;
using namespace reco;

GraphMap readGraph(string path){
  vector<vector<uint>> nodes;
  vector<vector<float>> scores;
  std::ifstream input {path};
  if (!input.fail()){
    string line;
    while(std::getline(input, line)){
      vector<uint> n;
      vector<float> s;
      std::istringstream iss(line);
      bool read_score = false;
      for (std::string token; std::getline(iss, token, ' '); ) {
        if (token == "|") {
          read_score = true;
          continue;
        }
        if (!read_score)
          n.push_back(std::stoul(token));
        else
          s.push_back(std::stof(token));
      }
      nodes.push_back(n);
      scores.push_back(s);
    }
  }
  GraphMap graph {static_cast<uint>(nodes.size()), {0,1}};
  std::cout << "Nodes: " << nodes.size() << std::endl;
  for(size_t k=0; k<nodes.size(); k++){
    const auto & out = nodes.at(k);
    if (out.size()>1){
      graph.addNode(k, 1);
      for (size_t i =0; i<out.size(); i++){
        graph.addEdge(k, out.at(i));
        graph.setAdjMatrix( k, out.at(i), scores.at(k).at(i));
      }
    }
    else graph.addNode(k, 0);
  }
  return graph;
}

void printOutput(GraphMap::GraphOutput out){
  for (const auto & [s,cls] : out){
    std::cout << "Seed: " << s << " ---> ";
    for (const auto & cl :cls){
      std::cout << cl << " ";
    }
    std::cout << std::endl;
  }  
}


int main( int argc, char** argv){

  std::cout << "Reading graph: " << argv[1] << std::endl;
  GraphMap graph = readGraph(argv[1]);

  for (int type=GraphMap::CollectionStrategy::A; type<=GraphMap::CollectionStrategy::E; type++){
    auto T = static_cast<GraphMap::CollectionStrategy>(type);
    GraphMap g {graph};
    std::cout << std::endl << "===================================" << std::endl;  
    std::cout << std::endl <<  "Strategy " << T << std::endl;
    g.printGraphMap();
    std::cout << std::endl << "Collecting..." << std::endl;
    const auto & out = g.collectNodes(T, 0.5);
    std::cout << "OUTPUT:" << std::endl;   
    printOutput(out);
  
  }
  
  

  // std::cout << std::endl << "===================================" << std::endl;  
  // std::cout << "Strategy B" << std::endl;
  // graph2.printGraphMap();
  // std::cout << std::endl << "Collecting..." << std::endl;
  // const auto & out2 = graph2.collectNodes(GraphMap::CollectionStrategy::B, 0.5);
  // std::cout << "OUTPUT:" << std::endl; 
  // printOutput(out2);

  // std::cout << std::endl << "===================================" << std::endl;  
  // std::cout << "Strategy C" << std::endl;
  // graph3.printGraphMap();
  // std::cout << std::endl << "Collecting..." << std::endl;
  // const auto & out3 = graph3.collectNodes(GraphMap::CollectionStrategy::C, 0.5);
  // std::cout << "OUTPUT:" << std::endl; 
  // printOutput(out3);

  return 0;
    
}
