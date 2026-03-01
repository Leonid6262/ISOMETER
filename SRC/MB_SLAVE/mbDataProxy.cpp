#include "mbDataProxy.hpp"
#include "system_LPC177x.h"

CModbusDataProxy& CModbusDataProxy::getInstance() {
  static CModbusDataProxy instance;
  return instance;
}



