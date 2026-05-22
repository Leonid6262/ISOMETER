#pragma once

#include "spi_init.hpp"
#include "system_LPC177x.h"

class CSET_SPI {
public:
  static LPC_SSP_TypeDef* config(LPC_SSP_TypeDef* SSP, unsigned int Hz_SPI, unsigned short bits) {
    SSP->CR0 = 0;
    SSP->CR0 = bits - 1;  // (16 - 1) -> 16 bits
    SSP->CR1 = 0;
    SPI_Config::set_spi_clock(SSP, Hz_SPI, PeripheralClock);
    SSP->CR1 |= SPI_Config::CR1_SSP_EN;
    return SSP;
  }
};
