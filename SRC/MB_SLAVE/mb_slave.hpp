#pragma once 
#include "bool_name.hpp"
#include "controllerDMA.hpp"
#include "mbUartDriver.hpp"
#include "mbDataProxy.hpp"
#include "crc16.hpp"

class CPROCESS;

class CMBSLAVE {
  
private:
 CModbusDataProxy& rModbusData; 
 CMBUartDriver& rUartDrv;

 enum class StatusTO { NO_RECEPTION, NOT_EXPIRED, EXPIRED };
 StatusTO TimeoutStatus();
 void ParseRequestF346();
 void Answer(unsigned char);
 
 static constexpr unsigned char F03 = 3;
 static constexpr unsigned char F04 = 4;
 static constexpr unsigned char F06 = 6;
 
 static constexpr unsigned char LRequestF346 = 8;
 
 static constexpr unsigned short bAddressSlave  = 0;
 static constexpr unsigned short bFunction      = 1;
 static constexpr unsigned short bStartAddress  = 2;
 static constexpr unsigned short bAddressReg    = 2;
 static constexpr unsigned short bQuantity      = 4;
 static constexpr unsigned short bValue         = 4;
 static constexpr unsigned short bCRC16         = 6;
 
 static constexpr unsigned int MODBUS_SILENCE_TICKS = 50000; // 5 мс
 
 unsigned char* pAddressSlave;
 unsigned char Function;
 unsigned short StartingAddress;
 unsigned short AddressReg;
 unsigned short Quantity;
 unsigned short Value;

public: 
 CMBSLAVE(CMBUartDriver&, CModbusDataProxy&, unsigned char*);
 
 static constexpr unsigned short TRANSACTION_LENGTH = 0x3F;  
 
 static unsigned char tx_mbs_buffer[];
 static unsigned char rx_mbs_buffer[];
 
 void monitor(); 

};
