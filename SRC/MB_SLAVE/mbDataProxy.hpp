#pragma once

class CModbusDataProxy {
  
public:  
  static CModbusDataProxy& getInstance();
  
  static constexpr unsigned short QFields = 6;
  unsigned short Modbas_fields[QFields];

private:
    CModbusDataProxy() = default; 
    CModbusDataProxy(const CModbusDataProxy&) = delete;
    CModbusDataProxy& operator=(const CModbusDataProxy&) = delete;   
    
    
};


