#pragma once

#include "bool_name.hpp"
#include "crc16.hpp"
#include "lpc_eeprom.h"
#include <stddef.h>

// Пространство имён глобальных констант
namespace G_CONST {
  constexpr unsigned short NUMBER_CHANNELS = 4;    // Количество каналов внешнего АЦП
  constexpr unsigned short Nlang = 3;              // Количество языков
}


class CEEPSettings {
  
 private:
  // --- Структура уставок ---
  struct WorkSettings {
    unsigned short checkSum;                            // 0 Контрольная сумма
    unsigned short Language;                            // 4 Номер языка
    signed short shift_adc[G_CONST::NUMBER_CHANNELS];   // 5 Смещения АЦП
    float incline_adc[G_CONST::NUMBER_CHANNELS];        // 6 Наклон
    unsigned short Rmin1;                               // 7 Rmin1
    unsigned short Rmin2;                               // 8 Rmin2
    // Добавляя новые уставки сюда, не забывайте обновлять defaultSettings ниже!!!
  };
  //  Статические константные уставки по умолчанию (во Flash) ---
  static const inline WorkSettings defaultSettings {
    .checkSum = 0x0000,
    .Language = 1,
    .shift_adc =   {   0, 2047, 2047,    0},
    .incline_adc = {0.0f, 0.0f, 0.0f, 0.0f},
    .Rmin1 = 10,
    .Rmin2 = 40
  };
    
  // Текущий набор уставок, хранящийся в RAM ---
  WorkSettings settings;

  // Механизмы Singleton ---
  CEEPSettings();                                         // Приватный конструктор
  CEEPSettings(const CEEPSettings&) = delete;             // Запрещаем копирование
  CEEPSettings& operator=(const CEEPSettings&) = delete;  // Запрещаем присваивание

  // Приватные методы для работы с EEP  ---
  StatusRet readFromEEPInternal(WorkSettings& outSettings);  // Чтение в WorkSettings
  void writeToEEPInternal(WorkSettings& inSettings);         // Запись из WorkSettings

  void EEP_init(void);
  void EEPr(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);
  void EEPw(uint16_t page_offset, uint16_t page_address, void* data, EEPROM_Mode_Type mode, uint32_t count);
  
 public:

  static CEEPSettings& getInstance();

  // Загрузки/сохранения уставок ---
  StatusRet loadSettings();  // Загружает из EEP. Если ошибка CRC - остаются дефолтные.
  void saveSettings();       // Сохраняет текущие уставки в EEP.

  // Прочитать/Изменить уставку ---
  inline WorkSettings& getSettings() { return settings; }
};
