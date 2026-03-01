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
  
  static constexpr unsigned short QFields = 4;  // Количество регистров ModBus
  Register registers[QFields];
  bool isDirty = false;
  
private:
  CModbusDataProxy() {
    // Инициализация прав доступа
    registers[0] = {0, false}; // RO
    registers[1] = {0, false}; // RO
    registers[2] = {0, true};  // RW
    registers[3] = {0, true};  // RW
  }
  CModbusDataProxy(const CModbusDataProxy&) = delete;
  CModbusDataProxy& operator=(const CModbusDataProxy&) = delete;   
  
};




