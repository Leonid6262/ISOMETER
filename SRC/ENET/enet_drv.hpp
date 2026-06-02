#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "emac.hpp"
#include <cstring>       

class CENET_DRV {
    
public:

    CENET_DRV();
    
    unsigned int status_rx;
    
    enum class ReceiveStatus 
    {
      EMPTY,  
      FRAME_RECIVED,
      ERROR
    };
    
    ReceiveStatus receiveFrame(unsigned char*);
    void sendFrame(const unsigned char*);

private:
  
  unsigned int idx_Cons;
  unsigned int idx_Prod;
  
  enum : unsigned int {
    RX_STATUS_RANGE_ERROR = (1UL << 26), // Ошибкой не считается, EMAC не различает тип и длину кадра.
    RX_STATU_LAST_FLAG    = (1UL << 30), // Уведомляющее сообщение о последнем фрагменте
    RX_STATU_SUM_ERROR    = (1UL << 31)  // Исключается из за RX_STATUS_RANGE_ERROR
  };
  
  static constexpr unsigned int RX_CTRL_ERR_BITS = 
    RX_STATUS_RANGE_ERROR | RX_STATU_LAST_FLAG |
    RX_STATU_SUM_ERROR | CEMAC::DESC_RX_SIZE;
  
};







