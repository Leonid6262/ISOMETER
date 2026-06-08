#include "udp_server.hpp"
#include "settings_eep.hpp"
#include <stdio.h>

CUDP_Server::CUDP_Server(CENET_DRV& m_eth, CEEPSettings& rSet) : m_eth(m_eth), rSet(rSet) { updateCurrentIP(); }

void CUDP_Server::poll() {
  // 1. Принимаем пакет строго в изолированный приемный буфер Rx_Frame
  if (m_eth.receiveFrame(Rx_Frame) != CENET_DRV::ReceiveStatus::FRAME_RECIVED) {
    return; 
  }
  
  updateCurrentIP();
  
  SEthernetHeader* eth = reinterpret_cast<SEthernetHeader*>(Rx_Frame);
  unsigned short eth_type = __REV16(eth->type);
  
  if (eth_type == 0x0806) {
    handleARP(eth);
  } 
  else if (eth_type == 0x0800) {
    handleIP(eth);
  }
}

void CUDP_Server::handleARP(SEthernetHeader* rx_eth) {
  // Жёсткие физические смещения в обход выравнивания структур IAR
  // В ARP-пакете Opcode запроса/ответа лежит в байтах 20 и 21 кадра Rx_Frame
  unsigned short opcode = (Rx_Frame[20] << 8) | Rx_Frame[21];
  
  // Target IP (тот, кого ищут в сети) всегда лежит в байтах 38, 39, 40, 41 кадра Rx_Frame
  if (opcode == 1 && 
      Rx_Frame[38] == m_my_ip[0] && 
      Rx_Frame[39] == m_my_ip[1] && 
      Rx_Frame[40] == m_my_ip[2] && 
      Rx_Frame[41] == m_my_ip[3]) 
  {
    // 1. Копируем заголовок (Eth + ARP) во внутренний Tx_Frame для сборки ответа
    memcpy(Tx_Frame, Rx_Frame, 42);
    
    SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
    
    // --- 2. Рокировка Ethernet-заголовка ---
    memcpy(tx_eth->dest_mac, rx_eth->src_mac, 6); 
    memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6); 
    
    // --- 3. Модификация ARP-данных строго по физическим индексам Tx_Frame ---
    Tx_Frame[20] = 0x00; // Opcode Hi
    Tx_Frame[21] = 0x02; // Opcode Lo -> Reply (Ответ)
    
    // Настраиваем target (куда шлем): копируем из sender'а входящего пакета
    // В Rx_Frame MAC отправителя — это [22..27], IP отправителя — [28..31]
    memcpy(Tx_Frame + 32, Rx_Frame + 22, 6); // target_mac = sender_mac
    memcpy(Tx_Frame + 38, Rx_Frame + 28, 4); // target_ip = sender_ip
    
    // Настраиваем sender (себя): пишем наш железный MAC и статический IP уставки
    memcpy(Tx_Frame + 22, G_CONST::MAC_Controller, 6); // sender_mac
    memcpy(Tx_Frame + 28, m_my_ip, 4);                // sender_ip
    
    // 4. Чистим хвост пакета (Padding) до 60 байт
    memset(Tx_Frame + 42, 0, 18);
    
    // 5. Отправляем
    m_eth.sendFrame(Tx_Frame, 42);
  }
}

void CUDP_Server::handleIP(SEthernetHeader* rx_eth) {
  SIPHeader* rx_ip = reinterpret_cast<SIPHeader*>(Rx_Frame + sizeof(SEthernetHeader));
  
  // Проверяем, что пакет пришёл именно на наш IP
  if (memcmp(rx_ip->dest_ip, m_my_ip, 4) != 0) {
    return;
  }
  
  if (rx_ip->protocol == 1) { // Протокол ICMP (Ping)
    handleICMP(rx_eth, rx_ip);
  } 
  else if (rx_ip->protocol == 17) { // Протокол UDP
    handleUDP(rx_eth, rx_ip);
  }
}

