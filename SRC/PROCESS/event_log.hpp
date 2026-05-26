#pragma once

#include "settings_eep.hpp"
#include "rtc.hpp"

class CPROCESS;

class CEVENT_LOG { 
  
public:
  CPROCESS* pProcess;
  unsigned short current_index = 0;
  bool log_wrapped = false; // Флаг того, что буфер заполнился и пошел на второй круг
  static constexpr unsigned short Log_Length  = 200;
  
  enum class EEvent {
    NoEvents,           // Нет событий
    LessMin,            // Rmin
    MoreMax,            // Rmax
    NormRange,          // R в пределах диапозона
    Alarm1_On,          // 1-е предупреждение          
    Alarm1_Off,         // Снято 1-е предупреждение 
    Alarm2_On,          // 2-е предупреждение             
    Alarm2_Off,         // Снято 2-е предупреждение
    Fault_On,           // Неисправность
    Fault_Off,          // Неисправность снята
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
  
  LogEntry log_buffer[Log_Length];
  
private: 
  

  
};

