#include "mb_udp_slave.hpp" 

CMB_UDP_Slave::CMB_UDP_Slave( CModbusDataProxy& rModbusData, CUDP_Server& pUDP_Server, unsigned char* pAddressSlave ) : 
  rModbusData(rModbusData), pUDP_Server(pUDP_Server), pAddressSlave(pAddressSlave) {} 

// ============================================================================
// Обработчик запроса Modbus UDP
// ============================================================================
void CMB_UDP_Slave::handleRequest(SEthernetHeader* rx_eth_header, SIPHeader* rx_ip_header, SUDPHeader* rx_udp_header) {
  // Получаем физический адрес начала принятого кадра (Rx_Frame)
  unsigned char* rx_frame_ptr = reinterpret_cast<unsigned char*>(rx_eth_header);
  
  // Для отправки используем Tx_Frame
  unsigned char* tx_frame_ptr = pUDP_Server.getTxBuffer(); 
  
  // 1. Проверяем, что порт назначения 502
  if (__REV16(rx_udp_header->dst_port) != 502) return;
  
  // 2. Вытаскиваем Modbus-заголовок (MBAP) строго по подтвержденным индексам из rx кадра
  unsigned char* rx_mbap = rx_frame_ptr + 42; 
  unsigned char  rx_unit_id  = rx_mbap[6]; // 48-й байт (Unit ID)
  
  // 3. Вытаскиваем Modbus PDU строго с 49-го байта
  unsigned char* rx_pdu = rx_frame_ptr + 49; 
  unsigned char  rx_function   = rx_pdu[0]; // 49-й байт
  unsigned short rx_start_addr = (rx_pdu[1] << 8) | rx_pdu[2]; // 50-51 байты
  unsigned short rx_value_or_q = (rx_pdu[3] << 8) | rx_pdu[4]; // 52-53 байты (Кол-во или Значение)
  
  CModbusDataProxy& mdp = CModbusDataProxy::getInstance();
  unsigned char slave_addr = *pAddressSlave; 
  
  if (rx_unit_id != slave_addr) return;
  
  unsigned short pdu_byte_len = 0; // Реальная длина PDU данных (без байта функции)
  bool send_reply = false;
  
  // Настраиваем указатель на PDU ответа (начиная с функции, 49-й байт tx буфера)
  unsigned char* tx_pdu = tx_frame_ptr + 49;
  
  // ============================================================================
  // ДИСПЕТЧЕРИЗАЦИЯ ФУНКЦИЙ MODBUS
  // ============================================================================
  switch (rx_function) {
  case 0x03:
  case 0x04: {
    // Проверка границ массива регистров
    if ((rx_start_addr + rx_value_or_q) <= mdp.QFields) {
      tx_pdu[0] = rx_function;         // Ответная функция
      tx_pdu[1] = rx_value_or_q * 2;     // Количество байт данных
      
      unsigned short byte_idx = 2;
      // Копируем запрашиваемые регистры из прокси-модуля
      for (int i = 0; i < rx_value_or_q; i++) {
        unsigned short val = mdp.registers[rx_start_addr + i].value;
        tx_pdu[byte_idx++] = static_cast<unsigned char>(val >> 8);   // Hi
        tx_pdu[byte_idx++] = static_cast<unsigned char>(val & 0xFF); // Lo
      }
      pdu_byte_len = byte_idx; 
      send_reply = true;
    }
    break;
  }
  
  case 0x06: {
    if (rx_start_addr < mdp.QFields) {
      // Проверяем права на запись, как в RTU!
      if (mdp.registers[rx_start_addr].isWritable) {
        mdp.registers[rx_start_addr].value = rx_value_or_q;
        mdp.isDirty = true; // Выставляем флаг для записи в EEPROM!
        
        // Ответ — точное эхо запроса
        tx_pdu[0] = rx_function;
        tx_pdu[1] = static_cast<unsigned char>(rx_start_addr >> 8);
        tx_pdu[2] = static_cast<unsigned char>(rx_start_addr & 0xFF);
        tx_pdu[3] = static_cast<unsigned char>(rx_value_or_q >> 8);
        tx_pdu[4] = static_cast<unsigned char>(rx_value_or_q & 0xFF);
        
        pdu_byte_len = 5; 
        send_reply = true;
      } 
    }
    break;
  }
  
  default:
    // Сюда можно будет встроить генерацию Modbus Exception (0x80 + функция)
    break;
  }
  
  // ============================================================================
  // СЕТЕВАЯ ОТПРАВКА СФОРМИРОВАННОГО ОТВЕТА
  // ============================================================================
  if (send_reply) {
    // Настраиваем базовые сетевые структуры на Tx_Frame
    SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(tx_frame_ptr);
    SIPHeader* tx_ip        = reinterpret_cast<SIPHeader*>(tx_frame_ptr + 14);
    SUDPHeader* tx_udp      = reinterpret_cast<SUDPHeader*>(tx_frame_ptr + 14 + 20);
    
    // Заполняем MBAP ответа строго по байтам
    unsigned char* tx_mbap = tx_frame_ptr + 42;
    tx_mbap[0] = rx_mbap[0]; // Transaction ID Hi
    tx_mbap[1] = rx_mbap[1]; // Transaction ID Lo
    tx_mbap[2] = 0;          // Protocol ID Hi
    tx_mbap[3] = 0;          // Protocol ID Lo
    
    // Записываем длину в MBAP (UnitID 1 байт + байт функции 1 байт + pdu_byte_len)
    unsigned short mbap_len = 1 + pdu_byte_len;
    tx_mbap[4] = static_cast<unsigned char>(mbap_len >> 8);
    tx_mbap[5] = static_cast<unsigned char>(mbap_len & 0xFF);
    tx_mbap[6] = rx_unit_id; 
    
    // --- СЕТЕВОЙ УРОВЕНЬ (L4) ---
    unsigned short udp_len = 8 + 7 + pdu_byte_len; // 8б UDP + 7б MBAP + PDU чистых данных
    tx_udp->length   = __REV16(udp_len);
    tx_udp->src_port = rx_udp_header->dst_port; // 502
    tx_udp->dst_port = rx_udp_header->src_port; // Порт панели
    tx_udp->checksum = 0;
    
    // --- СТРУКТУРА IP (L3) ---
    tx_ip->total_length = __REV16(20 + udp_len);
    memcpy(tx_ip->src_ip, pUDP_Server.getMyIPPtr(), 4);
    memcpy(tx_ip->dest_ip, rx_ip_header->src_ip, 4);
    tx_ip->checksum = 0;
    
    // Жестко фиксируем константы IP заголовка в буфере отправки
    tx_frame_ptr[14] = 0x45; 
    tx_frame_ptr[15] = 0x00; 
    tx_frame_ptr[23] = 17; // Протокол UDP
    
    // --- ПРОГРАММНЫЙ РАСЧЕТ CHECKSUM ДЛЯ IP ---
    unsigned int ip_sum = 0;
    unsigned short* ip_ptr = reinterpret_cast<unsigned short*>(tx_frame_ptr + 14);
    
    for (int i = 0; i < 10; i++) {
      ip_sum += __REV16(ip_ptr[i]); 
    }
    while (ip_sum >> 16) {
      ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
    }
    tx_ip->checksum = __REV16(static_cast<unsigned short>(~ip_sum));
    
    // --- КАНАЛЬНЫЙ УРОВЕНЬ (L2) ---
    memcpy(tx_eth->dest_mac, rx_eth_header->src_mac, 6);
    memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6);
    tx_eth->type = __REV16(0x0800); // IPv4
    
    // Отправляем готовый изолированный пакет наружу
    unsigned short total_packet_len = 14 + 20 + udp_len;
    pUDP_Server.rEnet_drv.sendFrame(tx_frame_ptr, total_packet_len);
  }
}

