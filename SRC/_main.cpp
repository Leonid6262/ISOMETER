#include "main.hpp"

void UserStartInit() {     
  CSET_EMC::initAndCheck();     // Инициализации  и контроль ext RAM
}

void main(void) {
  CFactory::Peripherals_init(); // Инициализация периферии
  
  Priorities::initPriorities(); // Распределение векторов по группам. см. в файле IntPriority.h  
  CFactory::load_settings();    // Загрузка уставок (RAM <- EEPROM)
  
  static auto& process = CFactory::create_Process();            // Основной объект измерений
  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave 
  static auto& term_manager = CFactory::createTM(process);      // Управление объектами ПТ
  
  CFactory::control_set(term_manager);                          // При ошибке КС требуется зпись дефолтных уставок  
  
  static auto loop_eth = CFactory::create_LOOP_ETH(); 
  
  CPROCESS::UserLedOff();       // Визуальный контроль окончания инициализации

  while (true) {       
    process.step();             // Процесс измерений
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    term_manager.dispatch();    // Управление объектами (режимами) пультового терминал  
    
    loop_eth.test();
    
  }
  
}
