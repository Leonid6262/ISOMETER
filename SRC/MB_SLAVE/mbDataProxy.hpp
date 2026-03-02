#pragma once

class CModbusDataProxy {
  
public:  
  static CModbusDataProxy& getInstance() {
    static CModbusDataProxy instance;
    return instance;
  }
  
  struct Register {
    unsigned short value;       // Данные
    bool isWritable;            // Права доступа
  };
  
  static constexpr unsigned short QFields = 5;  // Количество регистров ModBus
  Register registers[QFields];
  bool isDirty = false;
  
  enum ERegisters {
    RegStatus,
    RegR,
    RegAlarm1,
    RegAlarm2,
    RegSaveEEP
  };
  
  static constexpr unsigned short SaveCode = 0xAA55;
  
private:
  CModbusDataProxy() {
    // Инициализация прав доступа
    registers[RegStatus]  = {0, false}; // RO
    registers[RegR]       = {0, false}; // RO
    registers[RegAlarm1]  = {0, true};  // RW
    registers[RegAlarm2]  = {0, true};  // RW
    registers[RegSaveEEP] = {0, true};  // RW
  }
  CModbusDataProxy(const CModbusDataProxy&) = delete;
  CModbusDataProxy& operator=(const CModbusDataProxy&) = delete;   
  
};