/*
void CMB_UDP_Slave::handleRequest(SEthernetHeader* rx_eth_header, SIPHeader* rx_ip_header, SUDPHeader* rx_udp_header) {
// Получаем физический адрес начала принятого кадра (Rx_Frame)
unsigned char* rx_frame_ptr = reinterpret_cast<unsigned char*>(rx_eth_header);
// Для отправки используем Tx_Frame
unsigned char* tx_frame_ptr = pUDP_Server.getTxBuffer(); 

// 1. Проверяем, что порт назначения 502 (берём из проверенного серверного указателя)
if (__REV16(rx_udp_header->dst_port) != 502) return;

// 2. Вытаскиваем Modbus-заголовок (MBAP) строго по подтвержденным индексам из rx кадра
unsigned char* rx_mbap = rx_frame_ptr + 42; 
unsigned char  rx_unit_id  = rx_mbap[6]; // 48-й байт (Unit ID)

// 3. Вытаскиваем Modbus PDU строго с 49-го байта
unsigned char* rx_pdu = rx_frame_ptr + 49; 
unsigned char  rx_function   = rx_pdu[0]; // 49-й байт
unsigned short rx_start_addr = (rx_pdu[1] << 8) | rx_pdu[2]; // 50-51 байты
unsigned short rx_quantity   = (rx_pdu[3] << 8) | rx_pdu[4]; // 52-53 байты

CModbusDataProxy& mdp = CModbusDataProxy::getInstance();

unsigned char slave_addr = *pAddressSlave; // Адрес 

if (rx_unit_id == slave_addr) {

// Если панель просит 3x (0x04) или 4x (0x03)
if (rx_function == 0x03 || rx_function == 0x04) {

// Проверка границ массива регистров
if ((rx_start_addr + rx_quantity) <= mdp.QFields) {

// Настраиваем указатели ответа в чистом буфере отправки
SEthernetHeader* tx_eth = reinterpret_cast<SEthernetHeader*>(tx_frame_ptr);
SIPHeader* tx_ip        = reinterpret_cast<SIPHeader*>(tx_frame_ptr + 14);
SUDPHeader* tx_udp      = reinterpret_cast<SUDPHeader*>(tx_frame_ptr + 14 + 20);

// Заполняем MBAP ответа строго по байтам
unsigned char* tx_mbap = tx_frame_ptr + 42;
tx_mbap[0] = rx_mbap[0]; // Возвращаем Transaction ID Hi
tx_mbap[1] = rx_mbap[1]; // Возвращаем Transaction ID Lo
tx_mbap[2] = 0;          // Protocol ID Hi
tx_mbap[3] = 0;          // Protocol ID Lo

// Заполняем PDU ответа (начиная с 49-го байта tx буфера)
unsigned char* tx_pdu = tx_frame_ptr + 49;
tx_pdu[0] = rx_function;         // Ответная функция
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
tx_mbap[6] = rx_unit_id; 

// --- СЕТЕВОЙ УРОВЕНЬ (L4) ---
unsigned short udp_len = 8 + 7 + byte_idx; // 8б UDP + 7б MBAP + Modbus PDU
tx_udp->length   = __REV16(udp_len);
tx_udp->src_port = rx_udp_header->dst_port; // 502
tx_udp->dst_port = rx_udp_header->src_port; // Порт панели
tx_udp->checksum = 0;

// --- СТРУКТУРА IP (L3) ---
tx_ip->total_length = __REV16(20 + udp_len);
memcpy(tx_ip->src_ip, pUDP_Server.getMyIPPtr(), 4);
memcpy(tx_ip->dest_ip, rx_ip_header->src_ip, 4);
tx_ip->checksum = 0;

// Жестко фиксируем константы IP заголовка в буфере отправки
tx_frame_ptr[14] = 0x45; 
tx_frame_ptr[15] = 0x00; 
tx_frame_ptr[23] = 17; // Протокол UDP

// --- ПРОГРАММНЫЙ РАСЧЕТ CHECKSUM ДЛЯ IP ---
unsigned int ip_sum = 0;
unsigned short* ip_ptr = reinterpret_cast<unsigned short*>(tx_frame_ptr + 14);

for (int i = 0; i < 10; i++) {
ip_sum += __REV16(ip_ptr[i]); 
        }
while (ip_sum >> 16) {
ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
        }
tx_ip->checksum = __REV16(static_cast<unsigned short>(~ip_sum));

// --- КАНАЛЬНЫЙ УРОВЕНЬ (L2) ---
memcpy(tx_eth->dest_mac, rx_eth_header->src_mac, 6);
memcpy(tx_eth->src_mac, G_CONST::MAC_Controller, 6);
tx_eth->type = __REV16(0x0800); // IPv4

// Отправляем готовый изолированный пакет наружу
unsigned short total_packet_len = 14 + 20 + udp_len;
pUDP_Server.rEnet_drv.sendFrame(tx_frame_ptr, total_packet_len);
return;
      }
    }
  }
}
*/