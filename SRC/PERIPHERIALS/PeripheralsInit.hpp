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
  
  constexpr LPC_SSP_TypeDef* SPI1 = LPC_SSP1;
  
  constexpr LPC_GPDMA_TypeDef*  GPDMA = LPC_GPDMA;
    
  constexpr LPC_IOCON_TypeDef* IOCON = LPC_IOCON; 
  
  constexpr LPC_SC_TypeDef* SC = LPC_SC;           
  
  constexpr LPC_EMC_TypeDef* EMC = LPC_EMC;        

  constexpr LPC_PWM_TypeDef* PWM_DAC = LPC_PWM1;  
  
  constexpr LPC_RTC_TypeDef* RTC = LPC_RTC;
  
  constexpr LPC_EEPROM_TypeDef* EEPROM = LPC_EEPROM;

}

class CPERIPHERIALS_INIT {

public:
  
  void initDOutputs() {  
    
    // Дискретные выходы
    
    P::G1->SET = 0xFFFFFFFF; 
    P::G1->CLR = (1UL << bg::B_ULED) | (1UL << bg::B_TP) | (1UL << bg::B_TN); // ULED - On, Bridge - Off !!!
    P::G2->CLR = 0xFFFFFFFF;
    P::G3->CLR = 0xFFFFFFFF;
    P::G4->CLR = 0xFFFFFFFF;
    P::G5->CLR = 0xFFFFFFFF;
    
    // Настройка направления на вывод
    P::G1->DIR |= (1UL << bg::B_ULED);          
    
    P::G1->DIR |= (1UL << bg::B_RelReady);      
    P::G1->DIR |= (1UL << bg::B_RelAlarm1);     
    P::G1->DIR |= (1UL << bg::B_RelAlarm2);     
    
    P::G1->DIR |= (1UL << bg::B_TP);            
    P::G1->DIR |= (1UL << bg::B_TN);            
    
    P::G1->DIR |= (1UL << bg::B_LampReady);     
    P::G1->DIR |= (1UL << bg::B_LampAlarm1);    
    P::G1->DIR |= (1UL << bg::B_LampAlarm2);    
          
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
    
    P::IOCON->P1_24  = bf::IOCON_PORT_PWM;                  // P1_24 -> PWM1:5 PWM_DAC1
    
    P::IOCON->P1_31 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SCK1
    P::IOCON->P0_14 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SSEL1
    P::IOCON->P0_12 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MISO1
    P::IOCON->P0_13 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MOSI1
    
    P::IOCON->P0_0 = bf::IOCON_U0_TXD;          // U0_TXD
    P::IOCON->P0_1 = bf::IOCON_U0_RXD;          // U0_RXD
    
    P::IOCON->P1_19 = bf::IOCON_U2_OE;          // U2_OE
    P::IOCON->P0_10 = bf::IOCON_U2_TXD;         // U2_TXD
    P::IOCON->P0_11 = bf::IOCON_U2_RXD;         // U2_RXD
    
    // Настройка RMII пинов
    P::IOCON->P1_0 = bEMC::FUNC_ENET;                           // ENET_TXD0
    P::IOCON->P1_1 = bEMC::FUNC_ENET;                           // ENET_TXD1
    P::IOCON->P1_4 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;      // ENET_TX_EN
    P::IOCON->P1_8 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;      // ENET_CRS_DV
    P::IOCON->P1_9 = bEMC::FUNC_ENET;                           // ENET_RXD0
    P::IOCON->P1_10 = bEMC::FUNC_ENET;                          // ENET_RXD1
    P::IOCON->P1_14 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;     // RXER
    P::IOCON->P1_15 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;     // RXCK
    P::IOCON->P1_16 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;     // MDC
    P::IOCON->P1_17 |= bEMC::FUNC_ENET | bEMC::MODE_PULLUP;     // MDIO
    
  }
  
  void initTimers() {     
    P::SYST->PR =  6 - 1;  // 10 МГц, 1 тик = 0.1 мкс
    P::SYST->TCR |= 0x1;   // Включение системного таймера (TIM0)
  }
  
};

// Системный таймер
class SysT { public: inline static unsigned int TC() { return P::SYST->TC; } };
