#include "MQDC32.h"
#include <iostream>
#include <unistd.h>

CVErrorCodes MQDC32_Read_Register(int32_t Handle, uint32_t address, uint32_t *data)
{
  return CAENVME_ReadCycle(Handle, address, data, cvA32_U_DATA, cvD16);
}

CVErrorCodes MQDC32_Write_Register(int32_t Handle, uint32_t address, uint32_t data)
{
  return CAENVME_WriteCycle(Handle, address, &data, cvA32_U_DATA, cvD16);
}

CVErrorCodes MQDC32_Read_BLT(int32_t Handle, uint32_t address, uint32_t *data)
{
  return CAENVME_ReadCycle(Handle, address, data, cvA32_U_DATA, cvD32);
}

CVErrorCodes MQDC32_Setup(int32_t Handle)
{
  CVErrorCodes ret;
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_START_ACQ, 0x0);
  if (ret != 0) { std::cout << "========Error stopping acquisition==========" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_SOFT_RESET, 0x1);
  if (ret != 0) { std::cout << "========Error performing soft reset=========" << std::endl; return ret; }
  usleep(1000000);
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_DATA_LEN_FMT, 0x2);
  if (ret != 0) { std::cout << "=====Error setting data length format=======" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_MULTIEVENT, 0x0);
  if (ret != 0) { std::cout << "=======Error setting multievent off=========" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_MARKING_TYPE, 0x1);
  if (ret != 0) { std::cout << "===========Error setting EoE mark===========" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_IRQ_VECTOR, 0x1);
  if (ret != 0) { std::cout << "==========Error setting IRQ vector==========" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_IRQ_LEVEL, 0x1);
  if (ret != 0) { std::cout << "==========Error setting IRQ level===========" << std::endl; return ret; }
  ret = MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_READOUT_RESET, 0x0);
  if (ret != 0) { std::cout << "===========Error resetting readout==========" << std::endl; return ret; }
  return cvSuccess;
}

CVErrorCodes MQDC32_Start_Acquisition(int32_t Handle)
{
  return MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_START_ACQ, 0x1);
}

CVErrorCodes MQDC32_Num_Words(int32_t Handle, uint32_t * data)
{
  return MQDC32_Read_Register(Handle, MQDC32_BASE + MQDC32_BUF_DATA_LEN, data);
}

CVErrorCodes MQDC32_Reset_Data_Buffer(int32_t Handle)
{
  return MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_READOUT_RESET, 0x1);
}

CVErrorCodes MQDC32_Read_Word(int32_t Handle, uint32_t * data)
{
  return MQDC32_Read_BLT(Handle, MQDC32_BASE + MQDC32_EVENT_READOUT_BUFFER, data);
}

// Do Acquisition:::
//     start acquisition 0x603A
//     while loop:
//         caenvme_irqwait
//         read 0x6030 for event length (D16)
//         read from buffer event length + 1 (BLT32)
//         set 0x6034 to reset
