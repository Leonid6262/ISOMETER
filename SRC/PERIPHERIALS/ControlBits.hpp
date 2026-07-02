#pragma once

namespace bg {
  // Биты физических связей GPIO
  static constexpr unsigned short B_ULED = 28;        // Бит U-LED
  
  static constexpr unsigned short B_LampReady = 27;
  static constexpr unsigned short B_LampAlarm1 = 25;
  static constexpr unsigned short B_LampAlarm2 = 26;
  
  static constexpr unsigned short B_LampPult1 = 29;
  static constexpr unsigned short B_LampPult2 = 30;
  
  static constexpr unsigned short B_RelReady = 20;
  static constexpr unsigned short B_RelAlarm1 = 21;
  static constexpr unsigned short B_RelAlarm2 = 22;
  
  static constexpr unsigned short B_TN = 23;
  static constexpr unsigned short B_TP = 18;
  
  static constexpr unsigned short C_P40 = 29;
}

namespace bf {
  // Биты управления функциями GPIO
  static constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;
  
  static constexpr unsigned int IOCON_PORT_PWM = 2;
  
  static constexpr unsigned int IOCON_SPI1 = 0x82;
  
  static constexpr unsigned int IOCON_U0_TXD  = 0x4;  
  static constexpr unsigned int IOCON_U0_RXD  = 0x4;
  
  static constexpr unsigned int IOCON_U2_TXD  = 0x1;  
  static constexpr unsigned int IOCON_U2_RXD  = 0x1;
  static constexpr unsigned int IOCON_U2_OE   = 0x6;
 
}

namespace bPWM {
  static constexpr unsigned int MAT0LATCHEN = 1UL << 0;
  static constexpr unsigned int MAT5LATCHEN = 1UL << 5;
  static constexpr unsigned int CE_PWMEN = 0x09;
  static constexpr unsigned int PWMENA5 = 1UL << 13;
  static constexpr unsigned int TCR_RESET = 0x002;
} 

namespace bRTC {  
  static constexpr unsigned int RTC_OSCF = 1<<4; // RTC Oscillator Fail detect flag 
  static constexpr unsigned int CLKEN    = 1<<0; // Clock Enable.
  static constexpr unsigned int CCALDS   = 1<<4; // Calibration counter disable.
}

namespace bEMC {  
  static constexpr unsigned int FUNC_ENET   = 0x01;
  static constexpr unsigned int MODE_PULLUP = 0x02 << 3;  // Pull-up resistor
}

namespace bUART {  
  enum RegisterFlags {
    THRE         = 1UL << 5,
    RDR          = 1UL << 0,
    DCTRL        = 1UL << 4,
    OINV         = 1UL << 5,
    LCR_DLAB_ON  = 0x80,        // b7-DLAB 
    LCR_DLAB_OFF = 0x03,        // Чётность откл., 1-стоп бит, символ 8 бит
    FIFOEN       = 0x07,        // b2-очистка TXFIFO, b1-очистка RXFIFO, b0-вкл FIFO
    TXEN         = 1UL << 7     // Разрешение передачи
  }; 
}
  