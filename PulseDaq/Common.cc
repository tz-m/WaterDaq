#include "Common.h"

uint32_t BitMask(uint32_t val, uint32_t offset, uint32_t N)
{
  return ((val>>offset) & ((1<<N)-1));
}

void handler(int s)
{
  throw std::runtime_error(s+": Ctrl+C");
}

void checkApiCall(CVErrorCodes err, std::string s)
{
  if (err == cvSuccess) return;
  else 
    {
      std::cout << "Error in " << s << std::endl;
      throw err;
    }
}
