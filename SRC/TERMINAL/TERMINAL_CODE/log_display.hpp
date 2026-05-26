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
  
private:
  CTerminalUartDriver& uartDrv;  
  CEVENT_LOG& rEventLog;
  CTerminalManager* pTerminal_manager; 
  
  unsigned short current_index;
  bool wrapped;
  unsigned short current_view;
  unsigned short TOP;
  unsigned short END;
  unsigned short count;
  unsigned short Log_Length;
  
  void sendLine(const std::string&, bool newline = false);
  void Key_Handler(EKey_code);  
  unsigned short calc_cur_view(); 
  void renderLogEntry();
  const char* getEventText(CEVENT_LOG::EEvent);
  
};

