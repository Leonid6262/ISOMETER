#include "dac.hpp"
#include "settings_eep.hpp"
#include "system_LPC177x.h"

CDAC_PWM::CDAC_PWM() { 
  
  PWM_DAC = LPC_PWM1;
  PWM_DAC->PR = PWM_div_1 - 1;  // при PWM_div=6, F=60МГц/6=10МГц, 1тик=0.1мкс
  PWM_DAC->TCR = 0x000;         // Сброс регистра таймера
  PWM_DAC->TCR = 0x002;         // Сброс таймера
  
  pMR = &LPC_PWM1->MR5;
  LER = _MAT5LATCHEN;                  // Обновление MR5
  *pMR = DAC_PWM_MIN_VAL;
  PWM_DAC->PCR |= _PWMENA5;            // Включение PWM1:5 в стационарном режиме
  PWM_DAC->MR0 = _DAC_PWM_T;           // Период ШИМ. Канал  PWM5, стационарный. MR0 - включение
  PWM_DAC->LER |= LER | _MAT0LATCHEN;  // Обновление MR5 и MR0
  PWM_DAC->TCR = _CE_PWMEN;            // Включение таймера и PWM
  
}

void CDAC_PWM::conv(unsigned short data) { // dac1_min_val...dac1_max_val -> 0V...+3V
  if (data > DAC_PWM_MAX_VAL) {
    data = DAC_PWM_MAX_VAL;
  }
  *pMR = data;
  PWM_DAC->LER |= LER;
}
