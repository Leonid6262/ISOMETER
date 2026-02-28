#include "mb_slave.hpp" 

CMBSLAVE::CMBSLAVE(CMBUartDriver& rUartDrv, CDMAcontroller& rDMAc, CEEPSettings& rSet, CPROCESS& rProcess) : 
  rUartDrv(rUartDrv), rDMAc(rDMAc), rSet(rSet), rProcess(rProcess) {}

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
  
  if(rx_mbs_buffer[bAddressSlave] != rSet.getSettings().Address) return;
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
    if ((StartingAddress + Quantity) > QFields) return;
    Answer(Function);
    break;
  case F06:
    AddressReg = (rx_mbs_buffer[bAddressReg] << 8) | rx_mbs_buffer[bAddressReg + 1];
    if(AddressReg > (QFields - 1)) return;
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
    tx_mbs_buffer[0] = rSet.getSettings().Address;
    tx_mbs_buffer[1] = Function;
    tx_mbs_buffer[2] = Quantity * 2;
    
    //tx_mbs_buffer[3] = static_cast<unsigned char>(rProcess.UStatus.all >> 8);
    //tx_mbs_buffer[4] = static_cast<unsigned char>(rProcess.UStatus.all & 0xFF);
    //unsigned short R = static_cast<unsigned short>(rProcess.R + 0.5f);
    //tx_mbs_buffer[5] = static_cast<unsigned char>(R >> 8);
    //tx_mbs_buffer[6] = static_cast<unsigned char>(R & 0xFF);    
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



