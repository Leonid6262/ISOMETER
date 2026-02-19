#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc) : rAdc(rAdc) {}

void CPROCESS::step() {
  
  rAdc.conv_tnf({               
    static_cast<unsigned char>(CADC::EADC_NameCh::Ud),
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1), 
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2),
  });
  
  switch (phases) {
  case EPhases::PhaseP:         break;   
  case EPhases::MeasP:          break; 
  case EPhases::PhaseN:         break;   
  case EPhases::MeasN:          break; 
  }
  
  
  
}


