#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "lpc177x_8x_clkpwr.h"
#include "ControlBits.hpp"

namespace P { 
  constexpr LPC_GPIO_TypeDef* G0 = LPC_GPIO0;
  constexpr LPC_GPIO_TypeDef* G1 = LPC_GPIO1;
  constexpr LPC_GPIO_TypeDef* G2 = LPC_GPIO2;
  constexpr LPC_GPIO_TypeDef* G3 = LPC_GPIO3;
  constexpr LPC_GPIO_TypeDef* G4 = LPC_GPIO4;
  constexpr LPC_GPIO_TypeDef* G5 = LPC_GPIO5;
  
  constexpr LPC_TIM_TypeDef* SYST = LPC_TIM0;
  constexpr LPC_TIM_TypeDef* TIM1 = LPC_TIM1;
  constexpr LPC_TIM_TypeDef* TIM2 = LPC_TIM2;
  constexpr LPC_TIM_TypeDef* TIM3 = LPC_TIM3;
  
  constexpr LPC_UART_TypeDef* UART0 = LPC_UART0;
  constexpr LPC_UART_TypeDef* UART2 = LPC_UART2;
  constexpr LPC_UART_TypeDef* UART3 = LPC_UART3;
  
  constexpr LPC_SSP_TypeDef* SPI0 = LPC_SSP0;
  constexpr LPC_SSP_TypeDef* SPI1 = LPC_SSP1;
  constexpr LPC_SSP_TypeDef* SPI2 = LPC_SSP2;
  
  constexpr LPC_GPDMA_TypeDef*  GPDMA = LPC_GPDMA;
  
  constexpr LPC_CAN_TypeDef* CAN1 = LPC_CAN1;
  constexpr LPC_CAN_TypeDef* CAN2 = LPC_CAN2;
  constexpr LPC_CANAF_TypeDef* CANAF = LPC_CANAF;
  constexpr LPC_CANAF_RAM_TypeDef* CANAF_RAM = LPC_CANAF_RAM;
    
  constexpr LPC_IOCON_TypeDef* IOCON = LPC_IOCON;  // IOCON 
  constexpr LPC_SC_TypeDef* SC = LPC_SC;           // System Control
  constexpr LPC_EMC_TypeDef* EMC = LPC_EMC;        // EMC
  
  constexpr LPC_ADC_TypeDef* IADC = LPC_ADC;       // Внутреннее АЦП
  
  constexpr LPC_DAC_TypeDef* DAC = LPC_DAC;        // DAC0
  constexpr LPC_PWM_TypeDef* PWM_DAC = LPC_PWM1;   // PWM1 DAC_PWM
  
  constexpr LPC_PWM_TypeDef* PULS_PWM = LPC_PWM0;  // PWM ИУ
  
  constexpr LPC_RTC_TypeDef* RTC = LPC_RTC;
  constexpr LPC_EEPROM_TypeDef* EEPROM = LPC_EEPROM;

}


class CPERIPHERIALS_INIT {

public:
  
  void initDOutputs() {      
    
    // Дискретные выходы
    P::G0->SET = (1UL << bg::B_ULED);
    P::G0->CLR = (!(1UL << bg::B_ULED));
    
    P::G1->CLR = 0xFFFFFFFF;
    P::G2->CLR = 0xFFFFFFFF;
    P::G3->CLR = 0xFFFFFFFF;
    P::G4->CLR = 0xFFFFFFFF;
    P::G5->CLR = 0xFFFFFFFF;
    
    // Настройка направления
    P::G0->DIR |= (1UL << bg::B_ULED);
    P::G1->DIR |= (1UL << bg::B_RelReady);
    P::G2->DIR |= (
                   1UL << bg::B_LampMeas | 
                   1UL << bg::B_LampAlarm1 | 
                   1UL << bg::B_LampAlarm2 | 
                   1UL << bg::B_TP | 
                   1UL << bg::B_TN |
                   1UL << bg::B_RelAlarm2 |
                   1UL << bg::B_RelAlarm1
                                 );
    P::G3->DIR = 0x00000000;
    P::G4->DIR = 0x00000000;
    P::G5->DIR = 0x00000000;
  }
  
  
  void powerON() {
    P::SC->PCONP |= CLKPWR_PCONP_PCPWM1;      
    P::SC->PCONP |= CLKPWR_PCONP_PCTIM0;      
    P::SC->PCONP |= CLKPWR_PCONP_PCUART0;  
    P::SC->PCONP |= CLKPWR_PCONP_PCUART2;        
    P::SC->PCONP |= CLKPWR_PCONP_PCSSP1;          
    P::SC->PCONP |= CLKPWR_PCONP_PCGPDMA;      
  }
  
  void initIOCON() {
    
    P::IOCON->P2_4  = bf::IOCON_PORT_PWM;                   // P2_4 -> PWM1:5 PWM_DAC1
    
    P::IOCON->P4_20 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SCK1
    P::IOCON->P4_21 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SSEL1
    P::IOCON->P4_22 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MISO1
    P::IOCON->P4_23 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MOSI1
    
    P::IOCON->P0_2 = bf::IOCON_U0_TXD;         // U0_TXD
    P::IOCON->P0_3 = bf::IOCON_U0_RXD;         // U0_RXD
    
    P::IOCON->P2_6 = bf::IOCON_U2_OE;          // U2_OE
    P::IOCON->P2_8 = bf::IOCON_U2_TXD;         // U2_TXD
    P::IOCON->P2_9 = bf::IOCON_U2_RXD;         // U2_RXD
    
  }
  
  void initTimers() {     
    P::SYST->PR =  6 - 1;  // 10 МГц, 1 тик = 0.1 мкс
    P::SYST->TCR |= 0x1;   // Включение системного таймера (TIM0)
  }
  
};

// Системный таймер
class SysT { public: inline static unsigned int TC() { return P::SYST->TC; } };
