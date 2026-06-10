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
  // В ARP-пакете Opcode запроса/ответа лежит в байтах 20 и 21 кадра Rx_Frame
  unsigned short opcode = (Rx_Frame[ARP_OP_INDEX] << 8) | Rx_Frame[ARP_OP_INDEX +1];
  
  // Target IP (тот, кого ищут в сети) всегда лежит в байтах 38, 39, 40, 41 кадра Rx_Frame
  if (opcode == ARP_OP_REQUEST && 
      Rx_Frame[IP_TARGET_INDEX + 0] == My_IP[0] &&  
      Rx_Frame[IP_TARGET_INDEX + 1] == My_IP[1] && 
      Rx_Frame[IP_TARGET_INDEX + 2] == My_IP[2] && 
      Rx_Frame[IP_TARGET_INDEX + 3] == My_IP[3]) 
  {
    // 1. Копируем заголовок (Eth + ARP) во внутренний Tx_Frame для сборки ответа
    memcpy(Tx_Frame, Rx_Frame, ARP_MSG_LEN);
    
    SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
    SEthernetHeader* rx_eth = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
    
    // --- 2. Рокировка Ethernet-заголовка ---
    memcpy(tx_eth->dest_mac, rx_eth->src_mac, 6); 
    memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6); 
    
    // --- 3. Модификация ARP-данных строго по физическим индексам Tx_Frame ---
    Tx_Frame[ARP_OP_INDEX]   = 0x00; // Opcode Hi 
    Tx_Frame[ARP_OP_INDEX+1] = ARP_OP_REPLY; // Opcode Lo -> Reply (Ответ)
    
    // Настраиваем target (куда шлем): копируем из sender'а входящего пакета
    // В Rx_Frame MAC отправителя — это [22..27], IP отправителя — [28..31]
    // Настраиваем target (куда шлем): копируем из sender'а входящего пакета
    memcpy(Tx_Frame + ARP_TX_TARGET_MAC_IDX, Rx_Frame + ARP_RX_SENDER_MAC_IDX, 6); // target_mac = sender_mac
    memcpy(Tx_Frame + ARP_TX_TARGET_IP_IDX,  Rx_Frame + ARP_RX_SENDER_IP_IDX,  4); // target_ip = sender_ip
    
    // Настраиваем sender (себя): пишем наш железный MAC и статический IP уставки
    memcpy(Tx_Frame + ARP_RX_SENDER_MAC_IDX, G_CONST::MAC_Controller, 6);  // sender_mac
    memcpy(Tx_Frame + ARP_RX_SENDER_IP_IDX,  My_IP, 4);                    // sender_ip
    
    // 4. Чистим хвост пакета (Padding) до 60 байт
    memset(Tx_Frame + ARP_MSG_LEN, 0, ETH_MIN_FRAME_LEN - ARP_MSG_LEN); // 60 - 42 = 18 байт
    
    // 5. Отправляем
    rEnet_drv.sendFrame(Tx_Frame, ARP_MSG_LEN);
  }
}

void CUDP_Server::handleIP() { 
  
  SIPHeader* rx_ip_header = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  
  // Проверяем, что пакет пришёл именно на наш IP
  if (memcmp(rx_ip_header->dest_ip, My_IP, 4) != 0) { return; }
  
  
  switch (rx_ip_header->protocol) {
  case IP_PROTO_ICMP:
    handleICMP(); // Обработка протокол ICMP (Ping)  
    break;
  case IP_PROTO_UDP:
    handleUDP();  // Обработка протокол UDP (ModBus) 
    break;
  }
  
}

void CUDP_Server::handleICMP() {
  
  SIPHeader* rx_ip_header = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  SEthernetHeader* rx_eth_header = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  SICMPHeader* rx_icmp_header = reinterpret_cast<SICMPHeader*>(Rx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  if (rx_icmp_header->type != ICMP_TYPE_ECHO_REQ)  return; // Echo Request (Запрос пинга)
  
  // Вычисляем полную длину входящего ICMP пакета по IP заголовку
  unsigned short full_frame_len = sizeof(SEthernetHeader) + __REV16(rx_ip_header->total_length);
  
  // Полностью изолируем данные: копируем весь кадр "запрос пинга" из Rx в Tx
  memcpy(Tx_Frame, Rx_Frame, full_frame_len);
  
  // Инициализируем передающие указатели СТРОГО на Tx_Frame
  SEthernetHeader* tx_eth_header = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
  SIPHeader* tx_ip_header        = reinterpret_cast<SIPHeader*>(Tx_Frame + sizeof(SEthernetHeader));
  SICMPHeader* tx_icmp_header    = reinterpret_cast<SICMPHeader*>(Tx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  // --- 1. Рокировка Ethernet в Tx_Frame ---
  memcpy(tx_eth_header->dest_mac, rx_eth_header->src_mac, 6);
  memcpy(tx_eth_header->src_mac, G_CONST::MAC_Controller, 6);
  
  // --- 2. Рокировка IP в Tx_Frame ---
  memcpy(tx_ip_header->dest_ip, rx_ip_header->src_ip, 4);
  memcpy(tx_ip_header->src_ip, My_IP, 4);
  
  // --- 3. Модифицируем ICMP в Tx_Frame ---
  tx_icmp_header->type = 0; // Echo Reply (Ответ)
  
  // Пересчет контрольной суммы (выполняется в Tx_Frame)
  unsigned int icmp_chk = __REV16(tx_icmp_header->checksum);
  icmp_chk += ICMP_CHKSUM_DELTA;
  if (icmp_chk > 0xFFFF) icmp_chk += 1;
  tx_icmp_header->checksum = __REV16(static_cast<unsigned short>(icmp_chk));
  
  rEnet_drv.sendFrame(Tx_Frame, full_frame_len); // Отправляем собранный Tx_Frame с указанием реальной длины
}

void CUDP_Server::handleUDP() {
  
  if (pMB_Slave == nullptr) return;
  
  SIPHeader* rx_ip_header = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  SEthernetHeader* rx_eth_header = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  // Указатель UDP-заголовка в текущем кадре (Rx_Frame)
  SUDPHeader* rx_udp_header = reinterpret_cast<SUDPHeader*>(Rx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  pMB_Slave->handleRequest(rx_eth_header, rx_ip_header, rx_udp_header); // handleRequest сам разберется с Unit ID, функциями и заполнением TX буфера
  
}
