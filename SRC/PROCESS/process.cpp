#include "process.hpp" 

CPROCESS::CPROCESS(CADC& rAdc, CEEPSettings& rSet) : rAdc(rAdc), rSet(rSet) { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseP;
  wait_number = WAIT_NUMBER;
  pause_counter = 0;
  RelReadyOn();
  PN_On();
  clr_bs();
  bsWork(State::ON);
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
    if(R == rSet.getSettings().Rmax) {
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
    if(R == rSet.getSettings().Rmax) {
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
    if(UStatus.sTest) { LampAlarm1On(); LampAlarm2On(); }
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      PN_Off();
      calc_avr(ph);
      phases = EPhases::PhaseN;
      LampMeasOff();
      if(UStatus.sTest) { LampAlarm1Off(); LampAlarm2Off(); }
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
    if(UStatus.sTest) { LampAlarm1On(); LampAlarm2On(); }
    if(pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      if(UStatus.sWork) { PN_On(); }      
      calc_avr(ph);
      phases = EPhases::PhaseP;
      LampMeasOff();
      if(UStatus.sTest) { LampAlarm1Off(); LampAlarm2Off(); }
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
    if(UStatus.sTest) { 
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
    if(UStatus.sTest) { 
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

  if(UStatus.sWork) {
    float r = ((rSet.getSettings().k2Ls * (2.0f * Umeas * (1 - Ucor))) / dIL) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs2;
    
    if(r > rSet.getSettings().Rmax) { 
      R = rSet.getSettings().Rmax;
      bsMoreMax(State::ON);
    } else if(r < rSet.getSettings().Rmin) { 
      R = rSet.getSettings().Rmin;
      bsLessMin(State::ON);
    } else { 
      R = r; 
      bsMoreMax(State::OFF);
      bsLessMin(State::OFF);
    }
    
    unsigned short gis = gis_const;
    if(R > range) gis = static_cast<unsigned short>(((gis_percent * R) / 100.0f) + 0.5f);
    
    if( R < rSet.getSettings().RAlarm1) {
      LampAlarm1On();
      RelAlarm1On();
      bsAlarm1(State::ON);      
    } else if(R > (rSet.getSettings().RAlarm1 + gis)) {
      LampAlarm1Off();
      RelAlarm1Off();
      bsAlarm1(State::OFF);
    }
    if( R < rSet.getSettings().RAlarm2) {
      LampAlarm2On();
      RelAlarm2On();
      bsAlarm2(State::ON);
    } else if(R > (rSet.getSettings().RAlarm2 + gis)) {
      LampAlarm2Off();
      RelAlarm2Off();
      bsAlarm2(State::OFF);
    }
    
  } else if(UStatus.sTest) { 
    R = 0;
    if(testRelAlarm1 == State::ON) {
      RelAlarm1On();
    } else {
      RelAlarm1Off();
    }
    if(testRelAlarm2 == State::ON) {
      RelAlarm2On();
    } else {
      RelAlarm2Off();
    }
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
  testRelAlarm1 = State::OFF;
  testRelAlarm2 = State::OFF;
  RelAlarm1Off();
  RelAlarm2Off();
  RelReadyOff();
  wait_number = TEST_NUMBER;
  clr_bs();
  bsTest(State::ON); 
}

void CPROCESS::clr_test_mode() { 
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseP;
  PN_On();
  wait_number = WAIT_NUMBER;
  clr_bs();
  bsWork(State::ON);
  LampAlarm1Off(); 
  LampAlarm2Off();
  testRelAlarm1 = State::OFF;
  testRelAlarm2 = State::OFF;
  RelAlarm1Off();
  RelAlarm2Off();
  RelReadyOn();
}
