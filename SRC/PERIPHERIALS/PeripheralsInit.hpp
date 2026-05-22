#pragma once
#include "LPC407x_8x_177x_8x.h"
#include "lpc177x_8x_clkpwr.h"
#include "Peripherals.hpp"
#include "ControlBits.hpp"

class CPERIPHERIALS_INIT {
private:
  CGPIO gpio0;
  CGPIO gpio1;
  CGPIO gpio2;
  CGPIO gpio3;
  CGPIO gpio4;
  CGPIO gpio5;
  
  CPCONP pconp;
  CIOCON iocon;
  CTIMER timer;
  
  CEXTINT extint;
  
public:
  CPERIPHERIALS_INIT() : 
    gpio0(P::G0), gpio1(P::G1), gpio2(P::G2), gpio3(P::G3), 
    gpio4(P::G4), gpio5(P::G5), iocon(P::IOCON), pconp(P::SC),
    timer(P::SYST, P::TIM1, P::TIM2, P::TIM3), extint(P::SC) {}
    
    void initDOutputs() {      
      
      // Дискретные выходы
      gpio0.set(1UL << bg::B_ULED);     
      gpio0.clr(!(1UL << bg::B_ULED));  
      
      gpio1.clr(0xFFFFFFFF);
      
      gpio2.clr(0xFFFFFFFF);
      gpio3.clr(0xFFFFFFFF);     
      gpio4.clr(0xFFFFFFFF);
      gpio5.clr(0xFFFFFFFF);
      
      // Настройка направления
      gpio0.dirOut(1UL << bg::B_ULED);          
      gpio1.dirOut(1UL << bg::B_RelReady);   
      gpio2.dirOut(
                   1UL << bg::B_LampMeas | 
                   1UL << bg::B_LampAlarm1 | 
                   1UL << bg::B_LampAlarm2 | 
                   1UL << bg::B_TP | 
                   1UL << bg::B_TN |
                   1UL << bg::B_RelAlarm2 |
                   1UL << bg::B_RelAlarm1
                     );
      gpio3.dirOut(0x00000000);
      gpio4.dirOut(0x00000000);
      gpio5.dirOut(0x00000008);
    }
    
    
    void powerON() {
      pconp.sc->PCONP |= CLKPWR_PCONP_PCPWM1;      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCTIM0;      
      pconp.sc->PCONP |= CLKPWR_PCONP_PCUART0;  
      pconp.sc->PCONP |= CLKPWR_PCONP_PCUART2;        
      pconp.sc->PCONP |= CLKPWR_PCONP_PCSSP1;          
      pconp.sc->PCONP |= CLKPWR_PCONP_PCGPDMA;      
    }
    
    void initIOCON() {

      iocon.base->P2_4  = bf::IOCON_PORT_PWM;                   // P2_4 -> PWM1:5 PWM_DAC1

      iocon.base->P4_20 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SCK1
      iocon.base->P4_21 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // SSEL1
      iocon.base->P4_22 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MISO1
      iocon.base->P4_23 = bf::D_MODE_PULLUP | bf::IOCON_SPI1;   // MOSI1
      
      iocon.base->P0_2 = bf::IOCON_U0_TXD;         // U0_TXD
      iocon.base->P0_3 = bf::IOCON_U0_RXD;         // U0_RXD
      
      iocon.base->P2_6 = bf::IOCON_U2_OE;          // U2_OE
      iocon.base->P2_8 = bf::IOCON_U2_TXD;         // U2_TXD
      iocon.base->P2_9 = bf::IOCON_U2_RXD;         // U2_RXD
      
    }
    
    void initTimers() {     
      timer.syst->PR =  6 - 1;  // 10 МГц, 1 тик = 0.1 мкс
      //timer.tim1->PR = 60 - 1;  // 1 МГц, 1 тик = 1.0 мкс
      //timer.tim2->PR = 60 - 1;  // 1 МГц
      //timer.tim3->PR = 60 - 1;  // 1 МГц   
      
      // Включение системного таймера (TIM0)
      timer.syst->TCR |= 0x1;
    }
    
};

