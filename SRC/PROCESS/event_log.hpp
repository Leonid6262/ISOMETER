#pragma once

#include "settings_eep.hpp"
#include "rtc.hpp"

class CPROCESS;

class CEVENT_LOG { 
  
public:
  CPROCESS* pProcess;
  
  enum class EEvent {
    Start_Log,
    Alarm1_On,              
    Alarm1_Off,
    Alarm2_On,              
    Alarm2_Off,
  };
  
  void clear_log();
  void save_event(EEvent, unsigned short);
  void get_pProcess(CPROCESS*);  
  
  CEVENT_LOG();
  
private: 
  
  static constexpr unsigned short Log_Length  = 200;
};

