#pragma once

#include "bool_name.hpp"
#include "settings_eep.hpp"

class CDIN_STORAGE {
 public:
   
  //--- Входы контроллера, порт Pi0 ---
  inline bool Pi00() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b0;} // 
  inline bool Pi01() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b1;} // 
  inline bool Pi02() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b2;} // 
  inline bool Pi03() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b3;} // 
  inline bool Pi04() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b4;} // 
  inline bool Pi05() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b5;} // 
  inline bool Pi06() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b6;} // 
  inline bool Pi07() {return UData_din_f[static_cast<char>(EIBNumber::CPU_PORT)].b7;} // 
  
  //--- Входы контроллера byte-0 SPI ---
  inline bool Pi10() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b0;} // 
  inline bool Pi11() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b1;} //
  inline bool Pi12() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b2;} //
  inline bool Pi13() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b3;} //
  inline bool Pi14() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b4;} // 
  inline bool Pi15() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b5;} // 
  inline bool Pi16() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b6;} // 
  inline bool Pi17() {return UData_din_f[static_cast<char>(EIBNumber::CPU_SPI)].b7;} //
  
  //--- Входы S600 byte-0 ---
  //
  //--- Входы S600 byte-1 ---
  //
  
  //--- Выходы контроллера Po0 ---
  static inline void bNamePo0B0(State state)    {edit_bit(0, state);}
  static inline void bNamePo0B1(State state)    {edit_bit(1, state);} 
  static inline void bNamePo0B2(State state)    {edit_bit(2, state);} 
  static inline void bNamePo0B3(State state)    {edit_bit(3, state);}
  static inline void bNamePo0B4(State state)    {edit_bit(4, state);}
  static inline void bNamePo0B5(State state)    {edit_bit(5, state);}
  static inline void bNamePo0B6(State state)    {edit_bit(6, state);}
  static inline void bNamePo0B7(State state)    {edit_bit(7, state);} 

  //--- Выходы контроллера SPI ---
  inline void Pi20  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b0 = static_cast<bool>(state);}
  inline void Pi21  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b1 = static_cast<bool>(state);}
  inline void Pi22  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b2 = static_cast<bool>(state);}
  inline void Pi23  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b3 = static_cast<bool>(state);}
  inline void Pi24  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b4 = static_cast<bool>(state);}
  inline void Pi25  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b5 = static_cast<bool>(state);}
  inline void Pi26  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b6 = static_cast<bool>(state);}
  inline void Pi27  (State state) {UData_dout[static_cast<char>(EOBNumber::CPU_SPI)].b7 = static_cast<bool>(state);}
  
  //--- Выходы S600 ---
  //
  
  // Номера считываемых байтов
  enum class EIBNumber {
    CPU_PORT = 0,    // Контроллер Pi0
    CPU_SPI = 1,     // Контроллер SPI
    BYTE_0_EX1 = 2,  // S600 byte-0 SPI
    BYTE_1_EX1 = 3   // S600 byte-1 SPI
  };

  // Номера записываемых байтов последовательности SPI
  enum class EOBNumber {
    BYTE_1_EX1 = 0,  // S600 byte-0 (в S600 2/1 - отсутствует)
    BYTE_0_EX1 = 1,  // S600 byte-1
    CPU_SPI = 2      // Контроллер (всегда (BYTES_RW_MAX-1)), так как, замыкает последовательность)
  };

  /*                    Последовательность цикла:

       MISO: <- byte_din_cpu  <- byte_0_din_ex1  <- byte_1_din_ex1
                     0                 1                  2
       MOSI: -> byte_dout_cpu -> byte_0_dout_ex1 -> byte_1_dout_ex1(*)
                     2                 1                  0

       (*) При использовании S600 2/1, byte_1_dout_ex1 - выходит в эфир
 */

  union {
    unsigned char all;
    struct {
      unsigned char b0 : 1;
      unsigned char b1 : 1;
      unsigned char b2 : 1;
      unsigned char b3 : 1;
      unsigned char b4 : 1;
      unsigned char b5 : 1;
      unsigned char b6 : 1;
      unsigned char b7 : 1;
    };
  } UData_din_f[G_CONST::BYTES_RW_MAX + 1];  // Данные din портов после фильтра

  void filter(unsigned char, unsigned int, unsigned char, CEEPSettings&);

  union {
    unsigned char all;
    struct {
      unsigned char b0 : 1;
      unsigned char b1 : 1;
      unsigned char b2 : 1;
      unsigned char b3 : 1;
      unsigned char b4 : 1;
      unsigned char b5 : 1;
      unsigned char b6 : 1;
      unsigned char b7 : 1;
    };
  } UData_dout[G_CONST::BYTES_RW_MAX];  // Данные для записи в д.выходы

  static CDIN_STORAGE& getInstance() {
    static CDIN_STORAGE instance;
    return instance;
  }

  static inline void UserLedOn() { LPC_GPIO0->CLR = (1UL << B_ULED); }   // Вкл. ULED
  static inline void UserLedOff() { LPC_GPIO0->SET = (1UL << B_ULED); }  // Выкл.ULED
  static inline void UserLedToggle() {
    if (LPC_GPIO0->PIN & (1UL << B_ULED)) { LPC_GPIO0->CLR = (1UL << B_ULED); } 
    else { LPC_GPIO0->SET = (1UL << B_ULED); }
  } 
  static inline void Q1VF_On() { LPC_GPIO1->CLR = (1UL << B_Q1VF); }     // Вкл. Q1VF
  static inline void Q1VF_Off() { LPC_GPIO1->SET = (1UL << B_Q1VF); }    // Выкл.Q1VF

  static inline void edit_bit(char bit_number, State state) {
    switch (state) {
      case State::ON: {
        LPC_GPIO2->SET = static_cast<unsigned int>(1UL << (B0_PORT_OUT + (bit_number & 0x07)));
      }  // dout-on
      break;
      case State::OFF: {
        LPC_GPIO2->CLR = static_cast<unsigned int>(1UL << (B0_PORT_OUT + (bit_number & 0x07)));
      }  // dout-off
      break;
    }
  }
  
  
 private:
  static constexpr unsigned int SWITCHING_TIME = 2000000; // 200ms
   
  static constexpr unsigned short B_ULED = 9;        // Бит U-LED
  static constexpr unsigned short B_Q1VF = 13;       // Бит Q1VF
  static constexpr unsigned short B0_PORT_OUT = 24;  // 1-й бит порта

  static constexpr unsigned short N_BITS = 8;  // Количество бит в портах
  static constexpr unsigned int TIC_ms = 10000;

  signed int integrator[G_CONST::BYTES_RW_MAX + 1][N_BITS];  // Интеграторы

  // Постоянные времени интегрирования фильтра (ms первый множитель) din портов.
  // TIC_ms = 10000 дискрет таймера на 1ms.
  // То есть, 50*TIC_ms = 50ms, 0*TIC_ms - нет фильтрации, и т.п.
  static constexpr unsigned int cConst_integr_Pi0[G_CONST::BYTES_RW_MAX + 1][N_BITS] = 
  {
    {2*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms,  50*TIC_ms,  50*TIC_ms, 50*TIC_ms}, // Контроллер Pi0
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // Контроллер SPI
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // S600 byte-0 SPI
    {50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms, 50*TIC_ms}, // S600 byte-1 SPI
  }; 

  // --- Механизмы Singleton ---
  CDIN_STORAGE();
  CDIN_STORAGE(const CDIN_STORAGE&) = delete;
  CDIN_STORAGE& operator=(const CDIN_STORAGE&) = delete;
};