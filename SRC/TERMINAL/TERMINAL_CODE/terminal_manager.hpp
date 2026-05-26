#pragma once
#include "process.hpp"
#include "menu_navigation.hpp"
#include "log_display.hpp"

class CTerminalManager {
public:
  CTerminalManager(CMenuNavigation&, CLogDisplay&);
  CMenuNavigation& rMenuNavigation;
  
  void dispatch();
  void switchToMenu();
  void switchToLog();
  
private:
  enum class ETerminalMode { Menu_mode, Log_mode } mode;
  CLogDisplay& rLogDisplay;
};

enum class ELED { 
  LED_GREEN  = 0x02, 
  LED_BLUE   = 0x03,
  LED_YELLOW = 0x04,
  LED_OFF = 0x0B 
};
