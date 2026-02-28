#include "mb_slave.hpp" 

CMBSLAVE::CMBSLAVE(CMbUartDriver& rUartDrv, CDMAcontroller& rDMAc) : rUartDrv(rUartDrv), rDMAc(rDMAc) {}

StatusTO CMBSLAVE::TimeoutStatus() { 
  if (rUartDrv.rx_idx == 0) { 
    return StatusTO::NO_RECEPTION;
  }
  unsigned int delta = LPC_TIM0->TC - rUartDrv.last_byte_time;
  if (delta > MODBUS_SILENCE_TICKS) {
    return StatusTO::EXPIRED;
  } 
  return StatusTO::NOT_EXPIRED;
}

void CMBSLAVE::monitor() {  
  if (TimeoutStatus() == StatusTO::EXPIRED) {    
    //parseRequest(); 
    rUartDrv.rx_idx = 0; 
  } 
}


