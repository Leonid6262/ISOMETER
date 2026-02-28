#pragma once
#include <cstdint>
#include "LPC407x_8x_177x_8x.h"

// Singleton класс драйвера UART для MbSlave
class CMbUartDriver {
public:
  void init(LPC_UART_TypeDef*, IRQn_Type);
  static CMbUartDriver& getInstance();
  
  unsigned short rx_idx = 0;
  unsigned int last_byte_time;
  
  void irq_handler();
  
private:
  static constexpr unsigned char UART_FIFO_SIZE = 16;  // глубина аппаратного FIFO
  static constexpr unsigned int RBR_I   = 1UL << 0;    // RBR interrupt
  static constexpr unsigned int RDR     = 1UL << 0;
  
  LPC_UART_TypeDef* UART;
  
  CMbUartDriver();
  CMbUartDriver(const CMbUartDriver&) = delete;
  CMbUartDriver& operator=(const CMbUartDriver&) = delete;
};
