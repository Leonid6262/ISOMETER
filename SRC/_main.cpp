#include "main.hpp"
#include "Config.hpp" 

void UserStartInit() {     
  CFactory::Peripherals_init(); // Инициализация периферии

#ifdef EXT_RAM
  CSET_EMC::initAndCheck();     // Инициализации  и контроль ext RAM
#endif

}

void main(void) {
   
  Priorities::initPriorities(); // Распределение векторов по группам. см. в файле IntPriority.h  
  CFactory::load_settings();    // Загрузка уставок (RAM <- EEPROM)
  
  static auto& process = CFactory::create_Process();            // Основной объект измерений
  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave 
  static auto& term_manager = CFactory::createTM(process);      // Управление объектами ПТ

#ifdef UDP_SERVER
  static auto udp_eth = CFactory::create_UDP_Server();          // UDP сервер
#endif
  
  CFactory::control_set(term_manager);                          // При ошибке КС требуется зпись дефолтных уставок  
  
  CPROCESS::FaultLedOff();       // Визуальный контроль окончания инициализации

  while (true) {       
    process.step();             // Основной процесс    
    term_manager.dispatch();    // Управление объектами (режимами) ПТ  
#ifdef MB_RS485
    mb_slave.monitor();         // Мониторинг запросов по ModBus
#endif
#ifdef UDP_SERVER
    udp_eth.poll();             // Мониторинг запросов по ENET
#endif
  
  }
  
}
