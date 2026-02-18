#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc) : rAdc(rAdc) {}

void CPROCESS::step() {
  rAdc.conv_tnf({               
    CADC_STORAGE::Ud,
    CADC_STORAGE::ILeak1, 
    CADC_STORAGE::ILeak2,
    CADC_STORAGE::ch_HRf 
  });
  
}


