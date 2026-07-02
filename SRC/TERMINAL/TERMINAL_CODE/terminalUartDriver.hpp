#pragma once
#include <cstdint>
#include "LPC407x_8x_177x_8x.h"
#include "PeripheralsInit.hpp"

// Singleton класс драйвера UART для терминала
// Использует прерывание по FIFO empty и всю глубину (16 byte) FIFO
// Для 16 байтных строк терминала: "0123456789012345\r" - одно прерывание дописывающее \r
class CTerminalUartDriver {
 public:
  void init(LPC_UART_TypeDef*, IRQn_Type);
  static CTerminalUartDriver& getInstance();

  bool sendBuffer(const unsigned char* data, unsigned char len);
  bool poll_rx(unsigned char& byte);
  
  bool bLampTx = false;
  bool bLampRx = false;
  unsigned int prev_TC0_Tx;
  unsigned int prev_TC0_Rx;
  
  void irq_handler();

 private:
  static constexpr unsigned char UART_FIFO_SIZE = 16;   // глубина аппаратного FIFO
  static constexpr unsigned int THRE_F  = 1UL << 5;     // THRE flag. FIFO empty
  static constexpr unsigned int THRE_I  = 1UL << 1;     // THRE interrupt. FIFO empty
  static constexpr unsigned int INTID_I = 0x01 << 1;    // ID THRE interrupt. FIFO empty
  static constexpr unsigned int RDR_F   = 1UL << 0;     // RDR flag. Есть данные для чтения
  

  static constexpr unsigned int LAMP_PERIOD_TICKS = 500000; // 50 мс
  
  static inline void LampPultTxOn()    { P::G1->SET = (1UL << bg::B_LampPult1);  }
  static inline void LampPultTxOff()   { P::G1->CLR = (1UL << bg::B_LampPult1);  }
  static inline void LampPultRxOn()    { P::G1->SET = (1UL << bg::B_LampPult2);  }
  static inline void LampPultRxOff()   { P::G1->CLR = (1UL << bg::B_LampPult2);  }

  // Внутренний кольцевой буфер
  struct RingBuffer {
    static constexpr int SIZE = 512;
    unsigned char buf[SIZE];
    unsigned short head = 0;
    unsigned short tail = 0;

    bool push(unsigned char b);
    bool pop(unsigned char& b);
    bool empty() const;
  };

  RingBuffer txbuf;

  LPC_UART_TypeDef* UART;

  CTerminalUartDriver();
  CTerminalUartDriver(const CTerminalUartDriver&) = delete;
  CTerminalUartDriver& operator=(const CTerminalUartDriver&) = delete;
};
