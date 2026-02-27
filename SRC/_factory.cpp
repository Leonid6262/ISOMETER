#include "factory.hpp"

#pragma location = ".dma_buffers" 
__root unsigned short CMBSLAVE::tx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

#pragma location = ".dma_buffers"
__root unsigned short CMBSLAVE::rx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

using ESET  = CEEPSettings;
using EUART = CSET_UART::EUartInstance;
using ESPI  = CSET_SPI::ESPIInstance;
 
StatusRet CFactory::load_settings()   { return ESET::getInstance().loadSettings(); }        // Загрузка уставок

// ModBus slave
CMBSLAVE CFactory::create_MBslave() {
  static CDMAcontroller cont_dma;     // Управление каналами DMA
  auto& udrv = CMbUartDriver::getInstance();                                 // Конфигурация и инициализация UART-2
  udrv.init(CSET_UART::configure(EUART::UART_2), UART2_IRQn); 
  return CMBSLAVE(udrv, cont_dma, CSET_UART::configure(EUART::UART_2));
}
extern "C" void UART2_IRQHandler(void) { CMbUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-2

// Основной класс
CPROCESS CFactory::create_Process() {
  static CADC adc(CSET_SPI::config(ESPI::SPI_1), ESET::getInstance()); 
  return CPROCESS(adc, ESET::getInstance());
}

// Пультовый терминал
CMenuNavigation CFactory::create_MN(CPROCESS& rProcess) {
  auto& udrv = CTerminalUartDriver::getInstance();                                 // Конфигурация и инициализация UART-0 
  udrv.init(CSET_UART::configure(EUART::UART_0), UART0_IRQn); 
  static CMenuNavigation menu_navigation(udrv, ESET::getInstance(), rProcess);     // Пультовый терминал (навигация по меню).
  return menu_navigation;
}
extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0

// Контроль загрузки. При ошибке КС требуется зпись дефолтных уставок 
void CFactory::control_set(CMenuNavigation& rMenu) {
  if(ESET::getInstance().err_load == StatusRet::ERROR) {
    
    unsigned int prev_TC0 = LPC_TIM0->TC;
    bool led_on = false; 
    unsigned int dTrsPhase = 0; 
    constexpr bool only_fn_enter = true;
    constexpr unsigned int LED_PAUSED  = 2000000;
    
    CPROCESS::RelReadyOff();
    while(ESET::getInstance().save_status == State::OFF) {
      // Индикация
      dTrsPhase = LPC_TIM0->TC - prev_TC0;
      if(dTrsPhase > LED_PAUSED) {
        prev_TC0 = LPC_TIM0->TC;
        led_on = !led_on;
        if(led_on) {
          CPROCESS::LampMeasOff();
          CPROCESS::LampAlarm1Off();
          CPROCESS::LampAlarm2Off();
        } else {
          CPROCESS::LampMeasOn();
          CPROCESS::LampAlarm1On();
          CPROCESS::LampAlarm2On();
        }
      } 
      // Ожидание записи дефолтных уставок (Fn+Enter)
      rMenu.get_key(only_fn_enter);
    }     
    CPROCESS::RelReadyOn();
  }  
}
