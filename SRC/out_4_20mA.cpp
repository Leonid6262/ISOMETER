#include "out_4_20mA.hpp"
#include "LPC407x_8x_177x_8x.h"

COUT_4_20::COUT_4_20(CDAC_PWM& dac, unsigned short max_dac, CEEPSettings& rSet) : dac(dac), max_dac(max_dac), rSet(rSet) {}
 
void COUT_4_20::update(unsigned short R) { 
  signed short dac_data = max_dac - ((R * max_dac) / rSet.getSettings().Rmax_20mA);
  if(dac_data < 0) dac_data = 0;
  dac.conv(dac_data); 
}


