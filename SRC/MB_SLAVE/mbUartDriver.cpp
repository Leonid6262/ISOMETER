#include "mbUartDriver.hpp"
#include "mb_slave.hpp" 
#include "IntPriority.hpp"
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

CMbUartDriver::CMbUartDriver() {};

void CMbUartDriver::init(LPC_UART_TypeDef* UART, IRQn_Type UART_IRQ) {
  this->UART = UART;

  // Настройка прерываний
  UART->IER |= RBR_I;  // b0-RBR

  NVIC_EnableIRQ(UART_IRQ);
}

CMbUartDriver& CMbUartDriver::getInstance() {
  static CMbUartDriver instance;
  return instance;
}
             
// RBR Handler
void CMbUartDriver::irq_handler() {
  unsigned int IRQ = UART->IIR;
  unsigned char byte = UART->RBR;
  if ((IRQ & INTID_I) == INTID_I)  // RBR interrupt
  {    
    if (rx_idx < CMBSLAVE::TRANSACTION_LENGTH) { 
      CMBSLAVE::rx_mbs_buffer[rx_idx++] = byte;
    }    
  }
}
