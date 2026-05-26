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
  text += newline ? "\r\n" : "\r";
  uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}

void CLogDisplay::first_render(){
  current_index = rEventLog.current_index;
  wrapped = rEventLog.log_wrapped;
  Log_Length = CEVENT_LOG::Log_Length;
  current_view = calc_cur_view();
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
  
  if (count == 0) {
    sendLine("   LOG BOOK     ", true);  // Центрированный заголовок
    sendLine("   Empty Log    ", false); // Сообщение о пустоте
    return;
  }
  
  unsigned char day    = rEventLog.log_buffer[current_view].day;
  unsigned char month  = rEventLog.log_buffer[current_view].month;
  unsigned char hour   = rEventLog.log_buffer[current_view].hour;
  unsigned char minute = rEventLog.log_buffer[current_view].minute;
  unsigned short R     = rEventLog.log_buffer[current_view].R_val;
  CEVENT_LOG::EEvent event = rEventLog.log_buffer[current_view].event;
  
  char line[G_CONST::disp_l + 1];
  
  char idx_part[6] = ""; // Буфер под " TOP", " END" или номер (макс 5 символов с \0)
  
  if (current_view == TOP) {
    snprintf(idx_part, sizeof(idx_part), " TOP");
  } else if (current_view == END) {
    snprintf(idx_part, sizeof(idx_part), " END");
  } else {
    // Вычисляем порядковый номер от 1 до count
    unsigned short user_num = ((current_view - TOP + Log_Length) % Log_Length) + 1;
    // Печатаем номер с ведущим пробелом для красоты
    snprintf(idx_part, sizeof(idx_part), "%4u", user_num); 
  }
  
  // 3. Формируем Строку 1: "DD-MM hh:mm" (11 симв) + idx_part (выравнивание займет оставшиеся 5 симв)
  // Шаблон %-11s прижмет дату влево, а %5s прижмет номер/маркер вправо
  snprintf(line, sizeof(line), "%-11s%5s", 
           (char[12]){0}, // Временный хак, лучше сделать чистый snprintf:
           idx_part); 
  
  char date_part[12];
  snprintf(date_part, sizeof(date_part), "%02u-%02u %02u:%02u", day, month, hour, minute);
  // Собираем вместе: 11 символов даты + 5 символов номера = ровно 16 символов
  snprintf(line, sizeof(line), "%-11s%5s", date_part, idx_part);
  
  sendLine(line, true);
  
  // 1. Формируем правую часть во временный буфер
  char r_part[9]; 
  snprintf(r_part, sizeof(r_part), "R=%uk", R);
  
  // 2. Получаем левую часть
  const char* l_part = getEventText(event); 
  
  // 3. Вычисляем, сколько символов займет правая часть
  int r_len = strlen(r_part);
  
  // 4. Спецификатор %-*s выравнивает l_part по левому краю с шириной (16 - r_len).
  // Все оставшееся место между ними автоматически забьется пробелами.
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

unsigned short CLogDisplay::calc_cur_view() {
  if (!wrapped) { 
    if (current_index == 0) {
      // Вариант 1: Лог пуст
      count = 0; 
      TOP = 0;
      END = 0;
    } else {
      // Вариант 2: Лог частично заполнен (первый круг)
      count = current_index;
      TOP = 0;
      END = current_index - 1;
    }
  } else {
    // Вариант 3: Лог зациклен
    count = Log_Length;
    TOP = current_index;
    END = (current_index > 0) ? (current_index - 1) : (Log_Length - 1);
  }
  return TOP;
}

void CLogDisplay::Key_Handler(EKey_code key) {
  switch (key) {
  case EKey_code::UP: 
    if (current_view == TOP) {
      current_view = END; 
    } else {
      current_view = (current_view + Log_Length - 1) % Log_Length;
    }
    renderLogEntry();
    break;    
  case EKey_code::DOWN:
    if (current_view == END) {
      current_view = TOP;
    } else {
      current_view = (current_view + 1) % Log_Length;
    }    
    renderLogEntry();
    break;            
  case EKey_code::ESCAPE:
    pTerminal_manager->switchToMenu(); // переключаемся в меню
    break;
  case EKey_code::START:
  case EKey_code::STOP:
  case EKey_code::FnEsc:    
  case EKey_code::FnUP:
  case EKey_code::FnDOWN:
  case EKey_code::FnENTER:
  case EKey_code::ENTER:
  case EKey_code::NONE:   
    break;
  }
}
