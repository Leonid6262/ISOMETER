#include "rtc.hpp"

CRTC::CRTC() {
  if ((P::RTC->RTC_AUX & bRTC::RTC_OSCF) || !isDateTimeValid()) { 
    setDefault();                       // Установка дефолтных значений если была остановка
    P::RTC->RTC_AUX |= bRTC::RTC_OSCF;  // генератора или прочитанные значения не корректные
  }
  DateTimeForSet = {0,0,0,0,0,0};
}

bool CRTC::isDateTimeValid() {
  unsigned int t = P::RTC->CTIME0; 
  unsigned int d = P::RTC->CTIME1;

  unsigned char month = static_cast<unsigned char>((d >> 8) & 0x0F);  // MONTH
  unsigned char day = static_cast<unsigned char>(d & 0x1F);           // DAY
  unsigned char hour = static_cast<unsigned char>((t >> 16) & 0x1F);  // HOUR
  unsigned char min = static_cast<unsigned char>((t >> 8) & 0x3F);    // MIN
  unsigned char sec = static_cast<unsigned char>(t & 0x3F);           // SEC
  
  return (sec <= 59) && (min <= 59) && (hour <= 23) && (day >= 1 && day <= 31) && (month >= 1 && month <= 12);
}

void CRTC::update_now() {
  unsigned int t = P::RTC->CTIME0;
  unsigned int d = P::RTC->CTIME1;

  date_now.year = static_cast<unsigned char>((d >> 16) & 0x7F);   // YEAR
  date_now.month = static_cast<unsigned char>((d >> 8) & 0x0F);   // MONTH
  date_now.day = static_cast<unsigned char>(d & 0x1F);            // DAY
  date_now.hour = static_cast<unsigned char>((t >> 16) & 0x1F);   // HOUR
  date_now.minute = static_cast<unsigned char>((t >> 8) & 0x3F);  // MIN
  date_now.second = static_cast<unsigned char>(t & 0x3F);         // SEC

}

void CRTC::update_for_set() {
  unsigned int t = P::RTC->CTIME0;
  unsigned int d = P::RTC->CTIME1;

  DateTimeForSet.year = static_cast<unsigned char>((d >> 16) & 0x7F);   // YEAR
  DateTimeForSet.month = static_cast<unsigned char>((d >> 8) & 0x0F);   // MONTH
  DateTimeForSet.day = static_cast<unsigned char>(d & 0x1F);            // DAY
  DateTimeForSet.hour = static_cast<unsigned char>((t >> 16) & 0x1F);   // HOUR
  DateTimeForSet.minute = static_cast<unsigned char>((t >> 8) & 0x3F);  // MIN
  DateTimeForSet.second = 0; 

}

const CRTC::SDateTime& CRTC::get_now() const { return date_now; }

void CRTC::setDateTime(const SDateTime& dt) {
  P::RTC->CCR &= ~bRTC::CLKEN;  // Stop Clock

  P::RTC->YEAR = dt.year;
  P::RTC->MONTH = dt.month;
  P::RTC->DOM = dt.day;
  P::RTC->HOUR = dt.hour;
  P::RTC->MIN = dt.minute;
  P::RTC->SEC = dt.second;

  P::RTC->CCR |= bRTC::CLKEN;  // Start Clock
}

void CRTC::setDefault() {
  P::RTC->CCR |= bRTC::CCALDS;  // Отключение калибратора
  setDateTime({
      25,  // Default year
      6,   // Default month
      15,  // Default day
      12,  // Default hour
      0,   // Default minute
      0    // Default second
  });
}
