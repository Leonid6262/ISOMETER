#include "main.hpp"

void UserStartInit() {     
  CSET_PORTS::initDOutputs();   // Инициализация дискретных выходов микроконтроллера (pins)
  CPROCESS::UserLedOn();        // Визуальный контроль светодиодов и начала инициализации 
  CSET_EMC::initAndCheck();     // Инициализации ext RAM и шины расширения. Контроль ext RAM 
}

void main(void) {
  
  Priorities::initPriorities(); // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();     // Инициализация таймеров.  
  CFactory::load_settings();    // Загрузка уставок (RAM <- EEPROM)

  static auto process = CFactory::create_Process();             // Основной объект измерений
  static auto menu_navigation = CFactory::create_MN(process);   // Навигация по меню ПТ
  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave
    
  CFactory::control_set(menu_navigation);                       // При ошибке КС требуется зпись дефолтных уставок  
  
  CPROCESS::UserLedOff();       // Визуальный контроль окончания инициализации
  process.start_test();

  while (true) {       
    process.step();             // Процесс измерений
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    menu_navigation.get_key();  // Опрос клавиатуры ПТ
  }
  
}
