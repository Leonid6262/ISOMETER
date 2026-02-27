#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "mbUartDriver.hpp"

class CMBSLAVE {
  
private:
 LPC_UART_TypeDef* UART;
 CMbUartDriver& rUartDrv;

public:
  
 CMBSLAVE(CMbUartDriver&, CDMAcontroller&, LPC_UART_TypeDef*);
 CDMAcontroller& rDMAc;
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x1F;  
 
 static unsigned short tx_mbs_buffer[];
 static unsigned short rx_mbs_buffer[];
 
 void monitor();
  

};
