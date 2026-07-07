#pragma once

#include "bool_name.hpp"
#include "crc16.hpp"
#include "lpc_eeprom.h"
#include <stddef.h>

// Пространство имён глобальных констант
namespace G_CONST {
  constexpr unsigned short N_CHANNELS = 16;   // Количество каналов внешнего АЦП
  constexpr unsigned short Nlang = 3;         // Количество языков
  constexpr unsigned char disp_l = 16;        // Знакомест ПТ
  constexpr unsigned char MAC_Controller[] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55};
}

class CEEPSettings {
  
 private:
  // --- Структура уставок ---
  struct WorkSettings {
    unsigned short checkSum;                            // 0 Контрольная сумма
    unsigned short Language;                            // 1 Номер языка
    float k_ch1;                                        // 2 Коэффициент ch1
    float k_ch2;                                        // 3 Коэффициент ch2
    float k_Ud;                                         // 4 Коэффициент ch Ud
    signed short shift_Ud;                              // 5 Коэффициент ch Ud
    unsigned short RAlarm1;                             // 6 RAlarm1
    unsigned short RAlarm2;                             // 7 RAlarm2
    float RTadd;                                        // 8 RLadd
    bool comp_dUd;                                      // 9 Компенсация dUd On/Off
    unsigned short Rmax_20mA;                           // 10 Max R для индикации по 4...20mA
    bool Ramp_20mA;                                     // 11 Наклон. false - 0-0, max-max. true - 0-max, max-0
    unsigned char  Address;                             // 11 Адрес Slave
    unsigned short Baud_rate;                           // 12 Скорость
    unsigned short ip3;                                 // 13 Третья  группа
    unsigned short ip2;                                 // 14 Вторая  группа
    unsigned short ip1;                                 // 15 Первая  группа
    unsigned short ip0;                                 // 16 Нулевая группа
    // Добавляя новые уставки сюда, не забывайте обновлять defaultSettings ниже!!!
  };
  //  Статические константные уставки по умолчанию (во Flash) ---
  static const inline WorkSettings defaultSettings {
    .checkSum = 0x0000,
    .Language = 1,
    .k_ch1 =   0.7,
    .k_ch2 =   4.8,
    .k_Ud  =   0.055,
    .shift_Ud = 0,
    .RAlarm1 = 40,
    .RAlarm2 = 20,
    .RTadd = 100,
    .comp_dUd = true,
    .Rmax_20mA = 500,
    .Ramp_20mA = false,
    .Address = 3,
    .Baud_rate = 5,
    .ip3 = 192,
    .ip2 = 168,
    .ip1 = 1,
    .ip0 = 0   
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
  State save_status = State::OFF;
  StatusRet err_load;

  // Прочитать/Изменить уставку ---
  inline WorkSettings& getSettings() { return settings; }
};
