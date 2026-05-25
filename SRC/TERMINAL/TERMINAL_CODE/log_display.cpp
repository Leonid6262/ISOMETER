#include "log_display.hpp"
#include "string_utils.hpp"
#include "terminal_manager.hpp"

CLogDisplay::CLogDisplay(CTerminalUartDriver& uartDrv) : uartDrv(uartDrv) {} 

void CLogDisplay::set_pTerminal(CTerminalManager* pTerminal_manager){
  this->pTerminal_manager = pTerminal_manager;
}

// Отправки строки
void CLogDisplay::sendLine(const std::string& s, bool newline) {
    std::string text = StringUtils::padTo16(s);
    text += newline ? "\r\n" : "\r";
    uartDrv.sendBuffer(reinterpret_cast<const unsigned char*>(text.c_str()), text.size());
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
  switch (key) {
  case EKey_code::START:
        sendLine("123456", true);
    break;
  case EKey_code::STOP:
    //rSysMgr.set_bsWorkTest(State::OFF);
    break;
  case EKey_code::ESCAPE:
    pTerminal_manager->switchToMenu(); // переключаемся в меню
    break;
  case EKey_code::FnEsc:

    break;
  case EKey_code::NONE:

    break;
  }
}