void CUDP_Server::handleICMP(SEthernetHeader* rx_eth, SIPHeader* rx_ip) {
  // Находим начало входящего ICMP-заголовка в Rx_Frame
  SICMPHeader* rx_icmp = reinterpret_cast<SICMPHeader*>(Rx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
  
  if (rx_icmp->type == 8) // Echo Request (Запрос пинга)
  {
    // Вычисляем полную длину входящего ICMP пакета по IP заголовку
    unsigned short ip_total_len = __REV16(rx_ip->total_length);
    unsigned short full_frame_len = sizeof(SEthernetHeader) + ip_total_len;
    
    // Полностью изолируем данные: копируем весь кадр "запрос пинга" из Rx в Tx
    memcpy(Tx_Frame, Rx_Frame, full_frame_len);
    
    // Инициализируем передающие указатели СТРОГО на Tx_Frame
    SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
    SIPHeader* tx_ip        = reinterpret_cast<SIPHeader*>(Tx_Frame + sizeof(SEthernetHeader));
    SICMPHeader* tx_icmp    = reinterpret_cast<SICMPHeader*>(Tx_Frame + sizeof(SEthernetHeader) + sizeof(SIPHeader));
    
    // --- 1. Рокировка Ethernet в Tx_Frame ---
    memcpy(tx_eth->dest_mac, rx_eth->src_mac, 6);
    memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6);
    
    // --- 2. Рокировка IP в Tx_Frame ---
    memcpy(tx_ip->dest_ip, rx_ip->src_ip, 4);
    memcpy(tx_ip->src_ip, m_my_ip, 4);
    
    // --- 3. Модифицируем ICMP в Tx_Frame ---
    tx_icmp->type = 0; // Echo Reply (Ответ)
    
    // Ваш рабочий оригинальный пересчет контрольной суммы (теперь выполняется в Tx_Frame)
    unsigned int icmp_chk = __REV16(tx_icmp->checksum);
    icmp_chk += 0x0800;
    if (icmp_chk > 0xFFFF) icmp_chk += 1;
    tx_icmp->checksum = __REV16(static_cast<unsigned short>(icmp_chk));
        // Отправляем собранный Tx_Frame буфер в сеть с указанием реальной длины
        m_eth.sendFrame(Tx_Frame, full_frame_len);
  }
}

