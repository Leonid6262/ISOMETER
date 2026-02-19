#include "main.hpp"

void main(void) {
  
  CSET_PORTS::initDOutputs();     // Инициализация дискретных выходов микроконтроллера (pins)
  CD_cpu::UserLedOn();            // Визуальный контроль начала инициализации 
  Priorities::initPriorities();   // Распределение векторов по группам. см. в файле IntPriority.h
  CSET_TIMER::initTimers();       // Инициализация таймеров.
  
  if (CFactory::load_settings() == StatusRet::ERROR) {  // Загрузка уставок (RAM <- EEPROM)    
    SWarning::setMessage(EWarningId::DEFAULT_SET);      // При ошибке - собщение: "Загружены дефолтные уставки",    
                                                        // и соответствующая светодиодная индикация 
  } 
  SWork::setMessage(EWorkId::MEASUREMENTS);
  
  static auto mb_slave = CFactory::create_MBslave();            // ModBus slave
  static auto process = CFactory::create_Process();             // Основной объект измерений
  static auto& term_manager = CFactory::createTM(process);      // Управление объектами ПТ
  
  CD_cpu::UserLedOff();  // Визуальный контроль окончания инициализации
 
  while (true) {   
    
    process.step();             // Процесс измерений
    mb_slave.monitor();         // Мониторинг запросов по ModBus
    term_manager.dispatch();    // Управление объектами (режимами) пультового терминал 

  }
}
