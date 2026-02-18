#include "main.hpp"

void UserStartInit() {  
  CSET_PORTS::initDOutputs(); // Инициализация дискретных выходов микроконтроллера (pins)
  CD_cpu::UserLedOn();        // Визуальный контроль начала инициализации  
  CSET_EMC::initAndCheck();   // Инициализации ext RAM и шины расширения. Контроль ext RAM
}

void main(void) {
  Priorities::initPriorities();                         // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();                             // Инициализация таймеров.
  CEEPSettings::getInstance().loadSettings();           // Загрузка уставок
  
  /* --- Создание объектов ---*/
  static CRTC rt_clock;                                                                          // Системные часы
  static CDMAcontroller cont_dma;                                                                // Управление каналами DMA
  static CADC adc(CSET_SPI::config(CSET_SPI::ESPIInstance::SPI_1), CADC_STORAGE::getInstance()); // Внешнее ADC
  
  static CMBSLAVE mb_slave(cont_dma, CSET_UART::configure(CSET_UART::EUartInstance::UART_1));    // Конфигурация и инициализация UART-1 - MB Slave
  
  static CPROCESS process(adc);  

  static auto& udrv = CTerminalUartDriver::getInstance();                                        // Конфигурация и инициализация UART-0 - пультовый терминал 
  udrv.init(CSET_UART::configure(CSET_UART::EUartInstance::UART_0), UART0_IRQn); 
  std::string test_string = "   1234567890   \r"; // Тестовая строка
  
  CD_cpu::UserLedOff();  // Визуальный контроль окончания инициализации
  /* --- Пример использования ---*/
  while (true) {   
    
    process.step();
    
    rt_clock.update_now();           // Обновление экземпляра структуы SDateTime данными из RTC
    mb_slave.monitor();              // Мониторинг запросов по ModBus

    
   // Вывод тестовой строки на терминал или ПК
   udrv.sendBuffer(reinterpret_cast<const unsigned char*>(test_string.c_str()), test_string.size());
  }
}
