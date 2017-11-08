#include "VX1290A.h"

CVErrorCodes VX1290A_Read_Register(int32_t Handle, uint32_t address, uint32_t * data)
{
  return CAENVME_ReadCycle(Handle, VX1290A_BASE + address, data, cvA24_U_DATA, cvD16);
}

CVErrorCodes VX1290A_Read_Word(int32_t Handle, uint32_t * data)
{
  return CAENVME_ReadCycle(Handle, VX1290A_BASE + VX1290A_OUT_BUFFER_ADD, data, cvA24_U_DATA, cvD32);
}

CVErrorCodes VX1290A_Read_OpCode(int32_t Handle, uint32_t * opaddress)
{
  int time = 0;
  uint32_t rdata;
  CVErrorCodes ret;
  do {
    ret = VX1290A_Read_Register(Handle, VX1290A_MICRO_HND_ADD, &rdata);
    if (ret != cvSuccess) return ret;
    ++time;
  } while (!(rdata & VX1290A_READ_OK) && time < 10000);
  if (time >= 10000)
    {
      throw std::runtime_error("Timeout reading READ_OK bit");
    }
  return VX1290A_Read_Register(Handle, VX1290A_MICRO_ADD, opaddress);
}

CVErrorCodes VX1290A_TouchRead_OpCode(int32_t Handle, uint32_t opaddress, uint32_t * data)
{
  CVErrorCodes ret = VX1290A_Read_OpCode(Handle, &opaddress);
  if (ret != cvSuccess) return ret;
  return VX1290A_Read_OpCode(Handle, data);
}

CVErrorCodes VX1290A_Write_Register(int32_t Handle, uint32_t address, uint32_t data)
{
  return CAENVME_WriteCycle(Handle, VX1290A_BASE + address, &data, cvA24_U_DATA, cvD16);
}

CVErrorCodes VX1290A_Write_OpCode(int32_t Handle, uint32_t opaddress)
{
  int time = 0;
  uint32_t rdata;
  CVErrorCodes ret;
  do {
    ret = VX1290A_Read_Register(Handle, VX1290A_MICRO_HND_ADD, &rdata);
    if (ret != cvSuccess) return ret;
    ++time;
  } while (!(rdata & VX1290A_WRITE_OK) && time < 10000);
  if (time >= 10000)
    {
      throw std::runtime_error("Timeout reading WRITE_OK bit");
    }
  return VX1290A_Write_Register(Handle, VX1290A_MICRO_ADD, opaddress);
}

CVErrorCodes VX1290A_TouchWrite_OpCode(int32_t Handle, uint32_t opaddress, uint32_t data)
{
  CVErrorCodes ret = VX1290A_Write_OpCode(Handle, opaddress);
  if (ret != cvSuccess) return ret;
  return VX1290A_Write_OpCode(Handle, data);
}

