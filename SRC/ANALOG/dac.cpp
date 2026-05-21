#include "dac.hpp"
#include "settings_eep.hpp"
#include "system_LPC177x.h"

CDAC_PWM::CDAC_PWM() { 
  
  LPC_SC->PCONP|= CLKPWR_PCONP_PCPWM1;
  LPC_IOCON->P2_4  = IOCON_PORT_PWM;     // P2_4 -> PWM1:5 PWM_DAC1 
  PWM_DAC = LPC_PWM1;
  
  PWM_DAC->PR = PWM_div_1 - 1;  // при PWM_div=1, F=60МГц
  PWM_DAC->TCR = 0x000;         // Сброс регистра таймера
  PWM_DAC->TCR = 0x002;         // Сброс таймера
  
  PWM_DAC->PCR |= _PWMENA5;                     // Включение PWM1:5 в стационарном режиме
  PWM_DAC->MR0 = _DAC_PWM_T;                    // Период ШИМ. Канал  PWM5, стационарный. MR0 - включение
  PWM_DAC->LER |= _MAT5LATCHEN | _MAT0LATCHEN;  // Обновление MR5 и MR0
  PWM_DAC->TCR = _CE_PWMEN;                     // Включение таймера и PWM
  
}

void CDAC_PWM::conv(unsigned short data) { // dac1_min_val...dac1_max_val -> 0V...+3V
  if (data > DAC_PWM_MAX_VAL) {
    data = DAC_PWM_MAX_VAL;
  }
  PWM_DAC->MR5 = data;
  PWM_DAC->LER |= _MAT5LATCHEN;
}
