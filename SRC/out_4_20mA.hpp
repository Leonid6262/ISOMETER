#pragma once

#include "dac.hpp"

class COUT_4_20 { 
  
public:
  
  COUT_4_20(CDAC_PWM&);
  
  CDAC_PWM& dac;
  
  void update(unsigned short);                    // Обновление данных
  
private: 
  
  
};

