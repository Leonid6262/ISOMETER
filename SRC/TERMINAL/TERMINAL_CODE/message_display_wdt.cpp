#include "message_display_wdt.hpp"
#include "string_utils.hpp"
#include "pause_us.hpp"

CMessageDisplay::CMessageDisplay(CTerminalUartDriver& uartDrv) : 
  uartDrv(uartDrv), l(CEEPSettings::getInstance().getSettings().Language - 1),
  COUNT_CATEGORIES(static_cast<unsigned char>(ECategory::COUNT)) {
  // Регистрация категорий в CategoryContext
  contexts[static_cast<unsigned char>(ECategory::NOT_READY)] = {
      SNotReady::active, 0,
      SNotReady::NAME, SNotReady::MSG,
      static_cast<unsigned char>(ENotReadyId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::READY)] = {
      SReady::active, 0,
      SReady::NAME, SReady::MSG,
      static_cast<unsigned char>(EReadyId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::WORK)] = {
      SWork::active, 0,
      SWork::NAME, SWork::MSG,
      static_cast<unsigned char>(EWorkId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::FAULT)] = {
      SFault::active, 0,
      SFault::NAME, SFault::MSG,
      static_cast<unsigned char>(EFaultId::COUNT)};
  contexts[static_cast<unsigned char>(ECategory::WARNING)] = {
      SWarning::active, 0,
      SWarning::NAME, SWarning::MSG,
      static_cast<unsigned char>(EWarningId::COUNT)};
}

void CMessageDisplay::set_pTerminal(CTerminalManager* pTerminal_manager){
  this->pTerminal_manager = pTerminal_manager;
}

// Отправки строки
void CMessageDisplay::sendLine(const std::string& s, bool newline) {
    std::string text = StringUtils::padTo16(s);
    text += newline ? "\r\n" : "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
}


// Вывод заголовка категории (если print_title == true) и сообщения
void CMessageDisplay::render_messages(signed char cat, bool print_title = false) {
  
  const auto& ctx = contexts[static_cast<unsigned char>(cat)];
  
  if (print_title) {
    sendLine(StringUtils::utf8_to_cp1251(ctx.NAME[l]), newline);
  }  
  sendLine(StringUtils::utf8_to_cp1251(ctx.MSG[ctx.cursor][l]));
}

void CMessageDisplay::rotate_messages() {
  static unsigned char cur_cat = 0;     // Текущая категория
  static unsigned char emp_num = 0;     // Счётчик "пустых" категорий
  static bool print_title = true;       // Признак вывода заголовка
  static bool had_active = false;       // В текущей категории был актив
  
  while (true) {
    
    if (contexts[cur_cat].cursor >= contexts[cur_cat].count) {
      contexts[cur_cat].cursor = 0;
      cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
      print_title = true;
    }
    
    auto &ctx = contexts[cur_cat];
    
    if (ctx.active[ctx.cursor]) {
      if (first_call) {
        first_call = false;
        print_title = true;
      }
      render_messages(cur_cat, print_title);
      print_title = false;
      ctx.cursor++;
      emp_num = 0;
      had_active = true;
      return;      
    } else {
      ctx.cursor++;
      if (ctx.cursor >= ctx.count) {
        ctx.cursor = 0;
        cur_cat = (cur_cat + 1) % COUNT_CATEGORIES;
        print_title = true;      
        if (had_active) {
          emp_num = 0;
        } else {
          emp_num++;
          if (emp_num >= COUNT_CATEGORIES) {
            emp_num = 0;
          }
        }
        had_active = false;        
      }
    }
    
  }
}


// "Опрос" клавиатуры
void CMessageDisplay::get_key() {
  unsigned char input_key;
  if (uartDrv.poll_rx(input_key)) {
    Key_Handler(static_cast<EKey_code>(input_key));
  } else {
    Key_Handler(EKey_code::NONE);
  }
}

void CMessageDisplay::Key_Handler(EKey_code key) {
  switch (key) {
  case EKey_code::ESCAPE:
    first_call = true;
    pTerminal_manager->switchToMenu(); // переключаемся в меню
    break;
  case EKey_code::FnEsc:
    {
      unsigned char led_blue[] = {static_cast<unsigned char>(ELED::LED_BLUE), '\r'}; 
      uartDrv.sendBuffer(led_blue, sizeof(led_blue));
      CategoryUtils::clearMessages(ECategory::FAULT);
      CategoryUtils::clearMessages(ECategory::COUNT);
      Pause_us(200000);
      unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
      uartDrv.sendBuffer(led_off, sizeof(led_off));
    }
    break;
  case EKey_code::NONE:
  default: {
    unsigned int dTrs = LPC_TIM0->TC - prev_TC0;
    if (dTrs >= MESSAGE_PERIOD_TICKS) { 
      prev_TC0 = LPC_TIM0->TC;
      if (first_call) {
        unsigned char led_off[] = {static_cast<unsigned char>(ELED::LED_OFF), '\r'};
        uartDrv.sendBuffer(led_off, sizeof(led_off));
      }     
      rotate_messages();
    }
  }
  }
}
