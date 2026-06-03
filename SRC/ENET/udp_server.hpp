#pragma once
#include "settings_eep.hpp" 
#include "enet_drv.hpp"
#include "net_structs.hpp"
#include <string.h>

class CUDP_Server {
private:
    CEEPSettings& rSet;
    CENET_DRV& m_eth;
    unsigned char m_rx_buffer[CEMAC::ETH_FRAG_SIZE];
    unsigned char m_my_ip[4];

    void handleARP(SEthernetHeader* eth);
    void handleIP(SEthernetHeader* eth);
    void handleICMP(SEthernetHeader* eth, SIPHeader* ip);
    void handleUDP(SEthernetHeader* eth, SIPHeader* ip);

    // Вспомогательный метод для динамического обновления IP из EEPROM уставки
    void updateCurrentIP() {
        m_my_ip[0] = rSet.getSettings().ip3;
        m_my_ip[1] = rSet.getSettings().ip2;
        m_my_ip[2] = rSet.getSettings().ip1;
        m_my_ip[3] = rSet.getSettings().ip0;
    }

public:
    CUDP_Server(CENET_DRV& eth_drv, CEEPSettings&);
    void poll();
};
