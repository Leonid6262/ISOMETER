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
                                         
    static constexpr unsigned short _port_502 = 502;
    
    // Смещения сетевых уровней (размеры заголовков)
    static constexpr unsigned short ETH_HDR_LEN = 14; // Ethernet II заголовок
    static constexpr unsigned short IP_HDR_LEN  = 20; // IPv4 заголовок (минимальный)
    static constexpr unsigned short UDP_HDR_LEN = 8;  // UDP заголовок

    // Точки входа для протокола Modbus
    // 14 + 20 + 8 = 42 (Начало заголовка MBAP)
    static constexpr unsigned short MBAP_OFFSET = ETH_HDR_LEN + IP_HDR_LEN + UDP_HDR_LEN; 
    
    static constexpr unsigned short MBAP_HDR_LEN = 7;  // Длина самого заголовка MBAP (Transaction ID .. Unit ID)

    // 42 + 7 = 49 (Начало Modbus PDU — байт функции)
    static constexpr unsigned short PDU_OFFSET  = MBAP_OFFSET + MBAP_HDR_LEN; 

    // Внутренние смещения внутри MBAP (относительно начала MBAP)
    static constexpr unsigned short MBAP_UNIT_ID_IDX = 6; // 6-й байт внутри MBAP
    
    static constexpr unsigned char  IP_VERSION_IHL_DEFAULT = 0x45; // IPv4, длина заголовка 5 слов (20 байт)
    static constexpr unsigned char  IP_PROTO_UDP           = 17;   // Код протокола UDP в заголовке IP
    static constexpr unsigned short ETH_TYPE_IPV4          = 0x0800;

    // Смещения внутри IP-заголовка (относительно начала IP-заголовка, то есть rx_frame_ptr + 14)
    static constexpr unsigned short IP_VERSION_IHL_IDX     = 0;   // 14-й байт в кадре
    static constexpr unsigned short IP_TOS_IDX             = 1;   // 15-й байт в кадре
    static constexpr unsigned short IP_PROTOCOL_IDX        = 9;   // 23-й байт в кадре (14 + 9 = 23)

    // Константы для расчета контрольной суммы IP
    // 20 байт заголовка IP — это ровно 10 двухбайтовых слов (unsigned short)
    static constexpr unsigned short IP_HDR_WORDS_COUNT    = IP_HDR_LEN / 2;
    
    static constexpr unsigned char Illegal_Function = 1;
    static constexpr unsigned char Illegal_Data_Addr = 2;

public:

    CMB_UDP_Slave(CModbusDataProxy&, CUDP_Server&, unsigned char*);
    
    void handleRequest(SEthernetHeader*, SIPHeader*, SUDPHeader*);
};

