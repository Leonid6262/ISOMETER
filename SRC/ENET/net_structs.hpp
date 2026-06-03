#pragma once
#pragma pack(push, 1)

// 1. Заголовок Ethernet (14 байт)
struct SEthernetHeader {
    unsigned char  dest_mac[6];
    unsigned char  src_mac[6];
    unsigned short type; // IPv4 = 0x0800, ARP = 0x0806 (в Network byte order)
};

// 2. Заголовок ARP (28 байт)
struct SARPHeader {
    unsigned short hw_type;       // Тип сети (Ethernet = 0x0001)
    unsigned short proto_type;    // Тип протокола (IP = 0x0800)
    unsigned char  hw_len;        // Длина MAC = 6
    unsigned char  proto_len;     // Длина IP = 4
    unsigned short opcode;        // 1 = Request, 2 = Reply
    unsigned char  sender_mac[6];
    unsigned char  sender_ip[4];
    unsigned char  target_mac[6];
    unsigned char  target_ip[4];
};

// 3. Заголовок IPv4 (20 байт)
struct SIPHeader {
    unsigned char  ver_ihl;       // Версия и длина заголовка
    unsigned char  tos;           // Тип службы
    unsigned short total_length;  // Полная длина пакета
    unsigned short id;            // Идентификатор фрагмента
    unsigned short flags_offset;  // Флаги и смещение
    unsigned char  ttl;           // Время жизни пакета
    unsigned char  protocol;      // Протокол (ICMP = 1, UDP = 17)
    unsigned short checksum;      // Контрольная сумма заголовка IP
    unsigned char  src_ip[4];
    unsigned char  dest_ip[4];
};

// 4. Заголовок ICMP (8 байт базовый)
struct SICMPHeader {
    unsigned char  type;          // 8 = Request, 0 = Reply
    unsigned char  code;          // Код
    unsigned short checksum;      // Контрольная сумма
    unsigned short id;            // ID
    unsigned short sequence;      // Номер последовательности
};

#pragma pack(pop)




