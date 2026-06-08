#include "mbUartDriver.hpp"
#include "mb_rtu_slave.hpp" 
#include "IntPriority.hpp"
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC177x.h"

CMBUartDriver::CMBUartDriver() {};

void CMBUartDriver::init(LPC_UART_TypeDef* pUART, IRQn_Type UART_IRQ, CDMAcontroller* pCont_dma) {
  UART = pUART;
  this->pCont_dma = pCont_dma;
  init_dma();
  UART->FCR |= DMAMODE;
  // Настройка прерываний
  UART->IER |= RBR_I;  // b0-RBR
  NVIC_EnableIRQ(UART_IRQ);
}

// RBR Handler
void CMBUartDriver::irq_handler() { 
  
  while (UART->LSR & RDR) { 
    unsigned char byte = UART->RBR; // Чтение RBR сбрасывает прерывание в IIR
    last_byte_time = SysT::TC();    // Фиксируем время КАЖДОГО байта
    
    if (rx_idx < CMBSLAVE::TRANSACTION_LENGTH) {
      CMBSLAVE::rx_mbs_buffer[rx_idx++] = byte;
    }
  }  
  
}

/* Конфигурирование используемых каналов DMA (карту каналов с.м. в controllerDMA.hpp) */
/* BurstSize = 4. При EWidth::Short → 16 бит → 4 × 2 байта = 8 байт, при FIFO = 16 байт, с запасом */
void CMBUartDriver::init_dma() {
    // Конфигурация канала записи
    CDMAcontroller::SChannelConfig cfg_ch_tx {
    CDMAcontroller::ChannelMap::UART2_Tx_Channel, // Номер канала
    CDMAcontroller::ETransferType::TYPE_M2P,      // Тип канала
    CDMAcontroller::EConnNumber::UART2_Tx,        // Номер периферийного подключения
    CDMAcontroller::DmaBurst::SIZE_1,             // Количество единичных элементов транзакции
    CDMAcontroller::EWidth::BYTE,                 // Размер единичного элемента
    static_cast<bool>(State::OFF)                 // Разрешение/запрет события окончания передачи (ON/OFF)
  };

  pCont_dma->init_M2P2M_Channel(&cfg_ch_tx);
}

void CMBUartDriver::transfer_data(unsigned short Length) {
  // Старт записи
  pCont_dma->StartTxTransfer(CDMAcontroller::ChannelMap::UART2_Tx_Channel,
                           static_cast<unsigned int>((long long)CMBSLAVE::tx_mbs_buffer), Length);
}

CMBUartDriver& CMBUartDriver::getInstance() {
  static CMBUartDriver instance;
  return instance;
}  
