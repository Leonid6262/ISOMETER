#include "mb_slave.hpp" 

CMBSLAVE::CMBSLAVE(CMbUartDriver& rUartDrv, CDMAcontroller& rDMAc, LPC_UART_TypeDef* UART) : rUartDrv(rUartDrv), rDMAc(rDMAc), UART(UART) {}


void CMBSLAVE::monitor() {
  
  
}


