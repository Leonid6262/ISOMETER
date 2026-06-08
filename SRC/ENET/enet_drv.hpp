#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "emac.hpp"
#include <cstring>       

class CENET_DRV {
    
public:

    CENET_DRV();
    
    unsigned int status_rx;
    
    enum class ReceiveStatus { EMPTY, FRAME_RECIVED, ERROR };
    
    ReceiveStatus receiveFrame(unsigned char*);
    void sendFrame(const unsigned char*, unsigned short);

private:
  
  unsigned int idx_Cons;
  unsigned int idx_Prod;
  
  static constexpr unsigned int CRCERR = (1UL << 20);

};