CVErrorCodes VX1290A_Setup(int32_t Handle)
{
  // Set event BERR enable (writing to control register automatically clears the module)
  uint32_t ctrl;
  checkApiCall(VX1290A_Read_Register(Handle, VX1290A_CONTROL_ADD, &ctrl),"Read control register");
  std::bitset<16> ctrlbit(ctrl);
  ctrlbit.set(0);
  ctrlbit.set(9);
  ctrl = ctrlbit.to_ulong();
  checkApiCall(VX1290A_Write_Register(Handle, VX1290A_CONTROL_ADD, ctrl),"Write control register");
  
  //Set Acquisition mode
  checkApiCall(VX1290A_Write_OpCode(Handle, VX1290A_TRG_MATCH_OPCODE),"VX1290A_Write_OpCode: Trigger Mode"); 
  
  //Set Window Width to 0x7d0 * 25ns = 50us
  checkApiCall(VX1290A_TouchWrite_OpCode(Handle, VX1290A_SET_WIN_WIDTH_OPCODE,VX1290A_WINDOW_WIDTH),"VX1290A_TouchWriteOpCode: Set Window Width");
  
  //Set window offset to 0xfc18 * 25ns = -25us
  checkApiCall(VX1290A_TouchWrite_OpCode(Handle, VX1290A_SET_WIN_OFFSET_OPCODE,VX1290A_WINDOW_OFFSET),"VX1290A_TouchWriteOpCode: Set Window Offset");

  //Enable subtraction of trigger time
  checkApiCall(VX1290A_Write_OpCode(Handle, VX1290A_EN_SUB_TRG_OPCODE),"VX1290A_Write_OpCode: Enable trigger time subtraction");

  //Set edge detection configuration to only leading
  checkApiCall(VX1290A_TouchWrite_OpCode(Handle, VX1290A_SET_DETECTION_OPCODE,0x2),"VX1290A_TouchWrite_OpCode: set edge detection configuration");
  
  //Set time resolution to minimum, 25ps
  checkApiCall(VX1290A_TouchWrite_OpCode(Handle, VX1290A_SET_TR_LEAD_LSB_OPCODE,0x3),"VX1290A_TouchWrite_OpCode: set time resolution");

  //Disable all channels
  checkApiCall(VX1290A_Write_OpCode(Handle, VX1290A_DIS_ALL_CH_OPCODE),"VX1290A_Write_OpCode: Disable all channels");
  // Enable two channels
  checkApiCall(VX1290A_Write_OpCode(Handle, VX1290A_EN_CHANNEL_OPCODE + VX1290A_CHANNEL_LE),"VX1290A_Write_OpCode: Enable channel LE");
  checkApiCall(VX1290A_Write_OpCode(Handle, VX1290A_EN_CHANNEL_OPCODE + VX1290A_CHANNEL_MAX),"VX1290A_Write_OpCode: Enable channel MAX");

  Status stat;
  checkApiCall(VX1290A_Status(Handle, &stat),"VX1290A_Status");

  usleep(1000000);
  
  return cvSuccess;
}

CVErrorCodes VX1290A_Status(int32_t Handle, Status * status)
{
  return VX1290A_Read_Register(Handle, VX1290A_STATUS_ADD, (uint32_t*)status);
}
  
CVErrorCodes VX1290A_Clear(int32_t Handle)
{
  return VX1290A_Write_Register(Handle, VX1290A_SW_CLEAR_ADD, 0x0);
}

CVErrorCodes VX1290A_Trigger(int32_t Handle)
{
  return VX1290A_Write_Register(Handle, VX1290A_SW_TRIGGER_ADD, 0x0);
}

CVErrorCodes VX1290A_ReadEvent(int32_t Handle, 
			       VX1290A_GlobalHeader * gh, 
			       VX1290A_GlobalTrailer * gt, 
			       std::vector<VX1290A_TDCHeader> * th, 
			       std::vector<VX1290A_TDCMeasurement> * tm, 
			       std::vector<VX1290A_TDCError> * te, 
			       std::vector<VX1290A_TDCTrailer> * tt, 
			       VX1290A_GlobalTrigTime * gtt)
{
  int time = 0;
  CVErrorCodes ret;
  Status stat;
  do {
    ret = VX1290A_Status(Handle, &stat);
    if (ret != cvSuccess) return ret;
    usleep(1000);
    ++time;
  } while (stat.DATA_READY != 1 && time < 10000);
  if (time >= 10000)
    {
      throw std::runtime_error("Data not ready to be read, TIMEOUT");
    }
  uint32_t word;
  do
    {
      ret = VX1290A_Read_Word(Handle,&word);
      if (ret == cvSuccess)
	{
	  if (VX1290A_IsGlobalHeader(word))
	    {
	      VX1290A_ParseGlobalHeader(word,gh);
	    }
	  else if (VX1290A_IsTDCHeader(word))
	    {
	      VX1290A_TDCHeader h;
	      VX1290A_ParseTDCHeader(word,&h);
	      th->push_back(h);
	    }
	  else if (VX1290A_IsTDCMeasurement(word))
	    {
	      VX1290A_TDCMeasurement m;
	      VX1290A_ParseTDCMeasurement(word,&m);
	      if (m.channel == VX1290A_CHANNEL_LE || 
	      m.channel == VX1290A_CHANNEL_MAX) 
		{
		  tm->push_back(m);
		}
	    }
	  else if (VX1290A_IsTDCTrailer(word))
	    {
	      VX1290A_TDCTrailer t;
	      VX1290A_ParseTDCTrailer(word,&t);
	      tt->push_back(t);
	    }
	  else if (VX1290A_IsTDCError(word))
	    {
	      VX1290A_TDCError e;
	      VX1290A_ParseTDCError(word,&e);
	      te->push_back(e);
	    }
	  else if (VX1290A_IsGlobalTrigTime(word))
	    {
	      VX1290A_ParseGlobalTrigTime(word,gtt);
	    }
	  else if (VX1290A_IsGlobalTrailer(word))
	    {
	      VX1290A_ParseGlobalTrailer(word,gt);
	      return cvSuccess;
	    }
	}
      else if (ret == cvBusError) break;
      else
	{
	  std::cout << "Error in VX1290A_ReadEvent" << std::endl;
	  throw ret;
	}
    } while (ret == cvSuccess);
  return cvSuccess;
}

