#pragma once

#include "crc16.hpp"
#include "settings_eep.hpp"
#include "set_uart.hpp"
#include "set_spi.hpp"
#include "adc.hpp"
#include "controllerDMA.hpp"
#include "bool_name.hpp"
#include "mbUartDriver.hpp"
#include "mb_rtu_slave.hpp"
#include "mb_udp_slave.hpp" 
#include "out_4_20mA.hpp"
#include "dac.hpp"
#include "process.hpp"
#include "terminalUartDriver.hpp"
#include "menu_navigation.hpp"
#include "log_display.hpp"
#include "mbDataProxy.hpp"
#include "rtc.hpp"
#include "PeripheralsInit.hpp"
#include "event_log.hpp"
#include "terminal_manager.hpp"
#include "emac.hpp"
#include "enet_drv.hpp"
#include "udp_server.hpp"


class CFactory {
public:     
  static void Peripherals_init(); 
  static StatusRet load_settings();                 
  static COUT_4_20 create_OUT_4_20(); 
  static CMBSLAVE create_MBslave();
  static CUDP_Server create_UDP_Server(); 
  static CPROCESS& create_Process(); 
  static CTerminalManager& createTM(CPROCESS&);  
  static void control_set(CTerminalManager&);  
};

