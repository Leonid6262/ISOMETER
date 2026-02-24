#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"

class CPROCESS {
  
  
public:  
  CPROCESS(CADC&, CEEPSettings&);
  CADC& rAdc;
  CEEPSettings& rSet;
  
  inline float* getPointerR()  { return &R;  }
  
  static inline void MeasOn()     { LPC_GPIO2->SET  = (1UL << 27); }
  static inline void MeasOff()    { LPC_GPIO2->CLR  = (1UL << 27); }
  static inline void Alarm1On()   { LPC_GPIO2->SET  = (1UL << 29); }
  static inline void Alarm1Off()  { LPC_GPIO2->CLR  = (1UL << 29); }
  static inline void Alarm2On()   { LPC_GPIO2->SET  = (1UL << 31); }
  static inline void Alarm2Off()  { LPC_GPIO2->CLR  = (1UL << 31); }
  
  static inline void UserLedOn()  { LPC_GPIO0->CLR  = (1UL << 9);  } 
  static inline void UserLedOff() { LPC_GPIO0->SET  = (1UL << 9);  }
  
  void set_test_mode();
  void clr_test_mode();
  void step();
  
private:
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  
  static constexpr unsigned short MEAS_PAUSED  = 7000;    // 700us - пауза между выборками
  static constexpr unsigned short WAIT_NUMBER  = 7000;    // Время заряда - 0.7ms * 7000 = 4.9s
  static constexpr unsigned short AVR_NUMBER   = 0x200;   // Количество выборок. (tmeas+0.7ms) * 512 примерно 400ms
  static constexpr unsigned short N_AVR        = 500;     // Кадр усреднения
  static constexpr unsigned short sh_avr       = 5;       // Сдвиг кадра
  
  static constexpr unsigned short gis_const    = 2;       // 2kOhm - гитерезис 1-го диапазона 
  static constexpr unsigned short range        = 50;      // 50kOhm - 1-й диапазон от 0 до range
  static constexpr unsigned short gis_percent  = 15;      // 15% - гитерезис 2-го диапазона (от range_1 и выше) 
  
  static constexpr float Umeas  = 20000.0f;             // U измерений [mV]
  static constexpr float RT  = 39.0f + 3.9f + 3.9f;     // RT [kOhm]
  static constexpr float Rs1 = 0.1f;                    // R шунта 1 [kOhm]
  static constexpr float Rs2 = 0.1f;                    // R шунта 2 [kOhm]
         
  static constexpr unsigned short B_PN = 13;        
  
  static inline void PN_On()  { LPC_GPIO1->CLR = (1UL << B_PN); }     
  static inline void PN_Off() { LPC_GPIO1->SET = (1UL << B_PN); }    

  signed short Ud_P[AVR_NUMBER];
  signed short ILeak1_P[AVR_NUMBER]; 
  signed short ILeak2_P[AVR_NUMBER];
  signed short Ud_N[AVR_NUMBER];
  signed short ILeak1_N[AVR_NUMBER]; 
  signed short ILeak2_N[AVR_NUMBER];
  
  unsigned short pause_counter;
  
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
  void wait(EPhases);
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
