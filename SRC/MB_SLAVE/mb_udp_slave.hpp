#pragma once 
#include "bool_name.hpp"
#include "mbDataProxy.hpp"
#include "crc16.hpp"
#include "net_structs.hpp"       
#include "settings_eep.hpp" 
#include "udp_server.hpp"

class CUDP_Server;

class CMB_UDP_Slave {
private:
    CModbusDataProxy& rModbusData;
    CUDP_Server& pUDP_Server;
    unsigned char* pAddressSlave;
                                         
    static constexpr unsigned char Illegal_Function = 0x01;
    static constexpr unsigned char Illegal_Data_Addr = 0x02;

public:

    CMB_UDP_Slave(CModbusDataProxy&, CUDP_Server&, unsigned char*);
    
    void handleRequest(SEthernetHeader*, SIPHeader*, SUDPHeader*);
};

