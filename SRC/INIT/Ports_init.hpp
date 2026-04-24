#pragma once
#include "LPC407x_8x_177x_8x.h"

class CSET_PORTS {
public:
  static void initDOutputs() {
    // Дискретные выходы    
    LPC_GPIO0->SET = 0x00000200; 
    LPC_GPIO0->CLR = 0xFFFFFDFF; 
    LPC_GPIO0->DIR = 0x00000200; // U-LED
    
    LPC_GPIO1->CLR = 0xFFFFFFFF;  
    LPC_GPIO1->DIR = 0x00000000; 
    
    LPC_GPIO2->CLR = 0xFFFFFFFF;
    LPC_GPIO2->DIR = 0xFF000000; // L-Ready, L-Al1, L-Al2, Rel-Ready, Rel-Al1, Rel-Al2, On-N, On-P
    
    LPC_GPIO3->CLR = 0xFFFFFFFF;
    LPC_GPIO3->DIR = 0x00000000; 
    
    LPC_GPIO4->CLR = 0xFFFFFFFF;
    LPC_GPIO4->DIR = 0x00000000; 
    
    LPC_GPIO5->CLR = 0xFFFFFFFF;
    LPC_GPIO5->DIR = 0x00000000; 
    
  }
};