bool VX1290A_IsGlobalHeader(uint32_t word)
{
  return (BitMask(word,27,5)==8);
}

bool VX1290A_IsGlobalTrailer(uint32_t word)
{
  return (BitMask(word,27,5)==16);
}

bool VX1290A_IsTDCHeader(uint32_t word)
{
  return (BitMask(word,27,5)==1);
}

bool VX1290A_IsTDCMeasurement(uint32_t word)
{
  return (BitMask(word,27,5)==0);
}

bool VX1290A_IsTDCError(uint32_t word)
{
  return (BitMask(word,27,5)==4);
}

bool VX1290A_IsTDCTrailer(uint32_t word)
{
  return (BitMask(word,27,5)==3);
}

bool VX1290A_IsGlobalTrigTime(uint32_t word)
{
  return (BitMask(word,27,5)==17);
}

void VX1290A_ParseGlobalHeader(uint32_t word, VX1290A_GlobalHeader * head)
{
  head->geo = BitMask(word,0,5);
  head->evt_cnt = BitMask(word,5,22);
  head->id = BitMask(word,27,5);
}

void VX1290A_ParseGlobalTrailer(uint32_t word, VX1290A_GlobalTrailer * trail)
{
  trail->geo = BitMask(word,0,5);
  trail->word_cnt = BitMask(word,5,16);
  trail->status_tdcerr = BitMask(word,24,1);
  trail->status_overflow = BitMask(word,25,1);
  trail->status_triglost = BitMask(word,26,1);
  trail->id = BitMask(word,27,5);
}

void VX1290A_ParseTDCHeader(uint32_t word, VX1290A_TDCHeader * head)
{
  head->bunch_id = BitMask(word,0,12);
  head->event_id = BitMask(word,12,12);
  head->tdc = BitMask(word,24,2);
  head->id = BitMask(word,27,5);
}

void VX1290A_ParseTDCMeasurement(uint32_t word, VX1290A_TDCMeasurement * meas)
{
  meas->tdc_meas = BitMask(word,0,21);
  meas->channel = BitMask(word,21,5);
  meas->leadtrail = BitMask(word,26,1);
  meas->id = BitMask(word,27,5);
}

void VX1290A_ParseTDCError(uint32_t word, VX1290A_TDCError * err)
{
  err->err_flags = BitMask(word,0,15);
  err->tdc = BitMask(word,24,2);
  err->id = BitMask(word,27,5);
}

void VX1290A_ParseTDCTrailer(uint32_t word, VX1290A_TDCTrailer * trail)
{
  trail->word_cnt = BitMask(word,0,12);
  trail->evt_id = BitMask(word,12,12);
  trail->tdc = BitMask(word,24,2);
  trail->id = BitMask(word,27,5);
}

void VX1290A_ParseGlobalTrigTime(uint32_t word, VX1290A_GlobalTrigTime * trig)
{
  trig->trig_time = BitMask(word,0,27);
  trig->id = BitMask(word,27,5);
}


