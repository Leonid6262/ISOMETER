#pragma once 
#include "bool_name.hpp"
#include "settings_eep.hpp"
#include "controllerDMA.hpp"
#include "mbUartDriver.hpp"

class CMBSLAVE {
  
private:
 CMbUartDriver& rUartDrv;
 
 StatusTO TimeoutStatus();
 
 struct {
   unsigned char  SlaveAddress;    
   unsigned char  FunctionCode;
   //unsigned short StartingAddress;
   //unsigned short QuantityRegisters;
   unsigned short CRC16;
 } Request;//_03F;
 
 static constexpr unsigned int MODBUS_SILENCE_TICKS = 50000; // 5 мс

public:
  
 CMBSLAVE(CMbUartDriver&, CDMAcontroller&);
 CDMAcontroller& rDMAc;
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x3F;  
 
 static unsigned char tx_mbs_buffer[];
 static unsigned char rx_mbs_buffer[];
 
 void monitor();
  

};
