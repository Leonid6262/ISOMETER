#pragma once 
#include "bool_name.hpp"
#include "mbDataProxy.hpp"
#include "crc16.hpp"
#include "net_structs.hpp"       
#include "enet_drv.hpp"   
#include "settings_eep.hpp" 

class CMB_UDP_Slave {
private:
    CENET_DRV& m_eth;
    CModbusDataProxy& rModbusData;
    unsigned char* m_rx_buffer; // Только для чтения запроса
    unsigned char* m_tx_buffer; // Только для сборки ответа
    unsigned char* pAddressSlave;
    unsigned char* my_ip;
                                         
    static constexpr unsigned char Illegal_Function = 0x01;
    static constexpr unsigned char Illegal_Data_Addr = 0x02;

    // Служебные методы (ОБНОВЛЕНЫ: добавлены p_rxIpHdr и p_rxUdpHdr)
    void sendException(SEthernetHeader* p_txEthHdr, SIPHeader* p_txIpHdr, SUDPHeader* p_txUdpHdr, 
                       SModbusMBAP* p_txMbapHdr, SIPHeader* p_rxIpHdr, SUDPHeader* p_rxUdpHdr,
                       unsigned char function, unsigned char exceptionCode);

void finalizeAndSend(SIPHeader* p_rxIpHdr, SUDPHeader* p_rxUdpHdr, unsigned short pdu_length);

public:
    CMB_UDP_Slave(CModbusDataProxy&, CENET_DRV&, unsigned char*, unsigned char*, unsigned char*, unsigned char*);
   
    void handleRequest(SEthernetHeader* p_ethHdr, SIPHeader* p_ipHdr, SUDPHeader* p_udpHdr);
};

