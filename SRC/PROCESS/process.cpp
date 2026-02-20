#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc) : rAdc(rAdc) { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseN;
  mode = EMode::Work;
}

void CPROCESS::step() {
  
  if(mode == EMode::Test) { conv_adc(); return; }
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  
  switch (phases) {
  case EPhases::PhaseP:
    if(dTrsPhase > WAIT_PERIOD) { 
      prev_TC0_Phase = LPC_TIM0->TC;
      phases = EPhases::MeasP;
      ind_avr = 0;
    }
    break;   
  case EPhases::MeasP:   
    if(dTrsPhase > MEAS_PAUSED) { conv(phases); }    
    break; 
  case EPhases::PhaseN:         
    if(dTrsPhase > WAIT_PERIOD) {
      prev_TC0_Phase = LPC_TIM0->TC;
      phases = EPhases::MeasN;
      ind_avr = 0;
    }    
    break;   
  case EPhases::MeasN:  
    if(dTrsPhase > MEAS_PAUSED) { conv(phases); }    
    break;
  }
  
}

void CPROCESS::conv(EPhases ph) {
  switch (ph) {
  case EPhases::MeasP:   
    conv_adc();
    Ud_P[ind_avr]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak1_P[ind_avr] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    ILeak2_P[ind_avr] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    prev_TC0_Phase = LPC_TIM0->TC;
    ind_avr++;
    if(ind_avr > AVR_NUMBER - 1) {
      PN_On();
      calc_avr(ph);
      phases = EPhases::PhaseN;
    }
    break; 
  case EPhases::MeasN:  
    conv_adc();
    Ud_N[ind_avr]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak1_N[ind_avr] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    ILeak2_N[ind_avr] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    prev_TC0_Phase = LPC_TIM0->TC;
    ind_avr++;
    if(ind_avr > AVR_NUMBER - 1) {
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
    for(unsigned short n = 0+5; n < 500+5; n++) {
      ud += Ud_P[n];
      ileak1 += ILeak1_P[n];
      ileak2 += ILeak2_P[n];
    }
    UdP_avr = ud / 500.0f;
    ILeak1P_avr = ileak1 / 500.0f;
    ILeak2P_avr = ileak2 / 500.0f;
    break; 
  case EPhases::MeasN:  
    for(unsigned short n = 0+5; n < 500+5; n++) {
      ud += Ud_N[n];
      ileak1 += ILeak1_N[n];
      ileak2 += ILeak2_N[n];
    }
    UdN_avr = ud / 500.0f;
    ILeak1N_avr = ileak1 / 500.0f;
    ILeak2N_avr = ileak2 / 500.0f;
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
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
