#include "MQDC32.h"

CVErrorCodes MQDC32_Read_Register(int32_t Handle, uint32_t address, uint32_t *data)
{
  return CAENVME_ReadCycle(Handle, address, data, cvA32_U_DATA, cvD16);
}

CVErrorCodes MQDC32_Write_Register(int32_t Handle, uint32_t address, uint32_t data)
{
  return CAENVME_WriteCycle(Handle, address, &data, cvA32_U_DATA, cvD16);
}

// Setup:::
//     stop acquisition
//     soft reset
//     set data length format
//     set multi event off 0x6036
//     set end of event marking type to 1 (time stamp)
//     set irq vector 0x6012 to 0
//     set 0x6010 to 1
//     set 0x6034 to 1 to reset

// Do Acquisition:::
//     start acquisition 0x603A
//     while loop:
//         caenvme_irqwait
//         read 0x6030 for event length (D16)
//         read from buffer event length + 1 (BLT32)
//         set 0x6034 to reset
