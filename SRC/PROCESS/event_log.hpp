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
  
  struct LogEntry {
    unsigned char day;
    unsigned char month;
    unsigned char year;
    unsigned char hour;
    unsigned char minute;
    EEvent event;
    unsigned short R_val;
  };
  
  void clear_log();
  void save_event(EEvent);
  
  void get_pProcess(CPROCESS*);  
  
  CEVENT_LOG();
  
private: 
  
  static constexpr unsigned short Log_Length  = 200;
  
  LogEntry log_buffer[Log_Length];
    
  unsigned short current_index = 0; 
  bool log_wrapped = false; // Флаг того, что буфер заполнился и пошел на второй круг
  
};

