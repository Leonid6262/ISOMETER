#include "main.hpp"

void main(void) {
  
  CSET_PORTS::initDOutputs();   // Инициализация дискретных выходов микроконтроллера (pins)
  CPROCESS::UserLedOn();        // Визуальный контроль светодиодов и начала инициализации 
  CPROCESS::LampMeasOn();
  CPROCESS::LampAlarm1On();
  CPROCESS::LampAlarm2On();
  Priorities::initPriorities(); // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();     // Инициализация таймеров.  
  CFactory::load_settings();    // Загрузка уставок (RAM <- EEPROM)

  static auto process = CFactory::create_Process();             // Основной объект измерений
  static auto menu_navigation = CFactory::create_MN(process);   // Навигация по меню ПТ
  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave
    
  CFactory::control_set(menu_navigation);                       // При ошибке КС требуется зпись дефолтных уставок  
  
  CPROCESS::UserLedOff();       // Визуальный контроль окончания инициализации
  CPROCESS::LampMeasOff();
  CPROCESS::LampAlarm1Off();
  CPROCESS::LampAlarm2Off();
  
  Pause_us(500000);

  while (true) {       
    process.step();             // Процесс измерений
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    menu_navigation.get_key();  // Опрос клавиатуры ПТ
  }
  
}
