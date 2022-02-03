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
                graph.setAdjMatrix( k, i, scores.at(k).at(i));
            }
        }
        else graph.addNode(k, 0);
    }
    return graph;
}


int main( int argc, char** argv){

    GraphMap graph = readGraph(argv[1]);

    graph.printGraphMap();

    const auto & out = graph.collectNodes(GraphMap::CollectionStrategy::A, 0.5);
    return 0;
    
}
