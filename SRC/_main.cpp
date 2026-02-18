#include "main.hpp"

void UserStartInit() {  
  CSET_PORTS::initDOutputs(); // Инициализация дискретных выходов микроконтроллера (pins)
  CDIN_STORAGE::UserLedOn();  // Визуальный контроль начала инициализации  
  CSET_EMC::initAndCheck();   // Инициализации ext RAM и шины расширения. Контроль ext RAM
}

void main(void) {
  Priorities::initPriorities();                         // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();                             // Инициализация таймеров.
  CEEPSettings::getInstance().loadSettings();           // Загрузка уставок
  
  /* --- Создание объектов ---*/
  static CRTC rt_clock;                                                                          // Системные часы
  static CDin_cpu din_cpu;                                                                       // Дискретные входы контроллера
  static CDMAcontroller cont_dma;                                                                // Управление каналами DMA
  static CIADC int_adc(CADC_STORAGE::getInstance());                                             // Внутренее ADC.
  static CSPI_ports spi_ports (CSET_SPI::config(CSET_SPI::ESPIInstance::SPI_0));                 // R/W  dIO доступные по SPI
  static CADC adc(CSET_SPI::config(CSET_SPI::ESPIInstance::SPI_1), CADC_STORAGE::getInstance()); // Внешнее ADC
  
  static CDAC0 dac_0(CEEPSettings::getInstance());                                               // DAC0 
  static CDAC_PWM dac_1(CDAC_PWM::EPWM_DAC::PWM_DAC1, CEEPSettings::getInstance());              // DAC1
  static CDAC_PWM dac_2(CDAC_PWM::EPWM_DAC::PWM_DAC2, CEEPSettings::getInstance());              // DAC2
  
  static CMBSLAVE mb_slave(cont_dma, CSET_UART::configure(CSET_UART::EUartInstance::UART_1));    // Конфигурация и инициализация UART-1 - MB Slave
  
  static auto& udrv = CTerminalUartDriver::getInstance();                                        // Конфигурация и инициализация UART-0 - пультовый терминал 
  udrv.init(CSET_UART::configure(CSET_UART::EUartInstance::UART_0), UART0_IRQn); 
  std::string test_string = "   1234567890   \r"; // Тестовая строка
  
  CDIN_STORAGE::UserLedOff();  // Визуальный контроль окончания инициализации
  
  /* --- Пример использования ---*/
  while (true) {   
    
    rt_clock.update_now();           // Обновление экземпляра структуы SDateTime данными из RTC
    int_adc.measure_5V();            // Измерение напряжения питания +/- 5V (внутреннее ADC)
    din_cpu.input_Pi0();             // Чтение состояния дискретных входов контроллера Pi0
    spi_ports.rw();                  // Запись в дискретные выходы и чтение дискретных входов доступных по SPI
    dac_0.conv(CDAC0::HALF_VAL);     // Конвертация 0.5 MAX DAC0
    dac_1.conv(CDAC_PWM::HALF_VAL);  // Конвертация 0.5 MAX DAC1
    dac_2.conv(CDAC_PWM::HALF_VAL);  // Конвертация 0.5 MAX DAC2
    mb_slave.monitor();              // Мониторинг запросов по ModBus
    adc.conv_tnf({                   // Конвертация сигналов внешнего ADC
      CADC_STORAGE::Name_ch00, 
      CADC_STORAGE::Name_ch01, 
      CADC_STORAGE::Name_ch02,
      CADC_STORAGE::Name_ch03, 
      CADC_STORAGE::Name_ch04,
      CADC_STORAGE::Name_ch05,
      CADC_STORAGE::Name_ch06 });
    
   // Вывод тестовой строки на терминал или ПК
   udrv.sendBuffer(reinterpret_cast<const unsigned char*>(test_string.c_str()), test_string.size());
  }
}
