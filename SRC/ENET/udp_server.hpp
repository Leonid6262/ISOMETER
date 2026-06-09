#pragma once
#include "settings_eep.hpp" 
#include "enet_drv.hpp"
#include "net_structs.hpp"
#include "mb_udp_slave.hpp"
#include <string.h>

class CMB_UDP_Slave;

class CUDP_Server {
private:
    CEEPSettings& rSet;
    CMB_UDP_Slave* pMB_Slave;
    
    unsigned char Rx_Frame[CEMAC::ETH_FRAG_SIZE];
    unsigned char Tx_Frame[CEMAC::ETH_FRAG_SIZE];
    
    unsigned char My_IP[4];

    void handleARP();
    void handleIP();
    void handleICMP();
    void handleUDP();

    // Вспомогательный метод для динамического обновления IP из уставок
    void updateCurrentIP() {
        My_IP[0] = rSet.getSettings().ip3;
        My_IP[1] = rSet.getSettings().ip2;
        My_IP[2] = rSet.getSettings().ip1;
        My_IP[3] = rSet.getSettings().ip0;
    }

public:
  
    CUDP_Server(CENET_DRV& eth_drv, CEEPSettings&);
    
    CENET_DRV& rEnet_drv;
    
    
    unsigned char* getRxBuffer()  { return Rx_Frame; }
    unsigned char* getTxBuffer()  { return Tx_Frame; }
    unsigned char* getMyIPPtr()   { return My_IP; }
    
    void setModbusSlave(CMB_UDP_Slave* pMB_Slave) { this->pMB_Slave = pMB_Slave; }
    
    void poll();
};
