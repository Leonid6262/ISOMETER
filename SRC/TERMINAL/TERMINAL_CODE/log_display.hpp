#pragma once
#include "terminalUartDriver.hpp"
#include "pause_us.hpp"
#include "key_kodes.hpp"
#include "event_log.hpp"
#include <string>

class CTerminalManager;

class CLogDisplay { 
 
public: 
  CLogDisplay(CTerminalUartDriver&, CEVENT_LOG&);
  void get_key();
  void set_pTerminal(CTerminalManager*);
  void first_render();
  void sendLine(const std::string&, bool newline = false);
  
private:
  CTerminalUartDriver& uartDrv;  
  CEVENT_LOG& rEventLog;
  CTerminalManager* pTerminal_manager; 
  
  CEVENT_LOG::LogEntry local_buffer[CEVENT_LOG::Log_Length];    // Локальная копия лога 
  unsigned short local_count;                                   // Сколько реально записей скопировано
  unsigned short current_view;

  void Key_Handler(EKey_code);  
  void renderLogEntry();
  const char* getEventText(CEVENT_LOG::EEvent);
  
};

