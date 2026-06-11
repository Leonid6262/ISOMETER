#include "udp_server.hpp"
#include "settings_eep.hpp"
#include <stdio.h>

CUDP_Server::CUDP_Server(CENET_DRV& rEnet_drv, CEEPSettings& rSet) : rEnet_drv(rEnet_drv), rSet(rSet) { updateCurrentIP(); }

void CUDP_Server::poll() {
  
  // Пакет принят в буфер Rx_Frame
  if (rEnet_drv.receiveFrame(Rx_Frame) != CENET_DRV::ReceiveStatus::FRAME_RECIVED) { return; }
  
  updateCurrentIP();
  
  SEthernetHeader* rx_eth_header = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  
  unsigned short eth_type = __REV16(rx_eth_header->type);
  
  switch (eth_type) {
  case ETH_TYPE_ARP: 
    handleARP(); // Обработка ARP запроса
    break;
  case ETH_TYPE_IPV4:
    handleIP(); // Обработка IP запроса
    break;
  }
}

void CUDP_Server::handleARP() { 
  
  // Вместо 14 используем sizeof(SEthernetHeader)
  const auto* rx_arp = reinterpret_cast<const SARPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  
  // Ранний выход, если это не запрос или ищут не нас
  if (!(__REV16(rx_arp->opcode) == ARP_OP_REQUEST && memcmp(rx_arp->target_ip, My_IP, _ip_length) == 0)) return;
  
  // 1. Копируем заголовок (Eth + ARP) во внутренний Tx_Frame для сборки ответа
  memcpy(Tx_Frame, Rx_Frame, ARP_MSG_LEN);
  
  auto* tx_eth = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
  auto* rx_eth = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  auto* tx_arp = reinterpret_cast<SARPHeader*>(Tx_Frame + sizeof(SEthernetHeader));
  
  // --- 2. Рокировка Ethernet-заголовка ---
  memcpy(tx_eth->dest_mac, rx_eth->src_mac, _mac_length); 
  memcpy(tx_eth->src_mac,  G_CONST::MAC_Controller, _mac_length); 
  
  // --- 3. Модификация ARP-данных ---
  tx_arp->opcode = ARP_OP_REPLY_NET; // Используем готовую Big-Endian константу
  
  // Настраиваем target (куда шлем): копируем из sender'а входящего пакета
  memcpy(tx_arp->target_mac, rx_arp->sender_mac, _mac_length); 
  memcpy(tx_arp->target_ip,  rx_arp->sender_ip,  _ip_length); 
  
  // Настраиваем sender (себя): пишем наш железный MAC и статический IP
  memcpy(tx_arp->sender_mac, G_CONST::MAC_Controller, _mac_length); 
  memcpy(tx_arp->sender_ip,  My_IP, _ip_length);
  
  // 4. Отправляем
  rEnet_drv.sendFrame(Tx_Frame, ARP_MSG_LEN);

}

void CUDP_Server::handleIP() { 
  
  SIPHeader* rx_ip_header = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  
  // Проверяем, что пакет пришёл именно на наш IP
  if (memcmp(rx_ip_header->dest_ip, My_IP, 4) != 0) { return; }
  
  
  switch (rx_ip_header->protocol) {
  case IP_PROTO_ICMP:
    handleICMP_Ping(); // Обработка протокол ICMP (Ping)  
    break;
  case IP_PROTO_UDP:
    handleUDP();  // Обработка протокол UDP (ModBus) 
    break;
  }
  
}

void CUDP_Server::handleICMP_Ping() {
// Накладываем структуры на входящий кадр
  const auto* rx_eth_header  = reinterpret_cast<const SEthernetHeader*>(Rx_Frame);
  const auto* rx_ip_header   = reinterpret_cast<const SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  const auto* rx_icmp_header = reinterpret_cast<const SICMPHeader*>(Rx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  // Ранний выход, если это не эхо-запрос
  if (rx_icmp_header->type != ICMP_TYPE_ECHO_REQ) return; 
  
  // Вычисляем полную длину входящего ICMP пакета по IP заголовку
  unsigned short full_frame_len = sizeof(SEthernetHeader) + __REV16(rx_ip_header->total_length);
  
  // Копируем весь кадр из Rx в Tx для сохранения Payload (данных пинга)
  memcpy(Tx_Frame, Rx_Frame, full_frame_len);
  
  // Инициализируем передающие указатели на Tx_Frame
  auto* tx_eth_header  = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
  auto* tx_ip_header   = reinterpret_cast<SIPHeader*>(Tx_Frame + sizeof(SEthernetHeader));
  auto* tx_icmp_header = reinterpret_cast<SICMPHeader*>(Tx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  // --- 1. Рокировка Ethernet в Tx_Frame ---
  memcpy(tx_eth_header->dest_mac, rx_eth_header->src_mac, _mac_length);
  memcpy(tx_eth_header->src_mac,  G_CONST::MAC_Controller, _mac_length);
  
  // --- 2. Рокировка IP в Tx_Frame ---
  memcpy(tx_ip_header->dest_ip, rx_ip_header->src_ip, _ip_length);
  memcpy(tx_ip_header->src_ip,  My_IP, _ip_length);
  
  // --- 3. Модифицируем ICMP в Tx_Frame ---
  tx_icmp_header->type = ICMP_TYPE_ECHO_RPL; // Используем константу ответа (0)
  
  // Быстрый пересчет контрольной суммы (выполняется в Tx_Frame)
  unsigned int icmp_chk = __REV16(tx_icmp_header->checksum);
  icmp_chk += ICMP_CHKSUM_DELTA;
  if (icmp_chk > 0xFFFF) {
      icmp_chk += 1;
  }
  tx_icmp_header->checksum = __REV16(static_cast<unsigned short>(icmp_chk));
  
  // Отправляем собранный кадр
  rEnet_drv.sendFrame(Tx_Frame, full_frame_len);  

}

void CUDP_Server::handleUDP() {
  
  if (pMB_Slave == nullptr) return;
  
  SIPHeader* rx_ip_header = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  SEthernetHeader* rx_eth_header = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  // Указатель UDP-заголовка в текущем кадре (Rx_Frame)
  SUDPHeader* rx_udp_header = reinterpret_cast<SUDPHeader*>(Rx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  pMB_Slave->handleRequest(rx_eth_header, rx_ip_header, rx_udp_header); // handleRequest сам разберется с Unit ID, функциями и заполнением TX буфера
  
}
