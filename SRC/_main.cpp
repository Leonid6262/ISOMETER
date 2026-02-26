#include "main.hpp"

void main(void) {
  
  CSET_PORTS::initDOutputs();     // Инициализация дискретных выходов микроконтроллера (pins)
  CPROCESS::UserLedOn();          // Визуальный контроль начала инициализации 
  Priorities::initPriorities();   // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();       // Инициализация таймеров.
  
  CSET_CONTROL::err_load = CFactory::load_settings();  // Загрузка уставок (RAM <- EEPROM)

  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave
  static auto process = CFactory::create_Process();             // Основной объект измерений
  static auto& menu_navigation = CFactory::create_MN(process);  // Навигация по меню ПТ
    
  // При ошибке КС требуется зпись дефолтных уставок  
  if(CSET_CONTROL::err_load == StatusRet::ERROR) { 
    CPROCESS::RelReadyOff();
    CSET_CONTROL::prev_TC0 = LPC_TIM0->TC;
    while(CEEPSettings::getInstance().save_status == State::OFF) {
      CSET_CONTROL::indication();
      menu_navigation.get_key(CSET_CONTROL::only_ctr_enter);
    } 
    process.prev_TC0_Phase = LPC_TIM0->TC;
    CPROCESS::RelReadyOn();
  }
    
  CPROCESS::UserLedOff();  // Визуальный контроль окончания инициализации

  while (true) {       
    process.step();             // Процесс измерений
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    menu_navigation.get_key();  // Опрос клавиатуры ПТ
  }
}
