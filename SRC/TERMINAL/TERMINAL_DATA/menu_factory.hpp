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
    const char* INDICATION[G_CONST::Nlang]     = {"ИНДИКАЦИЯ",       "INDICATION",       "IНДИКАЦIЯ"};
    const char* SETTINGS[G_CONST::Nlang]       = {"УСТАВКИ",         "SETTINGS",         "УСТАНОВКИ"};
    const char* LANGUAGE[G_CONST::Nlang]       = {"ЯЗЫК",            "LANGUAGE",         "МОВА"};   
    const char* PARAMS[G_CONST::Nlang]         = {"ПАРАМЕТРЫ",       "PARAMETERS",       "ПАРАМЕТРИ"};
    const char* SETTING_UP[G_CONST::Nlang]     = {"НАСТРОЙКА",       "SETTING UP",       "НАЛАШТУВАННЯ"};
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
      o("Rinsul", {}, rProcess.getPointerR(), "kOhm", 1, p0, vt::vfloat, nm::In1V),}),  
  o(Mn.SETTINGS[l],{
      o(Mn.PARAMS[l],{
          o("Rmin1",   {}, &set.Rmin1, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500),
          o("Rmin2",   {}, &set.Rmin2, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 500),
          o("RLadd",   {}, &set.RTadd, "kOhm", 1, p0, vt::ushort, nm::Ed1V, 0, 1000),}),
      o(Mn.SETTING_UP[l],{
          o::Dual("Ud",    rProcess.rAdc.getEPointer(static_cast<unsigned char>(CADC::EADC_NameCh::Ud)),    "d",1,p0,vt::sshort,
                  "shift", &set.shift_adc[static_cast<unsigned char>(CADC::EADC_NameCh::Ud)],               "d",1,p0,vt::sshort,nm::IE2V,-23,2070),
          o::Dual("ILeak1",rProcess.rAdc.getEPointer(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)),"d",1,p0,vt::sshort,
                  "shift", &set.shift_adc[static_cast<unsigned char>(CADC::EADC_NameCh::ILeak1)],           "d",1,p0,vt::sshort,nm::IE2V,-23,2070),
          o::Dual("ILeak2",rProcess.rAdc.getEPointer(static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)),"d",1,p0,vt::sshort,
                  "shift", &set.shift_adc[static_cast<unsigned char>(CADC::EADC_NameCh::ILeak2)],           "d",1,p0,vt::sshort,nm::IE2V,-23,2070),
          o::Dual("Rk1",  rProcess.getPointerR(),  "kOhm",1,p2,vt::vfloat,
                  "k1Ls", &set.k1Ls,               "",    1,p3,vt::vfloat,nm::IE2V, 0.1f, 100),
          o::Dual("Rk2",  rProcess.getPointerR(),  "kOhm",1,p2,vt::vfloat,
                  "k2Ls", &set.k2Ls,               "",    1,p3,vt::vfloat,nm::IE2V, 0.1f, 100),}),      
      o(Mn.LANGUAGE[l],{
          o("Language:", {}, &set.Language,"", 1, p0, vt::ushort, nm::Ed1V, 1, G_CONST::Nlang),}),})};
  
  return MENU;
}
