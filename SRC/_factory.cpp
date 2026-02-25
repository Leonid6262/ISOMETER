#include "factory.hpp"

#pragma location = ".dma_buffers" 
__root unsigned short CMBSLAVE::tx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

#pragma location = ".dma_buffers"
__root unsigned short CMBSLAVE::rx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

using ESET = CEEPSettings;
using EUART = CSET_UART::EUartInstance;
using ESPI = CSET_SPI::ESPIInstance;
 
StatusRet CFactory::load_settings()   { return ESET::getInstance().loadSettings(); }        // Загрузка уставок

// ModBus slave
CMBSLAVE CFactory::create_MBslave() {
  static CDMAcontroller cont_dma;     // Управление каналами DMA
  return CMBSLAVE(cont_dma, CSET_UART::configure(EUART::UART_1));
}
// Основной класс
CPROCESS CFactory::create_Process() {
  static CADC adc(CSET_SPI::config(ESPI::SPI_1), ESET::getInstance()); 
  return CPROCESS(adc, ESET::getInstance());
}

// Пультовый терминал
CMenuNavigation& CFactory::create_MN(CPROCESS& rProcess) {
  auto& udrv = CTerminalUartDriver::getInstance();                                 // Конфигурация и инициализация UART-0 - пультовый терминал 
  udrv.init(CSET_UART::configure(EUART::UART_0), UART0_IRQn); 
  static CMenuNavigation menu_navigation(udrv, ESET::getInstance(), rProcess);     // Пультовый терминал (навигация по меню).
  return menu_navigation;
}

extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0
