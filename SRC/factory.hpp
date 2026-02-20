#pragma once

#include "crc16.hpp"
#include "settings_eep.hpp"
#include "set_uart.hpp"
#include "set_spi.hpp"
#include "adc.hpp"

#include "controllerDMA.hpp"
#include "bool_name.hpp"

#include "mb_slave.hpp" 
#include "process.hpp"

#include "terminalUartDriver.hpp"
#include "menu_navigation.hpp"

class CFactory {
public:    
  static StatusRet load_settings();                 
  static CMBSLAVE create_MBslave(); 
  static CPROCESS create_Process();
  static CMenuNavigation& create_MN(CPROCESS&);  
};

