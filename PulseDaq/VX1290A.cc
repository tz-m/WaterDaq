#include "VX1290A.h"

CVErrorCodes VX1290A_Read_Register(int32_t Handle, uint32_t address, uint32_t * data)
{
  int time = 0;
  uint32_t rdata;
  do {
    checkApiCall(CAENVME_ReadCycle(Handle, VX1290A_BASE + VX1290A_MICRO_HND_ADD, &rdata, cvA24_U_DATA, cvD16),"CAENVME_ReadCycle");
    ++time;
  } while (!(rdata & 0x2) && time < 10000);
  if (time >= 10000)
    {
      throw std::runtime_error("Timeout Reading OpCode");
    }
  return CAENVME_ReadCycle(Handle, address, data, cvA24_U_DATA, cvD16);
}

CVErrorCodes VX1290A_Write_Register(int32_t Handle, uint32_t address, uint32_t data)
{
  int time = 0;
  uint32_t rdata;
  do {
    checkApiCall(CAENVME_ReadCycle(Handle, VX1290A_BASE + VX1290A_MICRO_HND_ADD, &rdata, cvA24_U_DATA, cvD16),"CAENVME_ReadCycle");
    ++time;
  } while (!(rdata & 0x2) && time < 10000);
  if (time >= 10000)
    {
      throw std::runtime_error("Timeout Writing OpCode");
    }
  return CAENVME_WriteCycle(Handle, address, &data, cvA24_U_DATA, cvD16);
}

CVErrorCodes VX1290A_Setup(int32_t Handle)
{
  //Software reset
  checkApiCall(VX1290A_Write_Register(Handle, VX1290A_BASE + VX1290A_MOD_RESET_ADD, 0x1),"VX1290A_Write: Reset");
  usleep(1000000);
  
  //Set Acquisition mode
  checkApiCall(VX1290A_Write_Register(Handle, VX1290A_BASE + VX1290A_MICRO_ADD, VX1290A_TRG_MATCH_OPCODE),"VX1290A_Write: Trigger Mode");

  //Set Window Width to 0x14*25ps = 500ns
  checkApiCall(VX1290A_Write_Register(Handle, VX1290A_BASE + VX1290A_MICRO_ADD, VX1290A_SET_WIN_WIDTH_OPCODE),"VX1290A_Write: Touch Window Width Register");
  checkApiCall(VX1290A_Write_Register(Handle, VX1290A_BASE + VX1290A_MICRO_ADD, 0x14),"VX1290A_Write: Set Window width");

  //Set window offset to 
  return cvSuccess;
}
