#include "terminal_manager.hpp"

CTerminalManager::CTerminalManager(CMenuNavigation& rMenuNavigation, CLogDisplay& rLogDisplay)  
: rMenuNavigation(rMenuNavigation), rLogDisplay(rLogDisplay), mode(ETerminalMode::Menu_mode) {} 

void CTerminalManager::dispatch() {
  switch (mode) {
  case ETerminalMode::Menu_mode:
    rMenuNavigation.get_key();
    break;
  case ETerminalMode::Log_mode:
    rLogDisplay.get_key();
    break;
  }
}

void CTerminalManager::switchToMenu() { 
  mode = ETerminalMode::Menu_mode; 
  rMenuNavigation.prev_TC0 = SysT::TC() - CMenuNavigation::DISPLAY_PERIOD_TICKS;
  rMenuNavigation.first_render();
}
void CTerminalManager::switchToLog() {
  mode = ETerminalMode::Log_mode; 
}
