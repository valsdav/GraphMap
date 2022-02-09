#include "EcalHashGrid.h"
#include <cmath>

using namespace reco;

EcalHashGrid::EcalHashGrid(uint nNodes):
  nNodes_(nNodes){
  objectIndex.resize(nNodes);
}

uint EcalHashGrid::hashFunction(double eta, double phi){
  bool negative = std::signbit(eta);
  int ieta = std::abs(eta) // 0.6;
    return 0;
}