void CUDP_Server::handleUDP(SEthernetHeader* rx_ethHdr, SIPHeader* rx_ipHdr) {
  
  if (m_mb_slave == nullptr) return;
    
    // 1. Вычисляем указатель на UDP-заголовок в текущем принятом кадра (Rx_Frame)
    SUDPHeader* rx_udpHdr = reinterpret_cast<SUDPHeader*>(Rx_Frame + 14 + 20);
    
    // 2. Просто передаем все три указателя в объект slave
    // Внутри handleRequest сам разберется с Unit ID, функциями и заполнением TX буфера
    m_mb_slave->handleRequest(rx_ethHdr, rx_ipHdr, rx_udpHdr);
  
  
  
/*  
  SUDPHeader* rx_udpHdr = reinterpret_cast<SUDPHeader*>(Rx_Frame + 14 + 20);
  
  // Проверяем, что порт назначения 502
  if (__REV16(rx_udpHdr->dst_port) == 502) {
    
    // Вытаскиваем Modbus-заголовок (MBAP) строго по подтвержденным индексам
    unsigned char* rx_mbap = Rx_Frame + 42; 
    

    unsigned char  rx_unit_id  = rx_mbap[6]; // 48-й байт (Unit ID = 3)
    
    // Вытаскиваем Modbus PDU строго с 49-го байта
    unsigned char* rx_pdu = Rx_Frame + 49; 
    unsigned char  rx_function   = rx_pdu[0]; // 49-й байт (Функция = 4)
    unsigned short rx_start_addr = (rx_pdu[1] << 8) | rx_pdu[2]; // 50-51 байты (Адрес = 0)
    unsigned short rx_quantity   = (rx_pdu[3] << 8) | rx_pdu[4]; // 52-53 байты (Кол-во = 1)
    
    // Проверяем Unit ID на соответствие уставке прибора
    CModbusDataProxy& mdp = CModbusDataProxy::getInstance();
    unsigned char slave_addr = CEEPSettings::getInstance().getSettings().Address;
    
    if (rx_unit_id == slave_addr) {
      
      // Если панель просит 3x (0x04) или 4x (0x03)
      if (rx_function == 0x03 || rx_function == 0x04) {
        
        // Проверка границ массива регистров
        if ((rx_start_addr + rx_quantity) <= mdp.QFields) {
          
          // Настраиваем указатели ответа в чистом буфере Tx_Frame
          SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(Tx_Frame);
          SIPHeader* tx_ip        = reinterpret_cast<SIPHeader*>(Tx_Frame + 14);
          SUDPHeader* tx_udp      = reinterpret_cast<SUDPHeader*>(Tx_Frame + 14 + 20);
          
          // Заполняем MBAP ответа строго по байтам (обходим выравнивание IAR)
          unsigned char* tx_mbap = Tx_Frame + 42;
          tx_mbap[0] = rx_mbap[0]; // Возвращаем Transaction ID Hi
          tx_mbap[1] = rx_mbap[1]; // Возвращаем Transaction ID Lo
          tx_mbap[2] = 0;          // Protocol ID Hi
          tx_mbap[3] = 0;          // Protocol ID Lo
          
          // Заполняем PDU ответа (начиная с 49-го байта Tx_Frame)
          unsigned char* tx_pdu = Tx_Frame + 49;
          tx_pdu[0] = rx_function;         // Ответная функция (0x04)
          tx_pdu[1] = rx_quantity * 2;     // Количество байт данных
          
          unsigned short byte_idx = 2;
          // Копируем запрашиваемые регистры из прокси-модуля
          for (int i = 0; i < rx_quantity; i++) {
            unsigned short val = mdp.registers[rx_start_addr + i].value;
            tx_pdu[byte_idx++] = static_cast<unsigned char>(val >> 8);   // Hi
            tx_pdu[byte_idx++] = static_cast<unsigned char>(val & 0xFF); // Lo
          }
          
          // Записываем длину в MBAP ответа (UnitID 1 байт + PDU ответа)
          unsigned short mbap_len = 1 + byte_idx;
          tx_mbap[4] = static_cast<unsigned char>(mbap_len >> 8);
          tx_mbap[5] = static_cast<unsigned char>(mbap_len & 0xFF);
          tx_mbap[6] = rx_unit_id; // Пишем честный Unit ID (3) в 48-й байт
          
          // --- СЕТЕВОЙ УРОВЕНЬ (L4) ---
          unsigned short udp_len = 8 + 7 + byte_idx; // 8б UDP + 7б MBAP + Modbus PDU
          tx_udp->length   = __REV16(udp_len);
          tx_udp->src_port = rx_udpHdr->dst_port; // 502
          tx_udp->dst_port = rx_udpHdr->src_port; // Порт панели
          
          tx_udp->checksum = 0;
          
          // --- СЕТЕВОЙ УРОВЕНЬ (L3) ---
          tx_ip->total_length = __REV16(20 + udp_len);
          memcpy(tx_ip->src_ip, m_my_ip, 4);
          memcpy(tx_ip->dest_ip, rx_ipHdr->src_ip, 4);
          tx_ip->checksum = 0;
          
          // Жестко фиксируем константы IP заголовка
          Tx_Frame[14] = 0x45; 
          Tx_Frame[15] = 0x00; 
          Tx_Frame[23] = 17; // Протокол UDP (Пинг не пострадает, это изолированный Tx_Frame)
          
          // --- ПРОГРАММНЫЙ РАСЧЕТ CHECKSUM ДЛЯ IP (СТРОГО ОБЯЗАТЕЛЕН ДЛЯ LPC) ---
          tx_ip->checksum = 0; // Сначала зануляем поле перед расчетом
          unsigned int ip_sum = 0;
          unsigned short* ip_ptr = reinterpret_cast<unsigned short*>(Tx_Frame + 14);
          
          // IP-заголовок занимает ровно 20 байт = 10 слов по 16 бит
          for (int i = 0; i < 10; i++) {
            ip_sum += __REV16(ip_ptr[i]); // Складываем в процессорном Little-Endian виде
          }
          // Складываем переносы
          while (ip_sum >> 16) {
            ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
          }
          // Инвертируем и разворачиваем обратно в сетевой Big-Endian
          tx_ip->checksum = __REV16(static_cast<unsigned short>(~ip_sum));
          
          // --- КАНАЛЬНЫЙ УРОВЕНЬ (L2) ---
          memcpy(tx_eth->dest_mac, rx_ethHdr->src_mac, 6);
          memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6);
          tx_eth->type = __REV16(0x0800); // IPv4
          
          // Отправляем готовый изолированный пакет
          unsigned short total_packet_len = 14 + 20 + udp_len;
          m_eth.sendFrame(Tx_Frame, total_packet_len);
          return;
        }
      }
    }
  }
*/
}
