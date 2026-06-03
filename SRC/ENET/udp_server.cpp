#include "udp_server.hpp"
#include "settings_eep.hpp"
#include <stdio.h>

CUDP_Server::CUDP_Server(CENET_DRV& m_eth, CEEPSettings& rSet) : m_eth(m_eth), rSet(rSet) { updateCurrentIP(); }

void CUDP_Server::poll() {
    // 1. Проверяем, пришёл ли легитимный кадр (проверка CRC уже внутри драйвера)
    if (m_eth.receiveFrame(m_rx_buffer) != CENET_DRV::ReceiveStatus::FRAME_RECIVED) {
        return; 
    }

    // Динамически обновляем IP из настроек, на случай если уставка изменилась на лету
    updateCurrentIP();

    SEthernetHeader* eth = reinterpret_cast<SEthernetHeader*>(m_rx_buffer);
    unsigned short eth_type = __REV16(eth->type);

    if (eth_type == 0x0806) {
        handleARP(eth);
    } 
    else if (eth_type == 0x0800) {
        handleIP(eth);
    }
}

void CUDP_Server::handleARP(SEthernetHeader* eth) {
    SARPHeader* arp = reinterpret_cast<SARPHeader*>(m_rx_buffer + sizeof(SEthernetHeader));

    // Проверяем: это запрос (opcode == 1) и ищут именно наш IP?
    if (__REV16(arp->opcode) == 1 && memcmp(arp->target_ip, m_my_ip, 4) == 0) 
    {
        // --- 1. Разворачиваем Ethernet-заголовок ---
        memcpy(eth->dest_mac, eth->src_mac, 6); // ПК становится получателем
        memcpy(eth->src_mac, G_CONST::MAC_Controller, 6); // Мы — отправитель

        // --- 2. Модифицируем ARP-заголовок ---
        arp->opcode = __REV16(2); // Меняем тип на Reply (Ответ)

        // Меняем местами данные узлов
        memcpy(arp->target_mac, arp->sender_mac, 6);
        memcpy(arp->target_ip, arp->sender_ip, 4);   // IP ПК теперь Target

        memcpy(arp->sender_mac, G_CONST::MAC_Controller, 6);
        memcpy(arp->sender_ip, m_my_ip, 4);          // Наш IP теперь Sender

        // Зануляем остаток пакета (Padding) до минимальных 60 байт кадра (без учёта CRC)
        // Структуры Eth(14) + ARP(28) = 42 байта. Нужно добить еще 18 байт нулями.
        memset(m_rx_buffer + sizeof(SEthernetHeader) + sizeof(SARPHeader), 0, 18);

        // Отправляем модифицированный буфер обратно
        m_eth.sendFrame(m_rx_buffer);
    }
}

void CUDP_Server::handleIP(SEthernetHeader* eth) {
    SIPHeader* ip = reinterpret_cast<SIPHeader*>(m_rx_buffer + sizeof(SEthernetHeader));

    // Проверяем, что пакет пришёл именно на наш IP
    if (memcmp(ip->dest_ip, m_my_ip, 4) != 0) {
        return;
    }

    if (ip->protocol == 1) { // Протокол ICMP (Ping)
        handleICMP(eth, ip);
    } 
    else if (ip->protocol == 17) { // Протокол UDP
        handleUDP(eth, ip);
    }
}

void CUDP_Server::handleICMP(SEthernetHeader* eth, SIPHeader* ip) {
    // Находим начало ICMP-заголовка
    SICMPHeader* icmp = reinterpret_cast<SICMPHeader*>(m_rx_buffer + sizeof(SEthernetHeader) + sizeof(SIPHeader));

    if (icmp->type == 8) // Echo Request (Запрос пинга)
    {
        // --- 1. Разворачиваем Ethernet-заголовок ---
        memcpy(eth->dest_mac, eth->src_mac, 6);
        memcpy(eth->src_mac, G_CONST::MAC_Controller, 6);

        // --- 2. Разворачиваем IP-заголовок ---
        // Так как меняем адреса местами, общая сумма IP не меняется. Просто копируем.
        memcpy(ip->dest_ip, ip->src_ip, 4);
        memcpy(ip->src_ip, m_my_ip, 4);

        // --- 3. Модифицируем ICMP ---
        icmp->type = 0; // Превращаем в Echo Reply (Ответ)

        // Дифференциальный пересчет контрольной суммы ICMP (быстрая поправка на смену типа 8 -> 0)
        unsigned int icmp_chk = __REV16(icmp->checksum);
        icmp_chk += 0x0800;
        if (icmp_chk > 0xFFFF) icmp_chk += 1;
        icmp->checksum = __REV16(static_cast<unsigned short>(icmp_chk));

        // Отправляем изменённый буфер в сеть
        m_eth.sendFrame(m_rx_buffer);
    }
}

void CUDP_Server::handleUDP(SEthernetHeader* eth, SIPHeader* ip) {
    // Здесь будет жить обработчик
}


