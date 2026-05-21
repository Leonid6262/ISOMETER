#include "out_4_20mA.hpp"
#include "LPC407x_8x_177x_8x.h"

COUT_4_20::COUT_4_20(CDAC_PWM& dac, CEEPSettings& rSet) : dac(dac), rSet(rSet) { }
 
void COUT_4_20::update(unsigned short R) { 
  
  dac.conv((R * CDAC_PWM::DAC_PWM_MAX_VAL) / rSet.getSettings().Rmax_20mA); 

}


