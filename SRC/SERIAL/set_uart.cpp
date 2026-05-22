#include "set_uart.hpp"

LPC_UART_TypeDef* CSET_UART::configure(EUartInstance UN, CEEPSettings& rSet) { 
  LPC_UART_TypeDef* UART = nullptr;

  switch (UN) {
    case EUartInstance::UART_0:
      UART = P::UART0; 
      /* Настройка частоты */
      UART->TER = 0x00;                 // Запрет передачи на момент настройки
      UART->LCR = bUART::LCR_DLAB_ON;   // DLAB вкл.
      UART->DLM = baud_19200.DLM;
      UART->DLL = baud_19200.DLL;
      UART->FDR = baud_19200.FDR;
      break;
    case EUartInstance::UART_2:
      UART = P::UART2; 
      UART->RS485CTRL = bUART::DCTRL;           // Автоматическое переключение OE
      UART->RS485CTRL |= bUART::OINV;           // Инверсия OE
      /* Настройка частоты */
      UART->TER = 0x00;                         // Запрет передачи на момент настройки
      UART->LCR = bUART::LCR_DLAB_ON;           // DLAB вкл.
      switch (rSet.getSettings().Baud_rate) {
      case 1:
        UART->DLM = baud_9600.DLM;
        UART->DLL = baud_9600.DLL;
        UART->FDR = baud_9600.FDR;
        break;
      case 2:
        UART->DLM = baud_19200.DLM;
        UART->DLL = baud_19200.DLL;
        UART->FDR = baud_19200.FDR;
        break;
      case 3:
        UART->DLM = baud_38400.DLM;
        UART->DLL = baud_38400.DLL;
        UART->FDR = baud_38400.FDR;
        break;
      case 4:
        UART->DLM = baud_57600.DLM;
        UART->DLL = baud_57600.DLL;
        UART->FDR = baud_57600.FDR;        
        break;
      case 5:
        UART->DLM = baud_115200.DLM;
        UART->DLL = baud_115200.DLL;
        UART->FDR = baud_115200.FDR;        
        break;
      }
      break;
  }
  UART->IER = 0;
  UART->LCR = bUART::LCR_DLAB_OFF;  // b7 - DLAB откл., чётность откл., 1-стоп бит, символ 8бит
  UART->FCR = bUART::FIFOEN;        // FIFO. b2-очистка TXFIFO, b1-очистка RXFIFO, b0-вкл FIFO
  UART->TER = bUART::TXEN;          // Разрешение передачи
  while (UART->LSR & bUART::RDR) {
    unsigned int tmp = UART->RBR;  // Очистка приёмника
  }
  while (!(UART->LSR & bUART::THRE)) {};  // Очистка передатчика

  return UART;
}
