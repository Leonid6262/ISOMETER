#pragma once

#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"
#include "settings_eep.hpp"
#include "Peripherals.hpp"

class CSET_UART{
public:    
  enum class EUartInstance {
    UART_0,              
    UART_2,                        
  };
  
  static LPC_UART_TypeDef* configure(EUartInstance, CEEPSettings&);
  
private: 
  // Настройки скорости
  struct SBaudRateSettings {
    unsigned int DLM;
    unsigned int DLL;
    unsigned int FDR;
  };
   
  /* Настройка на 9600 */
  static constexpr SBaudRateSettings baud_9600 = {
    .DLM = 0,
    .DLL = 255, 
    .FDR = 0xD7    // MulVal=13, DivAddVal=7
  };
  /* Настройка на 19200. UART_baudrate = 60000000 / ( (16*101)  *  (1+(14/15)) ) = 19205 (+0.026%) */ 
  static constexpr SBaudRateSettings baud_19200 = {
    .DLM = 0,
    .DLL = 101,
    .FDR = 0xFE    // MulVal = 15, DivAddVal = 14 
  };
  /* Настройка на 38400 */
  static constexpr SBaudRateSettings baud_38400 = {
    .DLM = 0,
    .DLL = 52,
    .FDR = 0xFE    // MulVal = 15, DivAddVal = 14
  };
  /* Настройка на 57600 */
  static constexpr SBaudRateSettings baud_57600 = {
    .DLM = 0,
    .DLL = 65,
    .FDR = 0x10    // MulVal = 1, DivAddVal = 0
  }; 
  /* Настройка на 115200. UART_baudrate = 60000000 / ( (16*21)  *  (1+(5/9)) ) =  114796 (-0.35%) */
  static constexpr SBaudRateSettings baud_115200 = {
    .DLM = 0,
    .DLL = 21,
    .FDR = 0x95    // MulVal = 9, DivAddVal = 5
  };
  
  LPC_UART_TypeDef* UART;
};

