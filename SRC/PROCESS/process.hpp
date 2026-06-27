#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "adc.hpp"
#include "mb_rtu_slave.hpp" 
#include "mbDataProxy.hpp"
#include "out_4_20mA.hpp"
#include "pause_us.hpp"
#include "rtc.hpp"
#include "PeripheralsInit.hpp"
#include "event_log.hpp"

class CPROCESS {
  
public:  
  CPROCESS(CADC&, CEEPSettings&, CModbusDataProxy&, COUT_4_20&, CRTC&, CEVENT_LOG&);
  CModbusDataProxy& rModbusData;
  COUT_4_20& rCOUT_4_20;
  CEEPSettings& rSet;
  CADC& rAdc;
  CRTC& rRTC;
  CEVENT_LOG& rEventLog;
  
  inline float*          getPointerR1()      { return &R1;              }
  inline float*          getPointerR2()      { return &R2;              }
  inline float*          getPointerUd_V()    { return &Ud_avr_V;        }
  inline signed short*   getPointerUd_d()    { return &Ud_avr_d;        }
  inline char*           getPointerDT()      { return date_time;        }
  inline char*           getPointerRES()     { return RES;              }
  inline float*          getPointerP5()      { return &P5_avr_V;        }
  inline float*          getPointerN5()      { return &N5_avr_V;        }
  inline bool*           getPointerC40()     { return &CP40V_PN;        }

  static inline void UserLedOn()  { P::G1->CLR  = (1UL << bg::B_ULED); } 
  static inline void UserLedOff() { P::G1->SET  = (1UL << bg::B_ULED); }
  
  static inline bool get_С_P40V()    { return !(((P::G0->PIN) & (1UL << bg::C_P40)) != 0); }
  
  static inline void LampReadyOn()    { P::G1->CLR = (1UL << bg::B_LampReady); }
  static inline void LampReadyOff()   { P::G1->SET = (1UL << bg::B_LampReady); }
  static inline void LampAlarm1On()   { P::G1->CLR = (1UL << bg::B_LampAlarm1); }
  static inline void LampAlarm1Off()  { P::G1->SET = (1UL << bg::B_LampAlarm1); }
  static inline void LampAlarm2On()   { P::G1->CLR = (1UL << bg::B_LampAlarm2); }
  static inline void LampAlarm2Off()  { P::G1->SET = (1UL << bg::B_LampAlarm2); }
  
  static inline void RelReadyOn()     { P::G1->SET = (1UL << bg::B_RelReady); }
  static inline void RelReadyOff()    { P::G1->CLR = (1UL << bg::B_RelReady); }
  
  static constexpr unsigned short Rmax         = 2500;

  unsigned int prev_TC0_Phase;
  
  // --- Статус ---
  union {
    unsigned short all;
    struct {
      unsigned char sWork      : 1; // Работа
      unsigned char sFault     : 1; // Неисправность
      unsigned char sLessMin   : 1; // R менише Rmin
      unsigned char sMoreMax   : 1; // R больше Rmax
      unsigned char sNormRange : 1; // R в диапазоне измерений
      unsigned char sAlarm1    : 1; // Alarm1
      unsigned char sAlarm2    : 1; // Alarm2
      unsigned char sFaultP5   : 1; // +5V
      unsigned char sFaultN5   : 1; // -5V
      unsigned char sFaultS40  : 1; // +/- 40V
    };
  } UStatus;
  
  unsigned short prevUStatus;
  
  // Установка/сброс битов статуса
  void bsWork(State state)      { UStatus.sWork      = static_cast<unsigned char>(state); }
  void bsFault(State state)     { UStatus.sFault     = static_cast<unsigned char>(state); }
  void bsLessMin(State state)   { UStatus.sLessMin   = static_cast<unsigned char>(state); }
  void bsMoreMax(State state)   { UStatus.sMoreMax   = static_cast<unsigned char>(state); }
  void bsNormRange(State state) { UStatus.sNormRange = static_cast<unsigned char>(state); }
  void bsAlarm1(State state)    { UStatus.sAlarm1    = static_cast<unsigned char>(state); }
  void bsAlarm2(State state)    { UStatus.sAlarm2    = static_cast<unsigned char>(state); }
  void bsFaultP5(State state)   { UStatus.sFaultP5   = static_cast<unsigned char>(state); }
  void bsFaultN5(State state)   { UStatus.sFaultN5   = static_cast<unsigned char>(state); }
  void bsFaultV40(State state)  { UStatus.sFaultS40  = static_cast<unsigned char>(state); }
  void clr_bs()                 { UStatus.all  = 0; }
  
