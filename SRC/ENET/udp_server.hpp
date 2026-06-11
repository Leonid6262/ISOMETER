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
    
    alignas(4) unsigned char Rx_Frame[CEMAC::ETH_FRAG_SIZE];
    alignas(4) unsigned char Tx_Frame[CEMAC::ETH_FRAG_SIZE];
    
    unsigned char My_IP[4];

    void handleARP();
    void handleIP();
    void handleUDP();
    void handleICMP_Ping();

    // Вспомогательный метод для динамического обновления IP из уставок
    void updateCurrentIP() {
        My_IP[0] = rSet.getSettings().ip3;
        My_IP[1] = rSet.getSettings().ip2;
        My_IP[2] = rSet.getSettings().ip1;
        My_IP[3] = rSet.getSettings().ip0;
    }
    
    // Эфирные типы (EtherType)
    static constexpr unsigned short ETH_TYPE_ARP  = 0x0806;
    static constexpr unsigned short ETH_TYPE_IPV4 = 0x0800;
    static constexpr unsigned short ETH_MIN_FRAME_LEN = 60; // Минимальная длина кадра в Ethernet

    // Коды протоколов внутри IP
    static constexpr unsigned char  IP_PROTO_ICMP = 1;
    static constexpr unsigned char  IP_PROTO_UDP  = 17;

    // ARP константы
    static constexpr unsigned short ARP_OP_REQUEST = 1;
    static constexpr unsigned short ARP_MSG_LEN    = 42;
    // Перевернутый Opcode Reply для записи напрямую в буфер (0x0002 -> Big-Endian -> 0x0200)
    static constexpr unsigned short ARP_OP_REPLY_NET  = 0x0200;

    // ICMP константы
    static constexpr unsigned char  ICMP_TYPE_ECHO_REQ = 8;
    static constexpr unsigned char  ICMP_TYPE_ECHO_RPL = 0;
    
    // Дельта для быстрого пересчета чексуммы: (Request Type (8) - Reply Type (0)) << 8 = 0x0800
    static constexpr unsigned short ICMP_CHKSUM_DELTA = 0x0800;

public:
  
    CUDP_Server(CENET_DRV& eth_drv, CEEPSettings&);
    
    CENET_DRV& rEnet_drv;
    
    
    unsigned char* getRxBuffer()  { return Rx_Frame; }
    unsigned char* getTxBuffer()  { return Tx_Frame; }
    unsigned char* getMyIPPtr()   { return My_IP; }
    
    void setModbusSlave(CMB_UDP_Slave* pMB_Slave) { this->pMB_Slave = pMB_Slave; }
    
    void poll();
};
