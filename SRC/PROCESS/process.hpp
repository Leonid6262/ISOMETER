#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"

class CPROCESS {
  
  
public:  
  CPROCESS(CADC&, CEEPSettings&);
  CEEPSettings& rSet;
  CADC& rAdc;
  
  inline float*        getPointerR()       { return &R;               }
  inline signed short* getPointerTUd()     { return &test_Ud_avr;     }
  inline signed short* getPointerTILeak1() { return &test_ILeak1_avr; }
  inline signed short* getPointerTILeak2() { return &test_ILeak2_avr; }
   
  static inline void UserLedOn()  { LPC_GPIO0->CLR  = (1UL << 9); } 
  static inline void UserLedOff() { LPC_GPIO0->SET  = (1UL << 9); }
  
  void set_test_mode();
  void clr_test_mode();
  void step();
  
private:
  unsigned int prev_TC0_Phase;
  unsigned int dTrsPhase;
  
  static inline void RelReadyOn()     {  }
  static inline void RelReadyOff()    {  }
  static inline void RelAlarm1On()    { LPC_GPIO2->SET = (1UL << 25); }
  static inline void RelAlarm1Off()   { LPC_GPIO2->CLR = (1UL << 25); }
  static inline void RelAlarm2On()    { LPC_GPIO2->SET = (1UL << 26); }  
  static inline void RelAlarm2Off()   { LPC_GPIO2->CLR = (1UL << 26); }
  
  static inline void LampMeasOn()     { LPC_GPIO2->SET = (1UL << 27); }
  static inline void LampMeasOff()    { LPC_GPIO2->CLR = (1UL << 27); }
  static inline void LampAlarm1On()   { LPC_GPIO2->SET = (1UL << 29); }
  static inline void LampAlarm1Off()  { LPC_GPIO2->CLR = (1UL << 29); }
  static inline void LampAlarm2On()   { LPC_GPIO2->SET = (1UL << 31); }
  static inline void LampAlarm2Off()  { LPC_GPIO2->CLR = (1UL << 31); }
  
  static inline void PN_On()  { LPC_GPIO1->CLR = (1UL << 13); }   /* Два порта в реальном изделии. */ 
  static inline void PN_Off() { LPC_GPIO1->SET = (1UL << 13); }   /* В стенде - реле.  */ 
  
  static constexpr unsigned short MEAS_PAUSED  = 7000;    // 700us - пауза между выборками
  static constexpr unsigned short WAIT_NUMBER  = 7000;    // Время заряда - 0.7ms * 7000 = 4.9s
  static constexpr unsigned short TEST_NUMBER  = 1000;    // Время заряда - 0.7ms * 1000 = 0.7s
  static constexpr unsigned short AVR_NUMBER   = 0x200;   // Количество выборок. (tmeas+0.7ms) * 512 примерно 400ms
  static constexpr unsigned short N_AVR        = 500;     // Кадр усреднения
  static constexpr unsigned short sh_avr       = 5;       // Сдвиг кадра
  
  static constexpr unsigned short gis_const    = 2;       // 2kOhm - гитерезис 1-го диапазона 
  static constexpr unsigned short range        = 50;      // 50kOhm - 1-й диапазон от 0 до range
  static constexpr unsigned short gis_percent  = 15;      // 15% - гитерезис 2-го диапазона (от range_1 и выше) 
  static constexpr unsigned short Rmax         = 1000;    // Максимально измеряемое сопротивление [kOhm] 
  
  static constexpr float Umeas  = 20000.0f;               // U измерений [mV]
  static constexpr float RT     = 39.0f + 3.9f + 3.9f;    // RT [kOhm]
  static constexpr float Rs1    = 0.1f;                   // R шунта 1 [kOhm]
  static constexpr float Rs2    = 0.1f;                   // R шунта 2 [kOhm]

  signed short Ud_P[AVR_NUMBER];
  signed short ILeak1_P[AVR_NUMBER]; 
  signed short ILeak2_P[AVR_NUMBER];
  signed short Ud_N[AVR_NUMBER];
  signed short ILeak1_N[AVR_NUMBER]; 
  signed short ILeak2_N[AVR_NUMBER];
  
  unsigned short pause_counter;
  unsigned short wait_number;
  signed short test_Ud_avr;
  signed short test_ILeak1_avr; 
  signed short test_ILeak2_avr;
  
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
