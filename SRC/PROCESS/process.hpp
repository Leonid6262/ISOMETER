#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"

class CPROCESS {
  
  
public:  
  CPROCESS(CADC&);
  CADC& rAdc;
  
  inline float* getPointerR() { return &R;  }
  
  static inline void UserLedOn() { LPC_GPIO0->CLR  = (1UL << B_ULED); } 
  static inline void UserLedOff() { LPC_GPIO0->SET = (1UL << B_ULED); }
  static inline void UserLedToggle() {
    if (LPC_GPIO0->PIN & (1UL << B_ULED)) { LPC_GPIO0->CLR = (1UL << B_ULED); } 
    else { LPC_GPIO0->SET = (1UL << B_ULED); }
  } 
  
  void set_test_mode();
  void clr_test_mode();
  void step();
  
private:
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  
  static constexpr unsigned int WAIT_PERIOD = 50000000;   // 5s
  static constexpr unsigned int MEAS_PAUSED = 7000;       // 700us
  static constexpr unsigned int AVR_NUMBER  = 0x200;
  
  static constexpr unsigned short B_ULED = 9;        
  static constexpr unsigned short B_PN = 13;        
  
  static inline void PN_On()  { LPC_GPIO1->CLR = (1UL << B_PN); }     
  static inline void PN_Off() { LPC_GPIO1->SET = (1UL << B_PN); }    

  signed short Ud_P[AVR_NUMBER];
  signed short ILeak1_P[AVR_NUMBER]; 
  signed short ILeak2_P[AVR_NUMBER];
  signed short Ud_N[AVR_NUMBER];
  signed short ILeak1_N[AVR_NUMBER]; 
  signed short ILeak2_N[AVR_NUMBER];
  
  unsigned short ind_avr;
  
  enum class EPhases {
    PhaseP,
    MeasP,
    PhaseN,
    MeasN
  };
  
  enum class EMode {
    Work,
    Test
  };
  
  EPhases phases;
  EMode mode;
  
  void conv_adc();
  void conv(EPhases);
  void calc_avr(EPhases);
  
  float UdP_avr;
  float UdN_avr;
  float ILeak1P_avr;
  float ILeak1N_avr;
  float ILeak2P_avr;
  float ILeak2N_avr;
  float R; 
  
};
