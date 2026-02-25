#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc, CEEPSettings& rSet) : rAdc(rAdc), rSet(rSet) { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseP;
  mode = EMode::Work;
  wait_number = WAIT_NUMBER;
  pause_counter = 0;
  PN_On();
}

void CPROCESS::step() {
  
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
    if(R == Rmax) {
      if(pause_counter > wait_number / 2) {
        LampMeasOff();
      } else {
        LampMeasOn();
      }      
    }
    if(pause_counter > wait_number) {
      pause_counter = 0;      
      phases = EPhases::MeasP;
    }
    break; 
  case EPhases::PhaseN:  
    conv_adc();
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    if(R == Rmax) {
      if(pause_counter > wait_number / 2) {
        LampMeasOff();
      } else {
        LampMeasOn();
      }      
    }
    if(pause_counter > wait_number) {
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
    LampMeasOn();
    if(mode == EMode::Test) { LampAlarm1On(); LampAlarm2On(); }
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      PN_Off();
      calc_avr(ph);
      phases = EPhases::PhaseN;
      LampMeasOff();
      if(mode == EMode::Test) { LampAlarm1Off(); LampAlarm2Off(); }
    }
    break; 
  case EPhases::MeasN:  
    conv_adc();
    Ud_N[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak1_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    ILeak2_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    prev_TC0_Phase = LPC_TIM0->TC;
    pause_counter++;
    LampMeasOn();
    if(mode == EMode::Test) { LampAlarm1On(); LampAlarm2On(); }
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      if(mode == EMode::Work) { PN_On(); }      
      calc_avr(ph);
      phases = EPhases::PhaseP;
      LampMeasOff();
      if(mode == EMode::Test) { LampAlarm1Off(); LampAlarm2Off(); }
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
    if(mode == EMode::Test) { 
      test_Ud_avr = UdP_avr;
      test_ILeak1_avr = ILeak1P_avr;
      test_ILeak2_avr = ILeak2P_avr;
    } 
    else { 
      test_Ud_avr = 0;
      test_ILeak1_avr = 0;
      test_ILeak2_avr = 0;
    }
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
    if(mode == EMode::Test) { 
      test_Ud_avr = UdN_avr;
      test_ILeak1_avr = ILeak1N_avr;
      test_ILeak2_avr = ILeak2N_avr;
    } 
    else { 
      test_Ud_avr = 0;
      test_ILeak1_avr = 0;
      test_ILeak2_avr = 0;
    }
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
  
  float dIL = ILeak2N_avr - ILeak2P_avr;
  float pIL = ILeak2N_avr + ILeak2P_avr;
  float dUd = UdN_avr - UdP_avr;
  float pUd = UdN_avr + UdP_avr;
  
  float Ucor = (pIL * dUd) / (pIL * dUd - dIL * pUd);

  if(mode == EMode::Work) {
    float r = ((rSet.getSettings().k2Ls * (2.0f * Umeas * (1 - Ucor))) / dIL) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs2;
    if(r > Rmax) { R = Rmax; } else { R = r; }
    
    unsigned short gis = gis_const;
    if(R > range) gis = static_cast<unsigned short>(((gis_percent * R) / 100.0f) + 0.5f);
    
    if( R < rSet.getSettings().Rmin1) {
      LampAlarm1On();
      RelAlarm1On();
    } else if(R > (rSet.getSettings().Rmin1 + gis)) {
      LampAlarm1Off();
      RelAlarm1Off();
    }
    if( R < rSet.getSettings().Rmin2) {
      LampAlarm2On();
      RelAlarm2On();
    } else if(R > (rSet.getSettings().Rmin2 + gis)) {
      LampAlarm2Off();
      RelAlarm2Off();
    }
    
  } else if(mode == EMode::Test) { 
    R = 0; 
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
  wait_number = TEST_NUMBER;
  mode = EMode::Test; 
}

void CPROCESS::clr_test_mode() { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseP;
  PN_On();
  wait_number = WAIT_NUMBER;
  mode = EMode::Work; 
  LampAlarm1Off(); 
  LampAlarm2Off();
}
