#pragma once

namespace bg {
  // Биты физических связей GPIO
  static constexpr unsigned short B_ULED      = 9;        // Бит U-LED
  static constexpr unsigned short B0_PORT_OUT = 24;       // 1-й бит DOUT порта GPIO2  
  static constexpr unsigned int B0_PORT_IN    = 15;       // 0-й бит порта. Начало Pi0 в PORT2 контроллера
  static constexpr unsigned int B_LampMeas = 31;
  static constexpr unsigned int B_LampAlarm1 = 30;
  static constexpr unsigned int B_LampAlarm2 = 29;
  static constexpr unsigned int B_RelReady = 21;
  static constexpr unsigned int B_RelAlarm1 = 25;
  static constexpr unsigned int B_RelAlarm2 = 26;
  static constexpr unsigned int B_TN = 27;
  static constexpr unsigned int B_TP = 28;
}

namespace bf {
  // Биты управления функциями GPIO
  static constexpr unsigned int D_MODE_PULLUP = 0x02 << 3;
  static constexpr unsigned int IOCON_DAC0_EN = 0x00010002;
  static constexpr unsigned int IOCON_PORT_PWM = 1;
  static constexpr unsigned int IOCON_CH_ADC_IOCON = 1;
  
  static constexpr unsigned int IOCON_SPI0 = 0x02;
  static constexpr unsigned int IOCON_SPI1 = 0x03;
  static constexpr unsigned int IOCON_SPI2 = 0x02;
  
  static constexpr unsigned int IOCON_U0_TXD  = 0x1;  
  static constexpr unsigned int IOCON_U0_RXD  = 0x1;
  
  static constexpr unsigned int IOCON_U2_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U2_RXD  = 0x2;
  static constexpr unsigned int IOCON_U2_OE   = 0x4;
  
  static constexpr unsigned int IOCON_U3_TXD  = 0x2;  
  static constexpr unsigned int IOCON_U3_RXD  = 0x2;
  static constexpr unsigned int IOCON_U3_OE   = 0x5;   
  
  static constexpr unsigned int IOCON_T2_CAP1 = 0x23;
  static constexpr unsigned int IOCON_T3_CAP1 = 0x23;
  
  static constexpr unsigned int IOCON_CAN1 = 0x01;
  static constexpr unsigned int IOCON_CAN2 = 0x02;    
  
  static constexpr unsigned int IOCON_EINT2 = 0x01;
  static constexpr unsigned int LineEINT2 = 2;
  static constexpr unsigned int EINT2_BIT_MARK = 0x04;
 
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
  