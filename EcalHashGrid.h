#ifndef RecoEcal_EgammaCoreTools_EcalHashGrid_h
#define RecoEcal_EgammaCoreTools_EcalHashGrid_h

#include <vector>
#include <array>
#include <map>
#include <algorithm>

/*
 * Class handling spatial hash grid for ECAL clusters
 *
 */

namespace reco {
  
  class EcalHashGrid {
    
  public:
    EcalHashGrid(uint nNodes);

    static constexpr uint N_CELLS = 300; // 30 eta x 10 phi  
    void addNode(double eta, double phi);
    uint hashFunction(double eta, double phi);
    
  private:
    uint nNodes_;
    std::array<std::vector<uint>, N_CELLS> hashTable;
    std::vector<uint> objectIndex; 
  };

}  // namespace reco

#endif


