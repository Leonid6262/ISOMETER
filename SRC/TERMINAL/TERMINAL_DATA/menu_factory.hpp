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
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETPOINT",         "УСТАВКИ"     };
    const char* PARAMS[G_CONST::Nlang]         = {"ПАРАМЕТРЫ",       "PARAMETERS",       "ПАРАМЕТРИ"   };
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
  
  unsigned short l = set.Language - 1;                          // Установка языка отображения согласно уставке
  
  enum Precision : unsigned char { p0, p1, p2, p3, p4 };        // количество знаков после запятой p4->0.0001
 
  std::vector<o> MENU = {
  o(Mn.INDICATION[l],{
      //o("Rinsul", {}, rProcess.getPointerR(), "kOhm", 1, p0, vt::vfloat, nm::In1V),}),  
      o::Dual("Rinsul", rProcess.getPointerR(),       "kOhm",   1, p0, vt::vfloat,
              "Nch",    rProcess.getPointerNch(),        "N",   1, p0, vt::ushort, nm::In2V ),}),
  o(Mn.SETTINGS[l],{
      o(Mn.PARAMS[l],{
          o("Alarm1",  {}, &set.RAlarm1, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500  ),
          o("Alarm2",  {}, &set.RAlarm2, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500  ),
          o("RTadd",   {}, &set.RTadd,   "kOhm", 1, p1, vt::vfloat, nm::Ed1V, 0, 200000 ),}),
      o(Mn.SETTING_UP[l],{
          o::Dual("R-ch1",  rProcess.getPointerR1(),       "kOhm",   1, p2, vt::vfloat,
                  "k-ch1",  &set.k_ch1,                    "",       1, p3, vt::vfloat, nm::IE2V,   0.1f, 100 ),
          o::Dual("R-ch2",  rProcess.getPointerR2(),       "kOhm",   1, p2, vt::vfloat,
                  "k-ch2",  &set.k_ch2,                     "",      1, p3, vt::vfloat, nm::IE2V,   0.1f, 100 ),
          o::Dual("Ud",     rProcess.getPointerUd_d(),     "d",      1, p0, vt::sshort,
                  "shift",  &set.shift_Ud,                 "d",      1, p0, vt::sshort, nm::IE2V, -100, 100   ),
          o::Dual("Ud",     rProcess.getPointerUd_V(),     "V",      1, p2, vt::vfloat,
                  "k-Ud",   &set.k_Ud,                     "",       1, p4, vt::vfloat, nm::IE2V, 0, 100   ),
          o("Control RelAl1",  {}, rProcess.getPointerSRl1(),   "",   1, p0, vt::vbool, nm::Ed1V,   0, 1   ),
          o("Control RelAl2",  {}, rProcess.getPointerSRl2(),   "",   1, p0, vt::vbool, nm::Ed1V,   0, 1   ),}),
      o("RS-485",{
          o("Slave address",    {}, &set.Address,   "", 1, p0, vt::ushort, nm::Ed1V, 1, 247  ),
          o("Baud 9600-115200", {}, &set.Baud_rate, "", 1, p0, vt::ushort, nm::Ed1V, 1,   5  ),}),      
      o(Mn.LANGUAGE[l],{
          o("Language:", {}, &set.Language,"", 1, p0, vt::ushort, nm::Ed1V, 1, G_CONST::Nlang),}),})};
  
  return MENU;
}
