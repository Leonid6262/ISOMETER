#include "loop_eth.hpp"

/* ----Тестовое ПО. К рабочим решениям, отношения не имеет!---- */

// MAC адрес ПК
const unsigned char CLOOP_ETH::MAC_PC[] = {0x48, 0xF3, 0x17, 0x12, 0x78, 0xDB};

CLOOP_ETH::CLOOP_ETH(CENET_DRV& rEnet_drv) : rEnet_drv(rEnet_drv) { init(); }

void CLOOP_ETH::init() {                                                // Тестовый кадр:
  short L_MAC = sizeof(CLOOP_ETH::MAC_PC);
  for(short n = 0; n < L_MAC; n++) {
    sendFrame[n] = MAC_PC[n];                                           // MAC получателя (PC)                             
  }                                                                     
  for(short n = 0; n < L_MAC; n++) {
    sendFrame[n + L_MAC] = CEMAC::MAC_Controller[n];                   // MAC отправителя (Controller)
  }                                                                     
  sendFrame[L_MAC * 2] = 0x08; sendFrame[1 + (L_MAC * 2)] = 0x00;       // Тип кадра 0x0800 - IPv4
  for(unsigned char n = 0; n < (CEMAC::ETH_FRAG_SIZE - 4); n++) {
    sendFrame[n + 2 + (L_MAC * 2)] = 0;                                 // 46 байт данных
  }    
}                                                                               

void CLOOP_ETH::test() {
  
  static unsigned int prev_TC0;  
  
  unsigned int dTrs = LPC_TIM0->TC - prev_TC0; //Текущая дельта [0.1*mks]
  if(dTrs < 5000000) return;
  prev_TC0 = LPC_TIM0->TC;

  if(rEnet_drv.receiveFrame(rxBuffer) == CENET_DRV::ReceiveStatus::FRAME_RECIVED)
  {
    sendFrame[0 + 2 + (2 * sizeof(CLOOP_ETH::MAC_PC))] = 'T';
    sendFrame[1 + 2 + (2 * sizeof(CLOOP_ETH::MAC_PC))] = 'E';
    sendFrame[2 + 2 + (2 * sizeof(CLOOP_ETH::MAC_PC))] = 'S';
    sendFrame[3 + 2 + (2 * sizeof(CLOOP_ETH::MAC_PC))] = 'T';
    sendFrame[4 + 2 + (2 * sizeof(CLOOP_ETH::MAC_PC))] = 0;
    rEnet_drv.sendFrame(sendFrame);
  }  

}
