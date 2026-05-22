#pragma once
#include "Peripherals.hpp"
#include "LPC407x_8x_177x_8x.h"

class CRTC
{ 

public:
  
  CRTC();
  
  struct SDateTime {
    unsigned short  year;
    unsigned short  month;
    unsigned short  day;
    unsigned short  hour;
    unsigned short  minute;
    unsigned short  second;
  };
   
  SDateTime DateTimeForSet;
  
  void update_now();                    // Обновление данных из RTC
  void update_for_set();                // Обновление данных из RTC для установки
  const SDateTime& get_now() const;     // Доступ к актуальным данным  
  void setDateTime(const SDateTime&);   // Установка даты и времени
  void setDefault();                    // Установка дефолтных значений
  
  bool set_date_time = false;
  
private: 
  
  bool isDateTimeValid();               // Проверка корректности данных при включеии питания
  
  SDateTime date_now;                   // Кэш актуального времени  
 
  
};

