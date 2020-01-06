#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include <string.h>
#include <iostream>

/*
  Set the n-th bit (0 counting): number |= 1UL << n;
  
  Clear the n-th bit (0 counting): number &= ~(1UL << n);

  Toggle the n-th bit (0 counting): number ^= 1UL << n;

  Changing the n-th bit to x: number ^= (-x ^ number) & (1UL << n);
*/

typedef struct {
  CAEN_DGTZ_ConnectionType LinkType;
  uint32_t VMEBaseAddress;
  uint32_t RecordLength;
  uint32_t ChannelMask;
  int EventAggr;
  CAEN_DGTZ_PulsePolarity_t PulsePolarity;
  CAEN_DGTZ_DPP_AcqMode_t AcqMode;
  CAEN_DGTZ_IOLevel_t IOlev;
} DigitizerParams_t;

void CheckErrorCode(CAEN_DGTZ_ErrorCode ret)
{
  switch (ret) {
  case 0: break;
  case -1L: std::cout << "CommError" << std::endl;
    break;
  case -2L: std::cout << "GenericError" << std::endl;
    break;
  case -3L: std::cout << "InvalidParam" << std::endl;
    break;
  case -4L: std::cout << "InvalidLinkType" << std::endl;
    break;
  case -5L: std::cout << "InvalidHandler" << std::endl;
    break;
  case -6L: std::cout << "MaxDevicesError" << std::endl;
    break;
  case -7L: std::cout << "BadBoardType" << std::endl;
    break;
  case -8L: std::cout << "BadInterruptLev" << std::endl;
    break;
  case -9L: std::cout << "BadEventNumber" << std::endl;
    break;
  case -10L: std::cout << "ReadDeviceRegisterFail" << std::endl;
    break;
  case -11L: std::cout << "WriteDeviceRegisterFail" << std::endl;
    break;
  case -13L: std::cout << "InvalidChannelNumber" << std::endl;
    break;
  case -14L: std::cout << "ChannelBusy" << std::endl;
    break;
  case -15L: std::cout << "FPIOModeInvalid" << std::endl;
    break;
  case -16L: std::cout << "WrongAcqMode" << std::endl;
    break;
  case -17L: std::cout << "FunctionNotAllowed" << std::endl;
    break;
  case -18L: std::cout << "Timeout" << std::endl;
    break;
  case -19L: std::cout << "InvalidBuffer" << std::endl;
    break;
  case -20L: std::cout << "EventNotFound" << std::endl;
    break;
  case -21L: std::cout << "InvalidEvent" << std::endl;
    break;
  case -22L: std::cout << "OutOfMemory" << std::endl;
    break;
  case -23L: std::cout << "CalibrationError" << std::endl;
    break;
  case -24L: std::cout << "DigitizerNotFound" << std::endl;
    break;
  case -25L: std::cout << "DigitizerAlreadyOpen" << std::endl;
    break;
  case -26L: std::cout << "DigitizerNotReady" << std::endl;
    break;
  case -27L: std::cout << "InterruptNotConfigured" << std::endl;
    break;
  case -28L: std::cout << "DigitizerMemoryCorrupted" << std::endl;
    break;
  case -29L: std::cout << "DPPFirmwareNotSupported" << std::endl;
    break;
  case -30L: std::cout << "InvalidLicense" << std::endl;
    break;
  case -31L: std::cout << "InvalidDigitizerStatus" << std::endl;
    break;
  case -32L: std::cout << "UnsupportedTrace" << std::endl;
    break;
  case -33L: std::cout << "InvalidProbe" << std::endl;
    break;
  case -34L: std::cout << "UnsupportedBaseAddress" << std::endl;
    break;
  case -99L: std::cout << "NotYetImplemented" << std::endl;
    break;
  default: std::cout << "UnspecifiedError" << std::endl;
    break;
  }
  if (ret != CAEN_DGTZ_Success) exit(ret);
}


