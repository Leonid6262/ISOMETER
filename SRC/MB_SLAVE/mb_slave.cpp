#include "mb_slave.hpp" 

CMBSLAVE::CMBSLAVE(CMBUartDriver& rUartDrv, CModbusDataProxy& rModbusData, unsigned char* AddressSlave) : 
  rUartDrv(rUartDrv), rModbusData(rModbusData), pAddressSlave(pAddressSlave) {}

CMBSLAVE::StatusTO CMBSLAVE::TimeoutStatus() { 
  if (rUartDrv.rx_idx == 0) { 
    return StatusTO::NO_RECEPTION;
  }
  unsigned int delta = LPC_TIM0->TC - rUartDrv.last_byte_time;
  if (delta > MODBUS_SILENCE_TICKS) {
    return StatusTO::EXPIRED;
  } 
  return StatusTO::NOT_EXPIRED;
}

void CMBSLAVE::ParseRequestF346() {
  
  if(rx_mbs_buffer[bAddressSlave] != *pAddressSlave) return;
  if (rUartDrv.rx_idx != LRequestF346) return;
  unsigned short crc_calc = CCRC16::calc(rx_mbs_buffer, LRequestF346 - 2);
  unsigned short crc_received = (rx_mbs_buffer[bCRC16 + 1] << 8) | rx_mbs_buffer[bCRC16];
  if(crc_calc != crc_received) return;
                                        
  Function = rx_mbs_buffer[bFunction];
  
  switch (Function) {
  case F03:
  case F04: 
    StartingAddress = (rx_mbs_buffer[bStartAddress] << 8) | rx_mbs_buffer[bStartAddress + 1];
    Quantity = (rx_mbs_buffer[bQuantity] << 8) | rx_mbs_buffer[bQuantity + 1];
    if ((StartingAddress + Quantity) > rModbusData.QFields) return;
    Answer(Function);
    break;
  case F06:
    AddressReg = (rx_mbs_buffer[bAddressReg] << 8) | rx_mbs_buffer[bAddressReg + 1];
    if(AddressReg > (rModbusData.QFields - 1)) return;
    Value = (rx_mbs_buffer[bQuantity] << 8) | rx_mbs_buffer[bQuantity + 1];
    Answer(F06);
    break;
  default:
    break;
  } 
}

void CMBSLAVE::Answer(unsigned char Function) {
  
  switch (Function) {
  case F03:
  case F04:
    {
      unsigned short reg_idx = StartingAddress;
      unsigned short tx_idx = 0;
      
      tx_mbs_buffer[tx_idx++] = *pAddressSlave;
      tx_mbs_buffer[tx_idx++] = Function;
      tx_mbs_buffer[tx_idx++] = Quantity * 2;
      
      for(int i = 0; i < Quantity; i++) {
        unsigned short val = rModbusData.Modbas_fields[reg_idx++];
        tx_mbs_buffer[tx_idx++] = static_cast<unsigned char>(val >> 8);   // Hi
        tx_mbs_buffer[tx_idx++] = static_cast<unsigned char>(val & 0xFF); // Lo
      }
      
      unsigned short crc_calc = CCRC16::calc(tx_mbs_buffer, tx_idx);
      
      tx_mbs_buffer[tx_idx++] = static_cast<unsigned char>(crc_calc & 0xFF); // CRC Lo
      tx_mbs_buffer[tx_idx++] = static_cast<unsigned char>(crc_calc >> 8);   // CRC Hi
      
      rUartDrv.transfer_data(tx_idx);
    }
    break;
  case F06:
    
    break;
  default:
    break;
  } 
  
}

void CMBSLAVE::monitor() {  
  if (TimeoutStatus() == StatusTO::EXPIRED) {    
    ParseRequestF346(); 
    rUartDrv.rx_idx = 0; 
  } 
}



