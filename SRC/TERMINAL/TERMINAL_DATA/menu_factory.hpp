#pragma once

#include "settings_eep.hpp"
#include "version.hpp"

namespace menu_alias {
  using vt   = EVarType;
  using o    = CMenuNavigation::MenuNode;
  using nm   = CMenuNavigation::ENodeMode;
}

// таблица переводов (масштабируемая)
static const struct {
    const char* INDICATION[G_CONST::Nlang]     = {"ИНДИКАЦИЯ",       "INDICATION",       "IНДИКАЦIЯ"   };
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETTINGS",         "УСТАНОВКИ"   };
    const char* PARAMS[G_CONST::Nlang]         = {"ПАРАМЕТРЫ",       "PARAMETERS",       "ПАРАМЕТРИ"   };
    const char* RS_485[G_CONST::Nlang]         = {"RS-485",          "RS-485",           "RS-485"      };
    const char* SETTING_UP[G_CONST::Nlang]     = {"НАСТРОЙКА",       "SETTING UP",       "НАЛАШТУВАННЯ"};
    const char* LANGUAGE[G_CONST::Nlang]       = {"ЯЗЫК",            "LANGUAGE",         "МОВА"        };  
} Mn;


//--- Фабрика дерева меню ---
/*
  Структура узла:

  { "Name", {children}, &var, un, cd, p, vt, nm, min, max }

  un - размерност
  cd - коэффициент отображения
  p  - точность
  vt - тип переменной
  nm - тип узла
*/
inline std::vector<menu_alias::o> MENU_Factory(CPROCESS& rProcess, CEEPSettings& rSet ) {
                                                   
  auto& set = rSet.getSettings();
  
  using namespace menu_alias;
  
  static constexpr unsigned char ch_UD     = static_cast<unsigned char>(CADC::EADC_NameCh::Ud);
  static constexpr unsigned char ch_ILeak1 = static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1);
  static constexpr unsigned char ch_ILeak2 = static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2);
  
  unsigned short l = set.Language - 1;                          // Установка языка отображения согласно уставке
  
  enum Precision : unsigned char { p0, p1, p2, p3, p4 };        // количество знаков после запятой p4->0.0001
 
  std::vector<o> MENU = {
  o(Mn.INDICATION[l],{
      o("Rinsul", {}, rProcess.getPointerR(), "kOhm", 1, p0, vt::vfloat, nm::In1V),}),  
  o(Mn.SETTINGS[l],{
      o(Mn.PARAMS[l],{
          o("Alarm1",  {}, &set.RAlarm1, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500  ),
          o("Alarm2",  {}, &set.RAlarm2, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500  ),
          o("RTadd",   {}, &set.RTadd,   "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 1000 ),
          o("Rmax",    {}, &set.Rmax,    "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 2000 ),
          o("Rmin",    {}, &set.Rmin,    "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 2000 ),}),
      o(Mn.SETTING_UP[l],{
          o::Dual("Ud",    rProcess.getPointerTUd(),      "d",   1, p0, vt::sshort,
                  "shift", &set.shift_adc[ch_UD],         "d",   1, p0, vt::sshort, nm::IE2V, -23, 2070   ),
          o::Dual("ILeak1",rProcess.getPointerTILeak1(),  "d",   1, p0, vt::sshort,
                  "shift", &set.shift_adc[ch_ILeak1],     "d",   1, p0, vt::sshort, nm::IE2V, -23, 2070   ),
          o::Dual("ILeak2",rProcess.getPointerTILeak2(),  "d",   1, p0, vt::sshort,
                  "shift", &set.shift_adc[ch_ILeak2],     "d",   1, p0, vt::sshort, nm::IE2V, -23, 2070   ),
          o::Dual("Rk1",   rProcess.getPointerR(),        "kOhm",1, p2, vt::vfloat,
                  "k1Ls",  &set.k1Ls,                     "",    1, p3, vt::vfloat, nm::IE2V,   0.1f, 100 ),
          o::Dual("Rk2",   rProcess.getPointerR(),        "kOhm",1, p2, vt::vfloat,
                  "k2Ls",  &set.k2Ls,                     "",    1, p3, vt::vfloat, nm::IE2V,   0.1f, 100 ),
          o("RelAl1",  {}, rProcess.getPointerSRl1(),     "",    1, p0, vt::vbool,  nm::Ed1V,   0, 1      ),
          o("RelAl2",  {}, rProcess.getPointerSRl2(),     "",    1, p0, vt::vbool,  nm::Ed1V,   0, 1      ),}),
      o(Mn.RS_485[l],{
          o("Address",   {}, &set.Address,   "", 1, p0, vt::ushort, nm::Ed1V, 1, 200  ),
          o("Baud 9600-115200", {}, &set.Baud_rate, "", 1, p0, vt::ushort, nm::Ed1V, 1, 5  ),
          o("Parity",    {}, &set.Parity,    "", 1, p0, vt::vbool,  nm::Ed1V, 0, 1 ),
          o("Stop bits", {}, &set.Stop_bits, "", 1, p0, vt::ushort, nm::Ed1V, 1, 2 ),}),      
      o(Mn.LANGUAGE[l],{
          o("Language:", {}, &set.Language,"", 1, p0, vt::ushort, nm::Ed1V, 1, G_CONST::Nlang),}),})};
  
  return MENU;
}
