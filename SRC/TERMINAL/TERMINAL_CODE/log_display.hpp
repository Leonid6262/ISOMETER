#pragma once
#include "terminalUartDriver.hpp"
#include <string>
#include "pause_us.hpp"

class CTerminalManager;

class CLogDisplay { 
 
public: 
  CLogDisplay(CTerminalUartDriver&);
  void get_key();
  void set_pTerminal(CTerminalManager*);
  
  unsigned int prev_TC0;
  static constexpr unsigned int MESSAGE_PERIOD_TICKS = 20000000; // 2 сек
  
private:
  CTerminalUartDriver& uartDrv;  
  CTerminalManager* pTerminal_manager;    
    
  enum class EKey_code { 
    NONE = 0x00, 
    ESCAPE = 0x1B, 
    FnEsc = 0x79,
    START = 0x70,
    STOP  = 0x2A
  };
  
  static constexpr signed char data_time = -1;
  static constexpr bool newline = true;
  static constexpr unsigned char disp_l = 16;
  
  void render_messages(signed char, bool); 
  void sendLine(const std::string&, bool newline = false);
  void Key_Handler(EKey_code);  
  
};

