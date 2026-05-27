#include "log_display.hpp"
#include "string_utils.hpp"
#include "terminal_manager.hpp"

CLogDisplay::CLogDisplay(CTerminalUartDriver& uartDrv, CEVENT_LOG& rEventLog) : uartDrv(uartDrv), rEventLog(rEventLog) {} 

void CLogDisplay::set_pTerminal(CTerminalManager* pTerminal_manager){
  this->pTerminal_manager = pTerminal_manager;
}

// Отправки строки 
void CLogDisplay::sendLine(const std::string& s, bool newline) {
  std::string text = StringUtils::padTo16(s);
  //text += newline ? "\r\n" : "\r";
  if(newline) {
    text = "\r\n" + text + "\r";
  } else {
    text = "\n" + text;
  }
  uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}

void CLogDisplay::first_render(){
  
  unsigned short global_index = rEventLog.current_index;
  bool global_wrapped = rEventLog.log_wrapped;
  unsigned short max_len = CEVENT_LOG::Log_Length;
  
  local_count = global_wrapped ? max_len : global_index;
  
  if (local_count == 0) {
    current_view = 0;
    renderLogEntry();
    return;
  }
  
  // Выпрямляем кольцо в плоский массив (от старых к новым)
  unsigned short src_idx = global_wrapped ? global_index : 0;
  
  for (unsigned short i = 0; i < local_count; i++) {
    local_buffer[i] = rEventLog.log_buffer[src_idx];
    src_idx = (src_idx + 1) % max_len;
  }
  
  // Лог скопирован. Теперь мы работаем с обычным массивом от 0 до local_count - 1
  // Показываем самую свежую запись (последнюю в массиве)
  current_view = local_count - 1; 
  renderLogEntry();
  
}

const char* CLogDisplay::getEventText(CEVENT_LOG::EEvent event) {
  switch (event) {
  // Каждая строка жестко макс. 8 символов
  case CEVENT_LOG::EEvent::NoEvents:   return "No Evnt";  // 7 симв.
  case CEVENT_LOG::EEvent::LessMin:    return "R<Rmin";   // 6 симв.
  case CEVENT_LOG::EEvent::MoreMax:    return "R>Rmax";   // 6 симв.
  case CEVENT_LOG::EEvent::NormRange:  return "NormRang"; // 8 симв.
  case CEVENT_LOG::EEvent::Alarm1_On:  return "Al1-On";   // 6 симв.
  case CEVENT_LOG::EEvent::Alarm1_Off: return "Al1-Off";  // 7 симв.
  case CEVENT_LOG::EEvent::Alarm2_On:  return "Al2-On";   // 6 симв.
  case CEVENT_LOG::EEvent::Alarm2_Off: return "Al2-Off";  // 7 симв.
  case CEVENT_LOG::EEvent::Fault_On:   return "Fault ON"; // 8 симв.
  case CEVENT_LOG::EEvent::Fault_Off:  return "Flt OFF";  // 7 симв.
  default:                             return "Unknown";  // 7 симв.
  }
}

void CLogDisplay::renderLogEntry() { 
  if (local_count == 0) {
    sendLine("    LOG BOOK    ", true);
    sendLine("   Empty Log    ", false);
    return;
  }
  
  // Читаем из гарантированно стабильного локального буфера
  unsigned char day    = local_buffer[current_view].day;
  unsigned char month  = local_buffer[current_view].month;
  unsigned char hour   = local_buffer[current_view].hour;
  unsigned char minute = local_buffer[current_view].minute;
  unsigned short R     = local_buffer[current_view].R_val;
  CEVENT_LOG::EEvent event = local_buffer[current_view].event;
  
  char line[G_CONST::disp_l + 1];
  
  char date_part[12];
  snprintf(date_part, sizeof(date_part), "%02u-%02u %02u:%02u", day, month, hour, minute);
  
  char idx_part[6] = "";
  if (current_view == 0) {
    snprintf(idx_part, sizeof(idx_part), " TOP"); // Самое старое событие
  } else if (current_view == local_count - 1) {
    snprintf(idx_part, sizeof(idx_part), " END"); // Самое свежее событие
  } else {
    // Человеческий порядковый номер — это просто индекс в массиве + 1
    snprintf(idx_part, sizeof(idx_part), "%4u", current_view + 1); 
  }
  
  snprintf(line, sizeof(line), "%-11s%5s", date_part, idx_part);
  sendLine(line, true);
  
  char r_part[9]; 
  snprintf(r_part, sizeof(r_part), "R=%uk", R);
  
  const char* l_part = getEventText(event); 
  int r_len = strlen(r_part);
  
  snprintf(line, sizeof(line), "%-*s%s", (G_CONST::disp_l - r_len), l_part, r_part);
  sendLine(line, false);

}

// "Опрос" клавиатуры
void CLogDisplay::get_key() {
  unsigned char input_key;
  if (uartDrv.poll_rx(input_key)) {
    Key_Handler(static_cast<EKey_code>(input_key));
  } else {
    Key_Handler(EKey_code::NONE);
  }
}

void CLogDisplay::Key_Handler(EKey_code key) {
  if (key == EKey_code::NONE) return;
  
  if (key == EKey_code::ESCAPE) {
    std::string text = "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
    pTerminal_manager->switchToMenu();
    return;
  }
  
  if (local_count == 0) return; // Лог пуст
  
  // Выгрузка лога
  if ((key == EKey_code::LoadLogl) || (key == EKey_code::LoadLogL)) {
    
    pTerminal_manager->rMenuNavigation.rProcess.rRTC.update_now();                 
    auto now = pTerminal_manager->rMenuNavigation.rProcess.rRTC.get_now();
    char date_time[G_CONST::disp_l + 5];
    snprintf(date_time, sizeof(date_time), "\n%02u.%02u.%02u %02u:%02u:%02u\r\n", 
             now.day, now.month, now.year, now.hour, now.minute, now.second);
    sendLine("\nLOG LIST:", true);
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(date_time), (G_CONST::disp_l + 5));

    current_view = 0;
    for(unsigned short n = 0; n < local_count; n++){
      renderLogEntry();
      current_view++;
    }
    
    sendLine("END LIST\r\n", true);
    current_view = local_count - 1; 
    renderLogEntry();
    return;
  }
  
  switch (key) {
  case EKey_code::UP: 
    if (current_view == 0) {
      current_view = local_count - 1; // Закольцовываем просмотр (в конец)
    } else {
      current_view--;
    }
    break;    
    
  case EKey_code::DOWN:
    if (current_view == local_count - 1) {
      current_view = 0; // Закольцовываем просмотр (в начало)
    } else {
      current_view++;
    }   
    break;            
  case EKey_code::FnUP:
    current_view = 0;
    break;    
  case EKey_code::FnDOWN:
    current_view = local_count - 1;
    break; 
  default:
    break;
  }
  
  renderLogEntry();

}
