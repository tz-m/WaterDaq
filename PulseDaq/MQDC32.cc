#include "MQDC32.h"

#include "PulseDaq.h"

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
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_START_ACQ, 0x0),"MQDC32_Setup: Write Stop Acquisition");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_SOFT_RESET, 0x1),"MQDC32_Setup: Write Soft Reset");
  usleep(1000000);
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_DATA_LEN_FMT, 0x2),"MQDC32_Setup: Write Data Length Format");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_MULTIEVENT, 0x0),"MQDC32_Setup: Write Multievent Off");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_MARKING_TYPE, 0x1),"MQDC32_Setup: Write EoE Mark");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_IRQ_VECTOR, 0x0),"MQDC32_Setup: Write IRQ Vector");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_IRQ_LEVEL, cvIRQ1),"MQDC32_Setup: Write IRQ Level");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_READOUT_RESET, 0x0),"MQDC32_Setup: Write Readout Reset");
  checkApiCall(MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_START_ACQ, 0x1),"MQDC32_Setup: Start Acquisition");
  return cvSuccess;
}

CVErrorCodes MQDC32_Reset_Data_Buffer(int32_t Handle)
{
  return MQDC32_Write_Register(Handle, MQDC32_BASE + MQDC32_READOUT_RESET, 0x1);
}

CVErrorCodes MQDC32_Read_Word(int32_t Handle, uint32_t * data)
{
  return MQDC32_Read_BLT(Handle, MQDC32_BASE + MQDC32_EVENT_READOUT_BUFFER, data);
}

bool MQDC32_IsHeader(uint32_t word)
{
  if (BitMask(word, 30, 2) == 1) return true;
  return false;
}

bool MQDC32_IsData(uint32_t word)
{
  if ((BitMask(word, 30, 2) == 0) && (BitMask(word, 21, 9) == 32)) return true;
  return false;
}

bool MQDC32_IsEoE(uint32_t word)
{
  if (BitMask(word, 30, 2) == 3) return true;
  return false;
}

void MQDC32_ParseHeaderWord(uint32_t word, MQDC32_Header * head)
{
  head->num_words = BitMask(word,0,12);
  head->fill = BitMask(word,12,3);
  head->module_id = BitMask(word,16,8);
  head->subheader = BitMask(word,24,6);
  head->hsig = BitMask(word,30,2);
}

void MQDC32_ParseDataWord(uint32_t word, MQDC32_Data * data)
{
  data->adc = BitMask(word,0,12);
  data->overflow = BitMask(word,15,1);
  data->channel = BitMask(word,16,5);
  data->fix = BitMask(word,21,9);
  data->dsig = BitMask(word,30,2);
}

void MQDC32_ParseEoEWord(uint32_t word, MQDC32_EoE * eoe)
{
  eoe->timestamp = BitMask(word,0,30);
  eoe->esig = BitMask(word,30,2);
}

CVErrorCodes MQDC32_ReadEvent(int32_t handle, MQDC32_Header * head, MQDC32_Data * data, MQDC32_EoE * eoe, MQDC32_EnableChannel chans)
{
  CVErrorCodes ret;
  uint32_t word;
  do
    {
      ret = MQDC32_Read_Word(handle,&word);
      if (ret == cvSuccess)
	{
	  if (MQDC32_IsHeader(word))
	    {
	      MQDC32_ParseHeaderWord(word,head);
	    }
	  else if (MQDC32_IsData(word))
	    {
	      MQDC32_ParseDataWord(word,data);
	      if (!chans.q(data->channel)) continue;
	    }
	  else if (MQDC32_IsEoE(word))
	    {
	      MQDC32_ParseEoEWord(word,eoe);
	    }
	}
      else if (ret == cvBusError) break;
      else
	{
	  std::cout << "Error in MQDC32_ReadEvent" << std::endl;
	  throw ret;
	}
    } while (ret == cvSuccess);
  return cvSuccess;
}

