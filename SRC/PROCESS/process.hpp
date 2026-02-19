#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"

class CPROCESS {
  
  
public:  
  CPROCESS(CADC&);
  inline float* getPointerR() { return &R;  }
  
  void step();
    CADC& rAdc;
private:

  enum class EPhases : unsigned short {
    PhaseP,
    MeasP,
    PhaseN,
    MeasN,
  };
  
  EPhases phases;
  
  float R; 
  
};
