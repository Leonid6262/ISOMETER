#pragma once
#include "controllerDMA.hpp"
#include <cstdint>
#include "LPC407x_8x_177x_8x.h"

// Singleton класс драйвера UART для MbSlave
class CMBUartDriver {
public:
  void init(LPC_UART_TypeDef*, IRQn_Type, CDMAcontroller*);
  static CMBUartDriver& getInstance();
  
  unsigned short rx_idx = 0;
  unsigned int last_byte_time;
  
  void irq_handler();
  void transfer_data(unsigned short);
  
private:
  static constexpr unsigned char UART_FIFO_SIZE = 16;  // глубина аппаратного FIFO
  static constexpr unsigned int RBR_I   = 1UL << 0;    // RBR interrupt
  static constexpr unsigned int RDR     = 1UL << 0;
  
  LPC_UART_TypeDef* UART;
  CDMAcontroller* pCont_dma;
  
  CMBUartDriver();
  CMBUartDriver(const CMBUartDriver&) = delete;
  CMBUartDriver& operator=(const CMBUartDriver&) = delete;
};