  void start_test();
  void step();
  
  unsigned short R;
  float R1;
  float R2;
  signed short Ud_avr_d;
  float Ud_avr_V;
  float P5_avr_V;
  float N5_avr_V;
  
private:

  unsigned int dTrsPhase;
  bool start_log = false;
  
  static inline void Negative_phase()     { P::G1->CLR = (1UL << bg::B_TP); Pause_us(2); P::G1->SET = (1UL << bg::B_TN); }
  static inline void Positive_phase()     { P::G1->CLR = (1UL << bg::B_TN); Pause_us(2); P::G1->SET = (1UL << bg::B_TP); }
  static inline void phase_Off()          { P::G1->CLR = (1UL << bg::B_TN); P::G1->CLR = (1UL << bg::B_TP); }
  
  static inline void RelAlarm1On()     { P::G1->SET = (1UL << bg::B_RelAlarm1); }
  static inline void RelAlarm1Off()    { P::G1->CLR = (1UL << bg::B_RelAlarm1); }
  static inline void RelAlarm2On()     { P::G1->SET = (1UL << bg::B_RelAlarm2); }
  static inline void RelAlarm2Off()    { P::G1->CLR = (1UL << bg::B_RelAlarm2); }
  
  static constexpr unsigned short MEAS_PAUSED  = 7000;    // 0.7ms - пауза между выборками
  static constexpr unsigned short WAIT_NUMBER  = 5000;    // Время заряда - 0.7ms * 5000 = 3.5s
  static constexpr unsigned short AVR_NUMBER   = 0x200;   // Количество выборок. 0.7ms * 512 примерно 358ms
  static constexpr unsigned short N_AVR        = 500;     // Кадр усреднения
  static constexpr unsigned short sh_avr       = 5;       // Сдвиг кадра
  
  static constexpr unsigned short gis_const    = 2;       // 2kOhm - гитерезис 1-го диапазона 
  static constexpr unsigned short range        = 50;      // 50kOhm - 1-й диапазон от 0 до range
  static constexpr unsigned short gis_percent  = 15;      // 15% - гитерезис 2-го диапазона (от range_1 и выше) 

  static constexpr unsigned short d_max        = 4070;    // дискрет считающихся насыщеним
  
  //С учётом Uop=3V и делителя 10к/10к -> (3V * 2) / 4095 = 0.0014652015
  static constexpr float K_P5 = 0.0014652015f;  
  //С учётом Uop=3V и делителя 10к/30к между +5V и -5V -> (3V * 4) / 4095 = 0.0029304029
  static constexpr float K_N5 = 0.0029304029f;
  
  static constexpr float u      = 40000.0f;               // u [mV]
  static constexpr float RT     = 51.0f + 5.1f + 5.1f;    // RT [kOhm]
  static constexpr float Rs     = 1.0f;                   // R шунта [kOhm]

  static inline signed short Ud[AVR_NUMBER];
  static inline signed short ILeak1[AVR_NUMBER]; 
  static inline signed short ILeak2[AVR_NUMBER];
  static inline signed short P5V[AVR_NUMBER]; 
  static inline signed short N5V[AVR_NUMBER];
  
  unsigned short pause_counter;
  unsigned short wait_number;
  char date_time[G_CONST::disp_l + 1];
  char RES[G_CONST::disp_l + 1];
  char polarity;
  char N_ch;
  
  bool CP40V_PN = true;
   
  enum class EPhases {
    PhaseP,
    MeasP,
    PhaseN,
    MeasN
  };
  
  EPhases phases;
  
  void conv_adc();
  void wait(EPhases);
  void conv(EPhases);
  void calc_avr(EPhases);
  void update_modbus_data();
  void SaveEvent(CEVENT_LOG::EEvent);
  
  float dIL1;   
  float dIL2;
  float dUd;
  
  float UdP_avr;
  float UdN_avr;
  float ILeak1P_avr;
  float ILeak1N_avr;
  float ILeak2P_avr;
  float ILeak2N_avr;
  float P5V_avr; 
  float N5V_avr;
  
};
