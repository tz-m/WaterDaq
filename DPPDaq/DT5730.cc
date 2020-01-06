#include "DT5730.h"

uint32_t RecordLengthCode(int nanoseconds)
{
  value = nanoseconds/16;
}

uint16_t RecordLengthRegister(int channel = -1)
{
  uint16_t reg;
  if (channel == -1) reg = 0x8020;
  else if (channel >= 0 && channel <= 7) reg = 0x1020+channel*0x100;
  else {
    std::cout << "DT5730::RecordLengthRegister: bad channel specifier --> " << channel << std::endl;
    exit(-1);
  }
  return reg;
}

uint16_t InputDynamicRangeRegister(int dynrange, int channel = -1, uint16_t reg, uint32_t value)
{
  uint16_t reg;
  if (channel == -1) reg = 0x8028;
  else if (channel >= 0 && channel <= 7) reg = 0x1028+channel*0x100;
  else {
    std::cout << "DT5730::InputDynamicRange: bad channel specifier --> " << channel << std::endl;
    exit(-1);
  }
  return reg;
}

uint32_t InputDynamicRangeCode(uint32_t dynrange)
{
  if (dynrange != 0 && dynrange != 1) {
    std::cout << "Choose a dynamic range that is EITHER 0 (=2Vpp) OR 1 (=0.5Vpp)!" << std::endl;
    exit(-1);
  }
  return dynrange;
}

uint16_t NumEvtPerAggregateRegister(int channel = -1)
{
  if (channel = -1) return 0x8034;
  else if (channel >= 0 && channel <= 7) return 0x1034+channel*0x100;
  else {
    std::cout << "DT5730::NumEvtPerAggregateRegister bad choice of channel" << std::endl;
    exit(-1);
  }
  return -1;
}

uint32_t NumEvtPerAggregateValue(uint32_t val)
{
  return val;
}

uint16_t PreTriggerRegister(int channel = -1)
{
  if (channel = -1) return 0x8038;
  else if (channel >= 0 && channel <= 7) return 0x1038+channel*0x100;
  else {
    std::cout << "DT5730::PreTriggerRegister bad choice of channel" << std::endl;
    exit(-1);
  }
  return -1;
}

uint32_t PreTriggerValue(int nanoseconds)
{
  return nanoseconds/8;
}


uint16_t DataFlushRegister(int channel = -1)
{
  if (channel = -1) return 0x803C;
  else if (channel >= 0 && channel <= 7) return 0x103C+channel*0x100;
  else {
    std::cout << "DT5730::DataFlushRegister bad choice of channel" << std::endl;
    exit(-1);
  }
  return -1;
}

