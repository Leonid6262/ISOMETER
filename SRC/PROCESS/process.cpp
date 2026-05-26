#include "process.hpp" 

#include <math.h>

CPROCESS::CPROCESS(CADC& rAdc, CEEPSettings& rSet, CModbusDataProxy& rModbusData, COUT_4_20& rCOUT_4_20, CRTC& rRTC, CEVENT_LOG& rEventLog) :   
  rAdc(rAdc), rSet(rSet), rModbusData(rModbusData), rCOUT_4_20(rCOUT_4_20), rRTC(rRTC), rEventLog(rEventLog) {   
  prev_TC0_Phase = SysT::TC(); 
  phases = EPhases::PhaseP;
  wait_number = WAIT_NUMBER;
  pause_counter = 0;
  clr_bs();
  bsWork(State::ON);
  bsNormRange(State::ON);
  prevUStatus = UStatus.all;
  start_test();
}

void CPROCESS::step() {
  
  dTrsPhase = SysT::TC() - prev_TC0_Phase; 
  
  
  switch (phases) {
  case EPhases::PhaseP:
    LampMeasOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); wait(phases); } 
    break;   
  case EPhases::MeasP:
    LampMeasOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); conv(phases); }    
    break; 
  case EPhases::PhaseN:
    LampMeasOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); wait(phases); } 
    break;   
  case EPhases::MeasN:
    LampMeasOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); conv(phases); }    
    break;
  }
  
  rRTC.update_now();                  // Обновление экземпляра структуы SDateTime данными из RTC
  auto now = rRTC.get_now();    
  snprintf(date_time, sizeof(date_time), "%02u.%02u.%02u %02u:%02u", now.day, now.month, now.year, now.hour, now.minute);   
  
  update_modbus_data(); // Обновление данных для MoBus (F03, F04), запись по F06
  rCOUT_4_20.update(static_cast<unsigned short>(R + 0.5f));  // Обновление 4...20мА
  
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
    
    if(UStatus.sFault) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      Positive_phase();
      calc_avr(ph);
      phases = EPhases::PhaseN;
      if(UStatus.sFault) { LampAlarm1Off(); LampAlarm2Off(); }
      if(!start_log) { 
        start_log = true; 
        rEventLog.clear_log(); 
      }
    }
    break; 
  case EPhases::MeasN:  
    conv_adc();
    
    ILeak1_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    Ud_N[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak2_N[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)); 
    
    if(UStatus.sFault) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > AVR_NUMBER - 1) {
      pause_counter = 0;
      Negative_phase();    
      calc_avr(ph);
      phases = EPhases::PhaseP;
      if(UStatus.sFault) { LampAlarm1Off(); LampAlarm2Off(); }
    }
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
}

