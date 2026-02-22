#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc, CEEPSettings& rSet) : rAdc(rAdc), rSet(rSet) { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseN;
  mode = EMode::Work;
  pause_counter = 0;
  PN_On();
}

void CPROCESS::step() {
  
  if(mode == EMode::Test) { conv_adc(); return; }
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  
  switch (phases) {
  case EPhases::PhaseP:
    if(dTrsPhase > MEAS_PAUSED) { wait(phases); } 
    break;   
  case EPhases::MeasP:   
    if(dTrsPhase > MEAS_PAUSED) { conv(phases); }    
    break; 
  case EPhases::PhaseN:         
    if(dTrsPhase > MEAS_PAUSED) { wait(phases); } 
    break;   
  case EPhases::MeasN:  
    if(dTrsPhase > MEAS_PAUSED) { conv(phases); }    
    break;
  }
  
}

void CPROCESS::wait(EPhases ph) {
  switch (ph) {
  case EPhases::PhaseP:   
    conv_adc();
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    if(pause_counter > WAIT_NUMBER) {
      pause_counter = 0;      
      phases = EPhases::MeasP;
    }
    break; 
  case EPhases::PhaseN:  
    conv_adc();
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    if(pause_counter > WAIT_NUMBER) {
      pause_counter = 0;
      phases = EPhases::MeasN;
    }
    break; 
  case EPhases::MeasP:
  case EPhases::MeasN:
    break;
  }
}

void CPROCESS::conv(EPhases ph) {
  switch (ph) {
  case EPhases::MeasP:   
    conv_adc();
    Ud_P[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak1_P[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    ILeak2_P[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      PN_On();
      calc_avr(ph);
      phases = EPhases::PhaseN;
    }
    break; 
  case EPhases::MeasN:  
    conv_adc();
    Ud_N[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak1_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    ILeak2_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      PN_Off();
      calc_avr(ph);
      phases = EPhases::PhaseP;
    }
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
}

void CPROCESS::calc_avr(EPhases ph) {
  float ud = 0;
  float ileak1 = 0;
  float ileak2 = 0;
  switch (ph) {
  case EPhases::MeasP:     
    for(unsigned short n = sh_avr; n < (N_AVR + sh_avr); n++) {
      ud += Ud_P[n];
      ileak1 += ILeak1_P[n];
      ileak2 += ILeak2_P[n];
    }
    UdP_avr = ud / N_AVR;
    ILeak1P_avr = ileak1 / N_AVR;
    ILeak2P_avr = ileak2 / N_AVR;
    break; 
  case EPhases::MeasN:  
    for(unsigned short n = sh_avr; n < (N_AVR + sh_avr); n++) {
      ud += Ud_N[n];
      ileak1 += ILeak1_N[n];
      ileak2 += ILeak2_N[n];
    }
    UdN_avr = ud / N_AVR;
    ILeak1N_avr = ileak1 / N_AVR;
    ILeak2N_avr = ileak2 / N_AVR;
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
  Ud = rSet.getSettings().kUds * (UdP_avr + UdN_avr) / 2.0f;
  R = ((rSet.getSettings().k2Ls * 2.0f * Umeas) / (ILeak2P_avr - ILeak2N_avr)) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs2; 
  //R = ((rSet.getSettings().k1Ls * 2.0f * Umeas) / (ILeak1P_avr - ILeak1N_avr)) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs1;  
  //R = (20000.0f/(0.7f * (ILeak2P_avr - ILeak2N_avr) / 2.0f)) - 23.3f;
}

void CPROCESS::conv_adc() {
  rAdc.conv_tnf({               
    static_cast<unsigned char>(CADC::EADC_NameCh::Ud),
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1), 
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2),
  });  
}

void CPROCESS::set_test_mode() { 
  PN_Off(); 
  mode = EMode::Test; 
}

void CPROCESS::clr_test_mode() { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseN; 
  mode = EMode::Work; 
}
