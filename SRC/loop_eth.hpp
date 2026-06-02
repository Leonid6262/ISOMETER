#pragma once
     
#include "emac.hpp"
#include "enet_drv.hpp"

class CLOOP_ETH {
public: 
  
    static const unsigned char MAC_PC[];

    unsigned char sendFrame[CEMAC::ETH_FRAG_SIZE - 4]; //6mac1+6mac2+2type+46data(макс) = 60байт (4crc add EMAC)
    unsigned char rxBuffer[CEMAC::ETH_FRAG_SIZE];      //6mac1+6mac2+2type+46data+4crc  = 64байта
    
    CENET_DRV& rEnet_drv;
    CLOOP_ETH(CENET_DRV&);
    void test(); 
    
private:
  
    void init(); 

};
