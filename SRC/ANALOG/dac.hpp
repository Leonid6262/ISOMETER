#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "Peripherals.hpp"

// DAC на бпзе PWM
class CDAC_PWM {
 private:
  LPC_PWM_TypeDef* PWM_DAC;

  static constexpr unsigned int DAC_PWM_T = 3000;  // Период PWM. dac_pwm_T * 1/60 uS = 50 uS -> 20 kHz

 public:
  static constexpr unsigned short DAC_PWM_MAX_VAL = DAC_PWM_T - 1;  // Максимальное значение DAC_PWM
  static constexpr unsigned short DAC_PWM_MIN_VAL = 0;               // Минимальное значение DAC_PWM

  CDAC_PWM(LPC_PWM_TypeDef*);

  void conv(unsigned short);
};
