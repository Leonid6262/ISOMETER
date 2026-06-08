#include "factory.hpp"

#pragma location = ".dma_buffers" 
__root unsigned char CMBSLAVE::tx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

#pragma location = ".dma_buffers"
__root unsigned char CMBSLAVE::rx_mbs_buffer[CMBSLAVE::TRANSACTION_LENGTH];

using ESET  = CEEPSettings;
using EUART = CSET_UART::EUartInstance;

void CFactory::Peripherals_init() {  
    CPERIPHERIALS_INIT cpi; 
    cpi.initDOutputs();
    cpi.powerON();
    cpi.initIOCON();
    cpi.initTimers();
}

StatusRet CFactory::load_settings()   { return ESET::getInstance().loadSettings(); }        // Загрузка уставок

// ModBus slave
CMBSLAVE CFactory::create_MBslave() {
  static CDMAcontroller cont_dma;                                                           // Управление каналами DMA
  auto& udrv = CMBUartDriver::getInstance();                                                // Конфигурация и инициализация UART-2
  udrv.init(CSET_UART::configure(EUART::UART_2, ESET::getInstance()), UART2_IRQn, &cont_dma); 
  return CMBSLAVE(udrv, CModbusDataProxy::getInstance(), &ESET::getInstance().getSettings().Address);
}
extern "C" void UART2_IRQHandler(void) { CMBUartDriver::getInstance().irq_handler(); }      // Вызов обработчика UART-2

// Основной класс
CPROCESS& CFactory::create_Process() { 
  static CADC adc(CSET_SPI::config(P::SPI1, 3000000, 16), ESET::getInstance());
  static CRTC rt_clock;  
  static CDAC_PWM dac_pwm(P::PWM_DAC);
  static CEVENT_LOG event_log; 
  static COUT_4_20 out_4_20(dac_pwm, CDAC_PWM::DAC_PWM_MAX_VAL, ESET::getInstance());  // Out 4...20mA   
  static CPROCESS process(adc, ESET::getInstance(), CModbusDataProxy::getInstance(), out_4_20, rt_clock, event_log);
  event_log.get_pProcess(&process);
  return process; 
}

// Инициализация драйвера ПТ, создание объектов ПТ и его окружения
CTerminalManager& CFactory::createTM(CPROCESS& rProcess) {  
  auto& udrv = CTerminalUartDriver::getInstance();                                          // Конфигурация и инициализация UART-0 
  udrv.init(CSET_UART::configure(EUART::UART_0, ESET::getInstance()), UART0_IRQn); 
  
  static CLogDisplay log_display(udrv, rProcess.rEventLog);                                                     // Пультовый терминал (просмотр журнала).
  static CMenuNavigation menu_navigation(udrv, ESET::getInstance(), rProcess);              // Пультовый терминал (индикация и навигация по меню).
  
  static CTerminalManager terminal_manager(menu_navigation, log_display);                   // Управление режимами пультового терминал
  menu_navigation.set_pTerminal(&terminal_manager);                                         // Создание циклической зависимости menu  
  log_display.set_pTerminal(&terminal_manager);                                             // Создание циклической зависимости log
  return terminal_manager;
}
extern "C" void UART0_IRQHandler(void) { CTerminalUartDriver::getInstance().irq_handler(); }  // Вызов обработчика UART-0

CUDP_Server CFactory::create_UDP_Server() {
  CEMAC emac;
  emac.initEMAC();
  CModbusDataProxy& modbusProxy = CModbusDataProxy::getInstance();
  ESET& settings = ESET::getInstance();
  static CENET_DRV enet;
  static CUDP_Server udp_server(enet, settings);
  static CMB_UDP_Slave mb_slave(
    modbusProxy,
    enet,
    udp_server.getRxBuffer(), // Метод должен возвращать указатель на Rx_Frame
    udp_server.getTxBuffer(), // Метод должен возвращать указатель на Tx_Frame
    &settings.getSettings().Address, // Указатель на адрес прибора
    udp_server.getMyIPPtr()   // Указатель на текущий IP контроллера
  );
  
  udp_server.setModbusSlave(&mb_slave);
  return udp_server;
}

// Контроль загрузки. При ошибке КС требуется зпись дефолтных уставок 
void CFactory::control_set(CTerminalManager& rTM) { 
  if(ESET::getInstance().err_load == StatusRet::ERROR) {
    
    unsigned int prev_TC0 = SysT::TC();
    bool led_on = false; 
    unsigned int dTrsPhase = 0; 
    constexpr bool only_fn_enter = true;
    constexpr unsigned int LED_PAUSED  = 2000000;
    
    while(ESET::getInstance().save_status == State::OFF) {
      // Индикация
      dTrsPhase = SysT::TC() - prev_TC0;
      if(dTrsPhase > LED_PAUSED) {
        prev_TC0 = SysT::TC();
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
      rTM.rMenuNavigation.get_key(only_fn_enter);
    }     
  }  
}
