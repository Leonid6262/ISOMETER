#include "mb_slave.hpp" 

CMBSLAVE::CMBSLAVE(CMbUartDriver& rUartDrv, CDMAcontroller& rDMAc) : rUartDrv(rUartDrv), rDMAc(rDMAc) {}


void CMBSLAVE::monitor() {
  
  if (rUartDrv.rx_idx == 0) return; // Ждем начала приема
 
  unsigned int delta = LPC_TIM0->TC - rUartDrv.last_byte_time;
    
  if (delta > MODBUS_SILENCE_TICKS) {   
    rUartDrv.rx_idx = 0; 
    
  }
  
}


