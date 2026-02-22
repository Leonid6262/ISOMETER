#pragma once

#include "settings_eep.hpp"
#include "spi_init.hpp"
#include <array>
#include <initializer_list>

class CADC{ 
  
private:
  
  LPC_SSP_TypeDef* SSP;
  CEEPSettings& rSet;
  /* 
  cN_CH - данные для запуска конвертации (формат - см. док.) 
  000m.0ccc.c000.0000 m - Manual mode, cccc - N channel (0...15)
  */
  static constexpr unsigned short cN_CH[G_CONST::NUMBER_CHANNELS] = { 0x1000, 0x1080, 0x1100, 0x1180 };
  
  signed short data[G_CONST::NUMBER_CHANNELS] = {};  // Обработанные данные полученные АЦП
  // --- Обработка и запись данных внешнего АЦП ---
  inline void setData(unsigned char channel, signed short raw_adc_data) {
    data[channel] = (raw_adc_data - rSet.getSettings().shift_adc[channel]);
  }
  
public:
  enum class EADC_NameCh {
    Ud     = 0,
    ILeak1 = 1,
    ILeak2 = 2,
    ch_HRf = 3   // Промежуточный канал (0.5 Ref)
  };
  
  CADC(LPC_SSP_TypeDef*, CEEPSettings&);
  
  // Метод конвертации
  void conv_tnf(std::initializer_list<char>);
   // --- Чтение external_data[] указателей ---
  inline signed short* getEPointer(unsigned char channel) { return &data[channel]; }
  inline signed short  getData(unsigned char channel)     { return data[channel]; }
  
};
