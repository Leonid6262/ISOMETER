#pragma once

#include "settings_eep.hpp"

class CADC_STORAGE {
 public:
  // Список имён каналов внешнего АЦП
  enum EADC_NameCh {
    Ud = 0,
    ILeak1 = 1,
    ILeak2 = 2,
    ch_HRf = 15  // Промежуточный канал (0.5 Ref)
  };

  // --- Обработка и запись данных внешнего АЦП ---
  inline void setExternal(unsigned char channel, signed short raw_adc_data) {
    external_data[channel] = (raw_adc_data - rSettings.getSettings().shift_adc[channel]) *
                             (1.0f + rSettings.getSettings().incline_adc[channel]);
  }

  // --- Чтение данных внешнего АЦП ---
  inline signed short getExternal(unsigned char channel) { return external_data[channel]; }

  // --- Чтение external_data[] указателей ---
  inline signed short* getEPointer(unsigned char channel) { return &external_data[channel]; }


  // Для отладочного указателя на external_data[]
  inline signed short (&getExternal()) [G_CONST::NUMBER_CHANNELS] { return external_data; }

  static CADC_STORAGE& getInstance() {
    static CADC_STORAGE instance;
    return instance;
  }

 private:
  signed short external_data[G_CONST::NUMBER_CHANNELS] = {};  // Обработанные данные полученные от внешнего АЦП

  CEEPSettings& rSettings;

  // --- Механизмы Singleton ---
  CADC_STORAGE();
  CADC_STORAGE(const CADC_STORAGE&) = delete;
  CADC_STORAGE& operator=(const CADC_STORAGE&) = delete;
};