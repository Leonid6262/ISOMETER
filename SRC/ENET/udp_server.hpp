#pragma once
#include "settings_eep.hpp" 
#include "enet_drv.hpp"
#include "net_structs.hpp"
#include "mb_udp_slave.hpp"
#include <string.h>

class CUDP_Server {
private:
    CEEPSettings& rSet;
    CENET_DRV& m_eth;
    CMB_UDP_Slave* m_mb_slave;
    
    unsigned char Rx_Frame[CEMAC::ETH_FRAG_SIZE];
    unsigned char Tx_Frame[CEMAC::ETH_FRAG_SIZE];
    
    unsigned char m_my_ip[4];

    void handleARP(SEthernetHeader* eth);
    void handleIP(SEthernetHeader* eth);
    void handleICMP(SEthernetHeader* eth, SIPHeader* ip);
    void handleUDP(SEthernetHeader* eth, SIPHeader* ip);

    // Вспомогательный метод для динамического обновления IP из уставок
    void updateCurrentIP() {
        m_my_ip[0] = rSet.getSettings().ip3;
        m_my_ip[1] = rSet.getSettings().ip2;
        m_my_ip[2] = rSet.getSettings().ip1;
        m_my_ip[3] = rSet.getSettings().ip0;
    }

public:
    CUDP_Server(CENET_DRV& eth_drv, CEEPSettings&);
    
    unsigned char* getRxBuffer()  { return Rx_Frame; }
    unsigned char* getTxBuffer()  { return Tx_Frame; }
    unsigned char* getMyIPPtr()   { return m_my_ip; }
    
    void setModbusSlave(CMB_UDP_Slave* slave) { m_mb_slave = slave; }
    
    void poll();
};
