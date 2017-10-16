#include "MQDC32.h"
#include "CAENVMElib.h"
#include <iostream>
#include <unistd.h>

uint32_t GetN(uint32_t val, uint32_t offset, uint32_t N)
{
  return ((val>>offset) & ((1<<N)-1));
}

int main(int argc, char ** argv)
{
  std::cout << "argc=" << argc << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << "argv[" << i << "]=" << argv[i] << std::endl;

  int32_t handle;
  CVErrorCodes ret;
  ret = CAENVME_Init(cvV1718, 0, 0, &handle);
  if (ret == cvSuccess)
    {
      ret = MQDC32_Write_Register(handle, MQDC32_BASE+ 0x603A, 0x0);
      std::cout << "stop acquisition = " << ret << std::endl;
      ret = MQDC32_Write_Register(handle, 0x11110000 + 0x6008, 0x1);
      usleep(1000000);
      std::cout << "soft reset = " << ret << std::endl;
      uint32_t val = 0;
      ret = MQDC32_Read_Register(handle, 0x11110000 + 0x6032, &val);
      std::cout << "read datalenformat masked =" << std::hex << GetN(val,0,8) << std::dec << "  ret=" << ret << std::endl;
      std::cout << "read datalenformat hex=" << std::hex << val << std::dec << "  dec=" << val << std::endl;

      ret = MQDC32_Write_Register(handle, 0x11110000 + 0x6032, 0x1);
      std::cout << "write datalenformat ret=" << ret << std::endl;
      ret = MQDC32_Read_Register(handle, 0x11110000 + 0x6022, &val);
      std::cout << "read datalenformat masked=" << std::hex << GetN(val,0,8) << std::dec << "  ret=" << ret << std::endl;
      std::cout << "read datalenformat hex=" << std::hex << val << std::dec << "  dec=" << val << std::endl;
    }

  ret = CAENVME_End(handle);
  return 1;
}