void CPROCESS::calc_avr(EPhases ph) {
  signed int ud = 0;
  signed int ileak1 = 0;
  signed int ileak2 = 0;
  switch (ph) {
  case EPhases::MeasP:
    polarity = 'U';
    for(unsigned short n = sh_avr; n < (N_AVR + sh_avr); n++) {
      ud += Ud_P[n];
      ileak1 += ILeak1_P[n];
      ileak2 += ILeak2_P[n];
    }
    UdP_avr = static_cast<float>(ud) / N_AVR;
    ILeak1P_avr = static_cast<float>(ileak1) / N_AVR;
    ILeak2P_avr = static_cast<float>(ileak2) / N_AVR;
    break; 
  case EPhases::MeasN:
    polarity = 0xCF; //'П';
    for(unsigned short n = sh_avr; n < (N_AVR + sh_avr); n++) {
      ud += Ud_N[n];
      ileak1 += ILeak1_N[n];
      ileak2 += ILeak2_N[n];
    }
    UdN_avr = static_cast<float>(ud) / N_AVR;
    ILeak1N_avr = static_cast<float>(ileak1) / N_AVR;
    ILeak2N_avr = static_cast<float>(ileak2) / N_AVR;
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
  
  Ud_avr_d = lroundf((UdN_avr + UdP_avr) / 2.0f ) + rSet.getSettings().shift_Ud;
  Ud_avr_V = ((UdN_avr + UdP_avr + 2.0f * rSet.getSettings().shift_Ud) / 2.0f) * rSet.getSettings().k_Ud;
  
  dIL1 = ILeak1N_avr - ILeak1P_avr;    
  dIL2 = ILeak2N_avr - ILeak2P_avr;
  
  if(abs(round(UdN_avr - UdP_avr)) > 2) dUd = 1000.0f * (UdN_avr - UdP_avr) * rSet.getSettings().k_Ud;
  else dUd = 0; 
 
  if(!rSet.getSettings().comp_dUd) dUd = 0;
  
  if(UStatus.sWork) {
 
    R1 = (((rSet.getSettings().k_ch1 * ((2 * u) + (dUd / 2.0f))) / dIL1) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);
    R2 = (((rSet.getSettings().k_ch2 * ((2 * u) + (dUd / 2.0f))) / dIL2) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);
 
    float r;
    if((ILeak2N_avr > d_max) || (ILeak2P_avr > d_max)){ r = R1; N_ch  = '1'; }  
    else { r = R2; N_ch = '2'; }
    
    char r_buf[10];
    if((r > Rmax) || (r < 0)) { 
      R = Rmax;
      bsMoreMax(State::ON);
      snprintf(r_buf, sizeof(r_buf), "R>%uk", Rmax);
      SaveEvent(CEVENT_LOG::EEvent::MoreMax);
    } else if((ILeak1N_avr >= d_max) || (ILeak1P_avr >= d_max)) { 
      R = 0;
      bsLessMin(State::ON);
      snprintf(r_buf, sizeof(r_buf), "R<Rmin");
      SaveEvent(CEVENT_LOG::EEvent::LessMin);
    } else {      
      R = round(r);
      bsMoreMax(State::OFF);
      bsLessMin(State::OFF);
      bsNormRange(State::ON);
      snprintf(r_buf, sizeof(r_buf), "R=%uk", R);     
      SaveEvent(CEVENT_LOG::EEvent::NormRange);
    }
    
    rRTC.update_now();
    auto now = rRTC.get_now();
    snprintf(RES, sizeof(RES), "%-7s %c%c %02u:%02u", 
         r_buf, 
         N_ch, 
         polarity, 
         now.hour, 
         now.minute);    
    
    unsigned short gis = gis_const;
    if(R > range) gis = static_cast<unsigned short>(((gis_percent * R) / 100.0f) + 0.5f);
    
    if( R < rSet.getSettings().RAlarm1) {
      LampAlarm1On();
      RelAlarm1On();
      bsAlarm1(State::ON);    
      SaveEvent(CEVENT_LOG::EEvent::Alarm1_On);
    } else if(R > (rSet.getSettings().RAlarm1 + gis)) {
      LampAlarm1Off();
      RelAlarm1Off();
      bsAlarm1(State::OFF);
      SaveEvent(CEVENT_LOG::EEvent::Alarm1_Off);
    }
    if( R < rSet.getSettings().RAlarm2) {
      LampAlarm2On();
      RelAlarm2On();
      bsAlarm2(State::ON);
      SaveEvent(CEVENT_LOG::EEvent::Alarm2_On);
    } else if(R > (rSet.getSettings().RAlarm2 + gis)) {
      LampAlarm2Off();
      RelAlarm2Off();
      bsAlarm2(State::OFF);
      SaveEvent(CEVENT_LOG::EEvent::Alarm2_Off);
    }
    
  } 
  
  if(UStatus.sFault) { 
    R = 0;
    RelAlarm1Off();
    RelAlarm2Off();
    RelReadyOff();
    SaveEvent(CEVENT_LOG::EEvent::Fault_On);
  } else {
    SaveEvent(CEVENT_LOG::EEvent::Fault_Off);
  }
  
}

void CPROCESS::SaveEvent(CEVENT_LOG::EEvent event) {
  if((prevUStatus != UStatus.all) & start_log) {
    prevUStatus = UStatus.all;
    rEventLog.save_event(event);
  }  
}

void CPROCESS::conv_adc() {
  rAdc.conv_tnf({     
    
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1), 
    static_cast<unsigned char>(CADC::EADC_NameCh::Ud),
    static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2),
    
  });  
}

void CPROCESS::start_test() { 
  CPROCESS::LampMeasOn();
  RelAlarm1On();
  Pause_us(500000); 
  CPROCESS::LampMeasOff();
  RelAlarm1Off();
  
  CPROCESS::LampAlarm1On();
  RelAlarm2On();
  Pause_us(500000);
  CPROCESS::LampAlarm1Off();
  RelAlarm2Off();
  
  CPROCESS::LampAlarm2On(); 
  RelReadyOn();
  Pause_us(500000);
  CPROCESS::LampAlarm2Off();
  RelReadyOff();
  
  Pause_us(500000);
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
