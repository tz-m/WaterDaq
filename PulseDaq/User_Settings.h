#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#define MQDC32_BASE    0x11110000
#define MQDC32_CHANNEL_CHARGE 0x0

#define VX1290A_BASE 0xEEEE0000
#define VX1290A_CHANNEL_LE 0x1 // channel 1
#define VX1290A_CHANNEL_MAX 0x19 // channel 25 = 16+9
#define VX1290A_WINDOW_WIDTH 0x07d0  // 0x07d0 * 25ns = 50us
#define VX1290A_WINDOW_OFFSET 0xfc18 // 0xfc18 * 25ns = -25us

#include "Common.h"

class Settings
{
 public:
  Settings();
  bool IsValid();
  void SetVerbose(bool v);
  void SetNumEvents(uint32_t n);
  void SetDelay(uint32_t d);
  void SetInteractive(uint32_t i);
  void SetUseTDC(bool d);
  bool Verbose();
  uint32_t NumEvents();
  uint32_t Delay();
  uint32_t Interactive();
  bool UseInteractive();
  bool UseTDC();

 private:
  bool setVerbose;
  bool setNumEvents;
  bool setDelay;
  bool setInteractive;
  bool setTDC;

  bool verb;
  uint32_t num;
  uint32_t del;
  uint32_t inter;
  bool tdc;
};

#endif
