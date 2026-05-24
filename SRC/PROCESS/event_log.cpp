#include "event_log.hpp"
#include "process.hpp" 

CEVENT_LOG::CEVENT_LOG() {}

void CEVENT_LOG::get_pProcess(CPROCESS* p) {
  pProcess = p;
}

void CEVENT_LOG::clear_log() {
  
}

void CEVENT_LOG::save_event(EEvent event, unsigned short Rcur) { 
  pProcess->rRTC.update_now();                 
  auto now = pProcess->rRTC.get_now();
  float Rc = pProcess->R;
}
