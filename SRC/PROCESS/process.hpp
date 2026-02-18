#pragma once 
#include "bool_name.hpp"
#include "AdcStorage.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"

class CPROCESS {
  
private:
 CADC& rAdc;
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x100;  

public:
  
 CPROCESS(CADC&);
 
 static signed short tx_dma_buffer[TRANSACTION_LENGTH];
 static signed short rx_dma_buffer[TRANSACTION_LENGTH];
 
 void step();
  

};
