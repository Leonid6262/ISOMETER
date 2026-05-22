#include "main.hpp"

void UserStartInit() {     
  CSET_EMC::initAndCheck();     // Инициализации ext RAM и шины расширения. Контроль ext RAM 
}

void main(void) {
  CFactory::Peripherals_init(); // Инициализация периферии
  CPROCESS::UserLedOn();        // Визуальный контроль начала инициализации 
  
  Priorities::initPriorities(); // Распределение векторов по группам. см. в файле IntPriority.h  
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
