#pragma once

#include "LPC407x_8x_177x_8x.h"

// DAC на бпзе PWM
class CDAC_PWM {
 private:
  LPC_PWM_TypeDef* PWM_DAC;
   
  static constexpr unsigned int IOCON_PORT_PWM = 1;
  static constexpr unsigned int PWM_div_1 = 1;  // Делитель частоты

  static constexpr unsigned int _MAT0LATCHEN = 1UL << 0;
  static constexpr unsigned int _MAT5LATCHEN = 1UL << 5;

  static constexpr unsigned int _CE_PWMEN = 0x09;

  static constexpr unsigned int _PWMENA5 = 1UL << 13;

  static constexpr unsigned int _DAC_PWM_T = 3000;  // Период PWM. dac_pwm_T * 1/60 uS = 50 uS -> 20 kHz

 public:
  static constexpr unsigned short DAC_PWM_MAX_VAL = _DAC_PWM_T - 1;  // Максимальное значение DAC_PWM
  static constexpr unsigned short DAC_PWM_MIN_VAL = 0;               // Минимальное значение DAC_PWM

  CDAC_PWM();

  void conv(unsigned short);
};
