#include "event_log.hpp"
#include "process.hpp" 

CEVENT_LOG::CEVENT_LOG() {}

void CEVENT_LOG::get_pProcess(CPROCESS* p) {
  pProcess = p;
}

void CEVENT_LOG::clear_log() {
  // Просто заполняем весь массив в ОЗУ нулями
  std::memset(log_buffer, 0, sizeof(log_buffer));
  current_index = 0;
  log_wrapped = false;
}

void CEVENT_LOG::save_event(EEvent event) { 
  if (pProcess == nullptr) return;
  
  pProcess->rRTC.update_now();                 
  auto now = pProcess->rRTC.get_now();
  
  // Пишем напрямую в ОЗУ-массив по текущему индексу
  log_buffer[current_index].day    = static_cast<unsigned char>(now.day);
  log_buffer[current_index].month  = static_cast<unsigned char>(now.month);
  log_buffer[current_index].year   = static_cast<unsigned char>(now.year);
  log_buffer[current_index].hour   = static_cast<unsigned char>(now.hour);
  log_buffer[current_index].minute = static_cast<unsigned char>(now.minute);
  log_buffer[current_index].event  = event;
  log_buffer[current_index].R_val  = pProcess->R;
  
  // Инкремент индекса
  current_index++;
  if (current_index >= Log_Length) {
    current_index = 0;   // Сброс в начало массива
    log_wrapped = true;  // Сигнализируем, что пошел перезапуск по кругу
  }
}
