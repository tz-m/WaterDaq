#include "MQDC32.h"
#include "CAENVMElib.h"
#include <iostream>
#include <unistd.h>

uint32_t GetN(uint32_t val, uint32_t offset, uint32_t N)
{
  return ((val>>offset) & ((1<<N)-1));
}

void ParseWord(uint32_t word)
{
  if (GetN(word, 30, 2) == 1)
    {
      std::cout << "=========Header========" << std::endl;
      std::cout << "num following words = " << GetN(word,0,12) << std::endl;
      std::cout << "fill = " << GetN(word, 12, 3) << std::endl;
      std::cout << "module id = " << GetN(word, 16, 8) << std::endl;
      std::cout << "subheader = " << GetN(word, 24, 6) << std::endl;
      std::cout << "hsig = " << GetN(word, 30, 2) << std::endl;
      std::cout << "=======================" << std::endl;
    }
  if (GetN(word, 30, 2) == 0)
    {
      if (GetN(word, 16, 5) == 0 && GetN(word, 21, 9) != 32) return;
      if (GetN(word, 16, 5) == 0 || GetN(word, 16, 5) == 1)
	{
	  std::cout << "==========Data=========" << std::endl;
	  std::cout << "adc = " << GetN(word, 0, 12) << std::endl;
	  std::cout << "overflow = " << GetN(word, 15, 1) << std::endl;
	  std::cout << "channel = " << GetN(word, 16, 5) << std::endl;
	  std::cout << "fix = " << GetN(word, 21, 9) << std::endl;
	  std::cout << "dsig = " << GetN(word, 30, 2) << std::endl;
	  std::cout << "=======================" << std::endl;
	}
    }
  if (GetN(word, 30, 2) == 3)
    {
      std::cout << "==========EoE==========" << std::endl;
      std::cout << "timestamp = " << GetN(word, 0, 30) << std::endl;
      std::cout << "esig = " << GetN(word, 30, 2) << std::endl;
      std::cout << "=======================" << std::endl;
    }
}

int main(int argc, char ** argv)
{
  std::cout << "argc=" << argc << std::endl;
  for (int i = 0; i < argc; ++i)
    std::cout << "argv[" << i << "]=" << argv[i] << std::endl;

  int32_t handle;
  CVErrorCodes ret;
  ret = CAENVME_Init(cvV1718, 0, 0, &handle);
  //std::cout << "initialise=" << ret << std::endl;
  if (ret == cvSuccess)
    {
      ret = MQDC32_Setup(handle);
      if (ret != cvSuccess) { std::cout << "problem in setup" << std::endl; return 0;}
      ret = MQDC32_Start_Acquisition(handle);
      if (ret != cvSuccess) { std::cout << "problem starting acquisition" << std::endl; return 0;}
      int n = 0;
      while (n < 10)
	{
	  //std::cout << "n=" << n << std::endl;
	  CAENVME_IRQEnable(handle,0x1);
	  std::cout << "enable irq " << ret << std::endl;
	  //std::cout << "cleared output register" << std::endl;
	  ret = CAENVME_IRQWait(handle,0x1,1000);
	  std::cout << "wait irq " << ret << std::endl;
	  /*
if (ret != cvSuccess)
	    {
	      std::cout << "Failed to acquire any new data before timeout" << std::endl;
	      return 0;
	    }
	  */
	  //std::cout << "caught IRQ" << std::endl;
	  uint32_t data;//, nwords;
	  //MQDC32_Num_Words(handle,&nwords);
	  //std::cout << "nwords=" << std::hex << nwords << std::dec << std::endl;
	  ret = cvSuccess;
	  int i = 0;
	  do
	    {
	      ret = MQDC32_Read_Word(handle,&data);
	      //if (ret == cvSuccess) ParseWord(data);
	      ++i;
	    } while (ret == cvSuccess);
	  //std::cout << "done reading data" << std::endl;
	  MQDC32_Reset_Data_Buffer(handle);
	  //std::cout << "reset data buffer" << std::endl;
	  ++n;
	}

    }

  /*

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
  */
  ret = CAENVME_End(handle);
  return 1;
}
