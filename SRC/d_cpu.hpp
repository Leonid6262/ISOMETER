#pragma once

class CD_cpu
{
  
private:
  
  static constexpr unsigned short B_ULED = 9;        // U-LED
  static constexpr unsigned short B_Q1VF = 13;       // Q1VF
  
public:  
  
  static inline void UserLedOn() { LPC_GPIO0->CLR = (1UL << B_ULED); }   // ULED
  static inline void UserLedOff() { LPC_GPIO0->SET = (1UL << B_ULED); }  // ULED
  static inline void UserLedToggle() {
    if (LPC_GPIO0->PIN & (1UL << B_ULED)) { LPC_GPIO0->CLR = (1UL << B_ULED); } 
    else { LPC_GPIO0->SET = (1UL << B_ULED); }
  } 
  static inline void Q1VF_On() { LPC_GPIO1->CLR = (1UL << B_Q1VF); }     // Q1VF
  static inline void Q1VF_Off() { LPC_GPIO1->SET = (1UL << B_Q1VF); }    // Q1VF
  
};

