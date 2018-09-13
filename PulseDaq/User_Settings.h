#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#define MQDC32_BASE    0x11110000
#define MQDC32_CHANNEL_CHARGE 0x4

// Make sure the trigger input is 50Ohm terminated!!!
#define VX1290A_BASE 0xEEEE0000
#define VX1290A_CHANNEL_LE 0x4 // channel 1
#define VX1290A_CHANNEL_MAX 0x1a // channel 25 = 16+9
#define VX1290A_WINDOW_WIDTH 0x0014  // 0x0014 * 25ns = 500ns
#define VX1290A_WINDOW_OFFSET 0xfff6 // 0xfff6 * 25ns = -250ns

#define VX1718_USB_CHANNEL 0 // Most likely 0 or 1. To find out easily, use the CAENVMElib/sample program and test different <VMEdevice> values.

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
