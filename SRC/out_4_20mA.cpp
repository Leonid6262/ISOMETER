#include "out_4_20mA.hpp"
#include "LPC407x_8x_177x_8x.h"

COUT_4_20::COUT_4_20(CDAC_PWM& dac, unsigned short max_dac, CEEPSettings& rSet) : dac(dac), max_dac(max_dac), rSet(rSet) {}
 
void COUT_4_20::update(unsigned short R) { dac.conv((R * max_dac) / rSet.getSettings().Rmax_20mA); }


