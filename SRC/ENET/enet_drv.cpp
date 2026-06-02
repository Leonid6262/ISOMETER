#include "enet_drv.hpp"
#include <stdio.h>

CENET_DRV::CENET_DRV() {}  

// ---Передача кадра---
void CENET_DRV::sendFrame(const unsigned char* data) {
  short dl = CEMAC::ETH_FRAG_SIZE - 4;                          // Не обязательно все 60 байт. Можно только кол. данных.
                                                                // При вкл. PAD, EMAC сам добавит нули до конца кадра.
  unsigned int idx = LPC_EMAC->TxProduceIndex;                  // Получаем текущий индекс для передачи
  memcpy(reinterpret_cast<void*>(CEMAC::txBuf(idx)), data, dl); // Копируем данные в соответствующий передающий буфер

  CEMAC::txDescPacket(idx) = CEMAC::txBuf(idx);                 // Устанавливаем адрес буфера и длину кадра в дескриптор
  CEMAC::txDescCtrl(idx) &= ~CEMAC::DESC_RX_SIZE;
  CEMAC::txDescCtrl(idx) |= (dl - 1); 
  idx = (idx + 1) % CEMAC::NUM_TX_FRAG;
  LPC_EMAC->TxProduceIndex = idx;                               // Запуск передачи
}

// ---Приём кадра---
CENET_DRV::ReceiveStatus CENET_DRV::receiveFrame(unsigned char* buffer) {
  idx_Cons = LPC_EMAC->RxConsumeIndex;
  idx_Prod = LPC_EMAC->RxProduceIndex;

  if (idx_Cons == idx_Prod) {
    return ReceiveStatus::EMPTY;  // Нет непринятого кадра
  }

  LPC_EMAC->RxConsumeIndex = (idx_Cons + 1) % CEMAC::NUM_RX_FRAG;  // Продвинуть ConsumeIndex
  status_rx = CEMAC::rxStatInfo(idx_Cons);

  if (status_rx & ~RX_CTRL_ERR_BITS) {
    return ReceiveStatus::ERROR;  // Кадр принят с ошибками
  }
  
  // Кадр принят, копируем весь кадр
  memcpy(buffer, reinterpret_cast<void*>(CEMAC::rxBuf(idx_Cons)), CEMAC::ETH_FRAG_SIZE);
  return ReceiveStatus::FRAME_RECIVED;  
}


