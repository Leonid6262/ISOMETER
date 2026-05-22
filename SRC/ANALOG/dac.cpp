#include "dac.hpp"
#include "settings_eep.hpp"
#include "system_LPC177x.h"

CDAC_PWM::CDAC_PWM(LPC_PWM_TypeDef* PWM_DAC) : PWM_DAC(PWM_DAC) { 
  
  PWM_DAC->TCR = 0x000;                 // Сброс регистра таймера
  PWM_DAC->TCR = bPWM::TCR_RESET;       // Сброс таймера 
  
  PWM_DAC->PCR |= bPWM::PWMENA5;                         // Включение PWM1:5 в стационарном режиме
  PWM_DAC->MR0 = DAC_PWM_T;                              // Период ШИМ. Канал  PWM5, стационарный. MR0 - включение
  PWM_DAC->LER |= bPWM::MAT5LATCHEN | bPWM::MAT0LATCHEN; // Обновление MR5 и MR0
  PWM_DAC->TCR = bPWM::CE_PWMEN;                         // Включение таймера и PWM
  
} 

void CDAC_PWM::conv(unsigned short data) { // dac1_min_val...dac1_max_val -> 0V...+3V
  if (data > DAC_PWM_MAX_VAL) {
    data = DAC_PWM_MAX_VAL;
  }
  PWM_DAC->MR5 = data;
  PWM_DAC->LER |= bPWM::MAT5LATCHEN;
}
