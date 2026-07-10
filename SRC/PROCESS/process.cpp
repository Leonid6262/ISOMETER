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
    if(UStatus.sWork) LampReadyOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); wait(phases); } 
    break;   
  case EPhases::MeasP:
    LampReadyOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); conv(phases); }    
    break; 
  case EPhases::PhaseN:
    if(UStatus.sWork) LampReadyOn();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); wait(phases); } 
    break;   
  case EPhases::MeasN:
    LampReadyOff();
    if(dTrsPhase > MEAS_PAUSED) { prev_TC0_Phase = SysT::TC(); conv(phases); }    
    break;
  }
  
  rRTC.update_now();                  // Обновление экземпляра структуы SDateTime данными из RTC
  auto now = rRTC.get_now();    
  snprintf(date_time, sizeof(date_time), "%02u.%02u.%02u %02u:%02u", now.day, now.month, now.year, now.hour, now.minute);   
  
  update_modbus_data(); // Обновление данных для MoBus (F03, F04), запись по F06
  
  // Обновление 4...20мА
  if(test_4_20){
    rCOUT_4_20.update(R_Test);  
  }else{
    rCOUT_4_20.update(static_cast<unsigned short>(R + 0.5f));
  }
  
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

    ILeak1[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1));
    Ud[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak2[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2));
    if(pause_counter < N_AVR_5V){
      P5V[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::P5V));
      N5V[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::N5V));
    }
       
    if(UStatus.sFault) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > N_AVR - 1) {
      pause_counter = 0;
      CP40V_PN = get_С_P40V();
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
    
    ILeak1[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)); 
    Ud[pause_counter]     = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)); 
    ILeak2[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2));
    if(pause_counter < N_AVR_5V){
      P5V[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::P5V));
      N5V[pause_counter] = rAdc.getData(static_cast<unsigned char>(CADC::EADC_NameCh::N5V));
    }
    
    if(UStatus.sFault) { LampAlarm1On(); LampAlarm2On(); }
    if(++pause_counter > N_AVR - 1) {
      pause_counter = 0;
      CP40V_PN = get_С_P40V();
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
  signed int p5v = 0;
  signed int n5v = 0;
  switch (ph) {
  case EPhases::MeasP:
    polarity = 'U';
    for(unsigned short n = 0; n < N_AVR; n++) {
      ud += Ud[n];
      ileak1 += ILeak1[n];
      ileak2 += ILeak2[n];
    }
    for(unsigned short n = 0; n < N_AVR_5V; n++) {
      p5v += P5V[n];
      n5v += N5V[n];
    }
    UdP_avr = static_cast<float>(ud) / N_AVR;
    ILeak1P_avr = static_cast<float>(ileak1) / N_AVR;
    ILeak2P_avr = static_cast<float>(ileak2) / N_AVR;
    P5V_avr = static_cast<float>(p5v) / N_AVR_5V;
    N5V_avr = static_cast<float>(n5v) / N_AVR_5V;
    break; 
  case EPhases::MeasN:
    polarity = 0xCF; //'П';
    for(unsigned short n = 0; n < N_AVR; n++) {
      ud += Ud[n];
      ileak1 += ILeak1[n];
      ileak2 += ILeak2[n];
    }
    for(unsigned short n = 0; n < N_AVR_5V; n++) {
      p5v += P5V[n];
      n5v += N5V[n];
    }
    UdN_avr = static_cast<float>(ud) / N_AVR;
    ILeak1N_avr = static_cast<float>(ileak1) / N_AVR;
    ILeak2N_avr = static_cast<float>(ileak2) / N_AVR;
    P5V_avr = static_cast<float>(p5v) / N_AVR_5V;
    N5V_avr = static_cast<float>(n5v) / N_AVR_5V; 
    break; 
  case EPhases::PhaseP:
  case EPhases::PhaseN:
    break;
  }
  
  
  P5_avr_V = K_P5 * P5V_avr;
  N5_avr_V = (K_N5 * N5V_avr) - (3 * P5_avr_V);
  
  if(P5_avr_V < P5_MIN) bsFaultP5(State::ON);
  else bsFaultP5(State::OFF);
 
  if(N5_avr_V > N5_MIN) bsFaultN5(State::ON);
  else bsFaultN5(State::OFF);
  
  //////////if(!CP40V_PN) bsFaultV40(State::ON);
  //////////else bsFaultV40(State::OFF);
  
  if(UStatus.sFaultP5 || UStatus.sFaultN5 || UStatus.sFaultS40) {
    bsWork(State::OFF);
    bsFault(State::ON);
  }
  else {
    bsWork(State::ON);
    bsFault(State::OFF);
  }
   
  Ud_avr_d = lroundf((UdN_avr + UdP_avr) / 2.0f ) + rSet.getSettings().shift_Ud;
  Ud_avr_V = ((UdN_avr + UdP_avr + 2.0f * rSet.getSettings().shift_Ud) / 2.0f) * rSet.getSettings().k_Ud;
  
  dIL1 = ILeak1N_avr - ILeak1P_avr;    
  dIL2 = ILeak2N_avr - ILeak2P_avr; 
  pIL1 = ILeak1N_avr + ILeak1P_avr - 4095 + rSet.getSettings().shift_ch1;    
  pIL2 = ILeak2N_avr + ILeak2P_avr - 4095 + rSet.getSettings().shift_ch2;
  
  ch1_4_shift = static_cast<signed short>(round(pIL1 / 2.0f)); 
  ch2_4_shift = static_cast<signed short>(round(pIL2 / 2.0f));  
  
  dUd = UdN_avr - UdP_avr;
  pUd = UdN_avr + UdP_avr;
  
  float ucor1 = 1 - ((pIL1 * dUd) / ((pIL1 * dUd) - (dIL1 * pUd)));
  float ucor2 = 1 - ((pIL2 * dUd) / ((pIL2 * dUd) - (dIL2 * pUd)));
  
  if (isnan(ucor1)) { Ucor1 = 1.0f; } else { Ucor1 = ucor1; } 
  if (isnan(ucor2)) { Ucor2 = 1.0f; } else { Ucor2 = ucor2; }
  
  if(!rSet.getSettings().comp_dUd)  { Ucor1 = 1; Ucor2 = 1; }
  
  bool top_ch2 = false;
  if(((ILeak2N_avr > d_max) || (ILeak2P_avr > d_max)) || ((ILeak2N_avr < d_min) || (ILeak2P_avr < d_min))) { top_ch2 = true; } 
  
  if(UStatus.sWork) {
    float r;
    R1 = (((rSet.getSettings().k_ch1 * ((2 * u * (Ucor1)))) / dIL1) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);
    R2 = (((rSet.getSettings().k_ch2 * ((2 * u * (Ucor2)))) / dIL2) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);
    
    if(R2 > 1000){
      float dIL2_cor = dIL2 * ( 1.0f / (1.0f + ( 1.0f / (rSet.getSettings().k_cor_ch2 * dIL2))));
      R2 = (((rSet.getSettings().k_ch2 * ((2 * u * (Ucor2)))) / dIL2_cor) - ((RT + rSet.getSettings().RTadd) / 2.0f) - Rs);
    }
    
    
    if(R1<0 && R2>0 && top_ch2 ) 
    {
      r=0;
    }
    else if(R1<0 || R2<0 && !top_ch2)
    {
      r=Rmax;
    } 
    else if(R1>0 && R2>0)
    {
      if(top_ch2) { r = R1; N_ch  = '1'; }  
      else { r = R2; N_ch = '2'; }
    }
    if(abs(r)<1.0f) {
      r=0;
    }

    char r_buf[10];
    if((r > Rmax)) { 
      R = Rmax;
      bsMoreMax(State::ON);
      bsNormRange(State::OFF);
      snprintf(r_buf, sizeof(r_buf), "R>%uk", Rmax);
      SaveEvent(CEVENT_LOG::EEvent::MoreMax);
    } else {  
      R = round(r);
      bsMoreMax(State::OFF);
      bsLessMin(State::OFF);
      bsNormRange(State::ON);
      
      if(R == 0) {
        if(Ud_avr_V > 10){
          if(pIL1 > 0){
            snprintf(r_buf, sizeof(r_buf), "S/C -L");
          }else if(pIL1 < 0){
            snprintf(r_buf, sizeof(r_buf), "S/C +L");
          }else if(pIL1 == 0){
            snprintf(r_buf, sizeof(r_buf), "SHORT");
          }
        }else{
          snprintf(r_buf, sizeof(r_buf), "SHORT");
        }        
      }else {
        snprintf(r_buf, sizeof(r_buf), "R=%uk", R);
      } 
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
    
  } else if(UStatus.sFault) { 
    R = 0;
    RelAlarm1Off();
    RelAlarm2Off();
    RelReadyOff();
    LampAlarm1Off();
    LampAlarm2Off();
    LampReadyOff();
    CPROCESS::UserLedOn();
    SaveEvent(CEVENT_LOG::EEvent::Fault_On);
    
    if(UStatus.sFaultP5) {
      snprintf(RES, sizeof(RES), "   FAULT P5     ");    
    }else if(UStatus.sFaultN5) {
      snprintf(RES, sizeof(RES), "   FAULT N5     ");   
    }else if(UStatus.sFaultS40) {
      snprintf(RES, sizeof(RES), "   FAULT P50    ");   
    }
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
    static_cast<unsigned char>(CADC::EADC_NameCh::P5V),
    static_cast<unsigned char>(CADC::EADC_NameCh::N5V)
      
  });  
}

void CPROCESS::start_test() { 
  CPROCESS::LampReadyOn();
  RelAlarm1On();
  Pause_us(500000); 
  CPROCESS::LampReadyOff();
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