int main(int argc, char * argv[])
{
  int handle;
  CAEN_DGTZ_ErrorCode ret;

  //buffers to store data
  char *buffer = NULL;
  CAEN_DGTZ_DPP_PHA_Event_t *Events[8];
  CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=NULL;

  // digitizer configuration parameters
  CAEN_DGTZ_DPP_PHA_Params_t DPPParams;
  DigitizerParams_t Params;

  // set parameters
  uint32_t NumEvents[8];
  memset(&Params, 0, sizeof(DigitizerParams_t));
  memset(&DPPParams, 0, sizeof(CAEN_DGTZ_DPP_PHA_Params_t));

  //communication parameters
  Params.LinkType = CAEN_DGTZ_USB;
  Params.VMEBaseAddress = 0;
  Params.IOlev = CAEN_DGTZ_IOLevel_NIM;

  //acquisition parameters
  Params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
  Params.RecordLength = 2000;
  Params.ChannelMask = (1<<0) + (1<<1) + (0<<2) + (0<<3) + (0<<4) + (0<<5) + (0<<6) + (0<<7); // {1=enable, 0=disable} << =left.bit.shift {channel number}
  Params.EventAggr = 0;
  Params.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive;

  for (int ch = 0; ch < 8; ch++)
    {
      DPPParams.thr[ch] = 50;
      DPPParams.k[ch] = 6000;
      DPPParams.m[ch] = 1000;
      DPPParams.M[ch] = 110000;
      DPPParams.ftd[ch] = 800;
      DPPParams.a[ch] = 16;
      DPPParams.b[ch] = 296;
      DPPParams.trgho[ch] = 496;
      DPPParams.nsbl[ch] = 6;
      DPPParams.nspk[ch] = 2;
      DPPParams.pkho[ch] = 960;
      DPPParams.blho[ch] = 500;
      DPPParams.enf[ch] = 1.0;
      DPPParams.decimation[ch] = 0;
      DPPParams.dgain[ch] = 0;
      DPPParams.otrej[ch] = 0;
      DPPParams.trgwin[ch] = 0;
      DPPParams.twwdt[ch] = 100;
    }
  
  CheckErrorCode(CAEN_DGTZ_OpenDigitizer(Params.LinkType,0,0,Params.VMEBaseAddress,&handle));
  
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CheckErrorCode(CAEN_DGTZ_GetInfo(handle,&BoardInfo));
  std::cout << "  ModelName: " << BoardInfo.ModelName << std::endl;
  std::cout << "  Model: " << BoardInfo.Model << std::endl;
  std::cout << "  Channels: " << BoardInfo.Channels << std::endl;
  std::cout << "  FormFactor: " << BoardInfo.FormFactor << std::endl;
  std::cout << "  FamilyCode: " << BoardInfo.FamilyCode << std::endl;
  std::cout << "  ROC_FirmwareRel: " << BoardInfo.ROC_FirmwareRel << std::endl;
  std::cout << "  AMC_FirmwareRel: " << BoardInfo.AMC_FirmwareRel << std::endl;
  std::cout << "  SerialNumber: " << BoardInfo.SerialNumber << std::endl;
  std::cout << "  PCB_Revision: " << BoardInfo.PCB_Revision << std::endl;
  std::cout << "  ADC_NBits: " << BoardInfo.ADC_NBits << std::endl;
  std::cout << "  CommHandle: " << BoardInfo.CommHandle << std::endl;
  std::cout << "  VMEHandle: " << BoardInfo.VMEHandle << std::endl;
  std::cout << "  License: " << BoardInfo.License << std::endl;

  ////////////////////////////////
  //Write configuration to board//
  ////////////////////////////////

  //uint32_t value;

  CheckErrorCode(CAEN_DGTZ_Reset(handle));
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8000,0x004E0115));
  CheckErrorCode(CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime));
  CheckErrorCode(CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED));
  CheckErrorCode(CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength));
  CheckErrorCode(CAEN_DGTZ_SetIOLevel(handle, Params.IOlev));
  CheckErrorCode(CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT));
  CheckErrorCode(CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask));
  CheckErrorCode(CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0));
  CheckErrorCode(CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled));
  CheckErrorCode(CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams));

  for (int i = 0; i < 8; i++)
    {
      if (Params.ChannelMask & (1<<i)) {
	CheckErrorCode(CAEN_DGTZ_SetChannelDCOffset(handle, i, 0x8000));
	CheckErrorCode(CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, 1000));
	CheckErrorCode(CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity));
      }
    }


  
	      
  
  /*
  // Record Length, 20 us
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8020,
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8020,20000/16));

  // Input dynamic range, 2 Vpp
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8028,0));

  // Number of Events per aggregate, 50???
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8034,50));

  // Pre trigger, 2 us
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8038,2000/8));

  // RC-CR2 smoothing factor, 16
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8054,0x8));

  // Input rise time, 296 ns
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8058,296/2));

  // Trapezoid rise time, 6 us
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0.805C,
  */

  uint32_t AllocatedSize;
  CheckErrorCode(CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize));
  CheckErrorCode(CAEN_DGTZ_MallocDPPEvents(handle, Events, &AllocatedSize));
  CheckErrorCode(CAEN_DGTZ_MallocDPPWaveforms(handle, &Waveform, &AllocatedSize));


  // Start acquisition
  uint32_t BufferSize;
  CheckErrorCode(CAEN_DGTZ_SWStartAcquisition(handle));

  for (int numevents = 0; numevents<100; numevents++)
    {
      CheckErrorCode(CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize));
      CheckErrorCode(CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, Events, NumEvents));
      for (int ch = 0; ch < 8; ch++)
	{
	  if (!(Params.ChannelMask & (1<<ch))) continue;
	  
	  for (uint32_t ev = 0; ev < NumEvents[ch]; ev++) {
	    std::cout << "Event " << ev << ": " << (Events[ch][ev].Energy) << std::endl;
	  }
	}
    }

  CheckErrorCode(CAEN_DGTZ_SWStopAcquisition(handle));
  CheckErrorCode(CAEN_DGTZ_CloseDigitizer(handle));
  CheckErrorCode(CAEN_DGTZ_FreeReadoutBuffer(&buffer));
  CheckErrorCode(CAEN_DGTZ_FreeDPPEvents(handle, Events));
  CheckErrorCode(CAEN_DGTZ_FreeDPPWaveforms(handle, Waveform));
  
  std::cout << "Exiting with status code: " << ret << std::endl;
  return ret;
}
