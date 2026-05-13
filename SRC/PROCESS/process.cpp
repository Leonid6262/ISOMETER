#include "process.hpp" 
//#include "menu_factory.hpp" 
#include <math.h>

CPROCESS::CPROCESS(CADC& rAdc, CEEPSettings& rSet,CModbusDataProxy& rModbusData) : 
  rAdc(rAdc), rSet(rSet), rModbusData(rModbusData) {
  prev_TC0_Phase = LPC_TIM0->TC; 
  phases = EPhases::PhaseP;
  wait_number = WAIT_NUMBER;
  pause_counter = 0;
  RelReadyOn();
  clr_bs();
  bsWork(State::ON);
}

void CPROCESS::step() {
  
  dTrsPhase = LPC_TIM0->TC - prev_TC0_Phase;
  
  switch (phases) {
  case EPhases::PhaseP:
    LampMeasOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = LPC_TIM0->TC; wait(phases); } 
    break;   
  case EPhases::MeasP:
    LampMeasOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = LPC_TIM0->TC; conv(phases); }    
    break; 
  case EPhases::PhaseN:
    LampMeasOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = LPC_TIM0->TC; wait(phases); } 
    break;   
  case EPhases::MeasN:
    LampMeasOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = LPC_TIM0->TC; conv(phases); }    
    break;
  }
  
  update_modbus_data(); //--- Обновление данных для MoBus (F03, F04), запись по F06 ---
  
}

void CPROCESS::wait(EPhases ph) {
  switch (ph) {
  case EPhases::PhaseP:   
    if(++pause_counter > wait_number) {
      pause_counter = 0;      
      phases = EPhases::MeasP;
    }
    break; 
  case EPhases::PhaseN:  
    if(++pause_counter > wait_number) {
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

    ILeak1_P[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1));
    Ud_P[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak2_P[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    
    if(UStatus.sTest) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      Positive_phase();
      calc_avr(ph);
      phases = EPhases::PhaseN;
      if(UStatus.sTest) { LampAlarm1Off(); LampAlarm2Off(); }
    }
    break; 
  case EPhases::MeasN:  
    conv_adc();
    
    ILeak1_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    Ud_N[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak2_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    
    if(UStatus.sTest) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      Negative_phase();    
      calc_avr(ph);
      phases = EPhases::PhaseP;
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
  
  Ud_avr = ((UdN_avr + UdP_avr) * rSet.getSettings().k_adc[CADC::EADC_NameCh::Ud]) / 2.0f;
  
  float dIL1 = ILeak1N_avr - ILeak1P_avr;   
  float dIL2 = ILeak2N_avr - ILeak2P_avr;
  float dUd = 1000.0f * (UdN_avr - UdP_avr) * rSet.getSettings().k_adc[CADC::EADC_NameCh::Ud];

  if(UStatus.sWork) {
 
    R1 = (((rSet.getSettings().k1Ls * ((2 * u) + (dUd / 2.0f))) / dIL1) -  
          ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);// * rSet.getSettings().k_adc[static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)];
    R2 = (((rSet.getSettings().k2Ls * ((2 * u) + (dUd / 2.0f))) / dIL2) - 
          ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);// * rSet.getSettings().k_adc[static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)];
 
    if((ILeak2N_avr > 4080) || (ILeak2P_avr > 4080)){ R = R1; N_ch = 1; } 
    else { R = R2; N_ch = 2; }
    
    bool r_min = false;
    if((ILeak1N_avr >= 4080) || (ILeak1P_avr >= 4080)) r_min = true;
    
    if((R > Rmax) || (R < 0)) { 
      R = Rmax;
      bsMoreMax(State::ON);
    } else if(r_min) { 
      R = 0;
      bsLessMin(State::ON);
    } else {      
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
    
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1), 
    static_cast<unsigned char>(CADC::EADC_NameCh::Ud),
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2),
    
  });  
}

void CPROCESS::set_test_mode() { 
  phase_Off();
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
  Positive_phase();
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

void CPROCESS::update_modbus_data() {
  //--- Обновление данных для MoBus (F03, F04), запись по F06 ---
  if(rModbusData.getInstance().isDirty) {
    rModbusData.getInstance().isDirty = false;
    rSet.getSettings().RAlarm1 = rModbusData.getInstance().registers[CModbusDataProxy::RegAlarm1].value;
    rSet.getSettings().RAlarm2 = rModbusData.getInstance().registers[CModbusDataProxy::RegAlarm2].value;
  } 
  if(rModbusData.getInstance().registers[CModbusDataProxy::RegSaveEEP].value == CModbusDataProxy::SaveCode) {
    rModbusData.getInstance().registers[CModbusDataProxy::RegSaveEEP].value = 0;
    rSet.saveSettings();
  } 
  rModbusData.getInstance().registers[CModbusDataProxy::RegStatus].value = UStatus.all;
  rModbusData.getInstance().registers[CModbusDataProxy::RegR].value      = static_cast<unsigned short>(R + 0.5f);
  rModbusData.getInstance().registers[CModbusDataProxy::RegAlarm1].value = rSet.getSettings().RAlarm1;
  rModbusData.getInstance().registers[CModbusDataProxy::RegAlarm2].value = rSet.getSettings().RAlarm2;
}
