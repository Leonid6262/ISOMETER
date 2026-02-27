#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "mbUartDriver.hpp"

class CMBSLAVE {
  
private:
 CMbUartDriver& rUartDrv;

public:
  
 CMBSLAVE(CMbUartDriver&, CDMAcontroller&);
 CDMAcontroller& rDMAc;
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x3F;  
 
 static unsigned char tx_mbs_buffer[];
 static unsigned char rx_mbs_buffer[];
 
 void monitor();
  

};
