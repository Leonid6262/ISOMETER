#pragma once

#include "dac.hpp"
#include "settings_eep.hpp"

class COUT_4_20 { 
  
public:
  
  COUT_4_20(CDAC_PWM&, CEEPSettings&);
  
  CDAC_PWM& dac;
  
  void update(unsigned short);                    // Обновление данных
  
private: 
  CEEPSettings& rSet;
  
};

