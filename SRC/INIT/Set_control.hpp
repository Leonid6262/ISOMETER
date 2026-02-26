#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "process.hpp" 

class CSET_CONTROL {
public:
  static inline StatusRet err_load = StatusRet::SUCCESS;
  static inline unsigned int prev_TC0;
  
  static void indication() {
    dTrsPhase = LPC_TIM0->TC - prev_TC0;
    if(dTrsPhase > LED_PAUSED) {
      prev_TC0 = LPC_TIM0->TC;
      led_on = !led_on;
      if(led_on) {
        CPROCESS::LampMeasOff();
        CPROCESS::LampAlarm1Off();
        CPROCESS::LampAlarm2Off();
      } else {
        CPROCESS::LampMeasOn();
        CPROCESS::LampAlarm1On();
        CPROCESS::LampAlarm2On();
      }
    }   
  }
  
  static constexpr bool only_ctr_enter = true;
  
private: 
  static inline bool led_on = false; 
  static inline unsigned int dTrsPhase = 0;
  
  static constexpr unsigned int LED_PAUSED  = 2000000;
};



