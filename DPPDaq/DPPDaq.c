#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
  /*
  switch (ret) {
  case 0: break;
  case -1L: cout << "CommError" << endl;
    break;
  case -2L: cout << "GenericError" << endl;
    break;
  case -3L: cout << "InvalidParam" << endl;
    break;
  case -4L: cout << "InvalidLinkType" << endl;
    break;
  case -5L: cout << "InvalidHandler" << endl;
    break;
  case -6L: cout << "MaxDevicesError" << endl;
    break;
  case -7L: cout << "BadBoardType" << endl;
    break;
  case -8L: cout << "BadInterruptLev" << endl;
    break;
  case -9L: cout << "BadEventNumber" << endl;
    break;
  case -10L: cout << "ReadDeviceRegisterFail" << endl;
    break;
  case -11L: cout << "WriteDeviceRegisterFail" << endl;
    break;
  case -13L: cout << "InvalidChannelNumber" << endl;
    break;
  case -14L: cout << "ChannelBusy" << endl;
    break;
  case -15L: cout << "FPIOModeInvalid" << endl;
    break;
  case -16L: cout << "WrongAcqMode" << endl;
    break;
  case -17L: cout << "FunctionNotAllowed" << endl;
    break;
  case -18L: cout << "Timeout" << endl;
    break;
  case -19L: cout << "InvalidBuffer" << endl;
    break;
  case -20L: cout << "EventNotFound" << endl;
    break;
  case -21L: cout << "InvalidEvent" << endl;
    break;
  case -22L: cout << "OutOfMemory" << endl;
    break;
  case -23L: cout << "CalibrationError" << endl;
    break;
  case -24L: cout << "DigitizerNotFound" << endl;
    break;
  case -25L: cout << "DigitizerAlreadyOpen" << endl;
    break;
  case -26L: cout << "DigitizerNotReady" << endl;
    break;
  case -27L: cout << "InterruptNotConfigured" << endl;
    break;
  case -28L: cout << "DigitizerMemoryCorrupted" << endl;
    break;
  case -29L: cout << "DPPFirmwareNotSupported" << endl;
    break;
  case -30L: cout << "InvalidLicense" << endl;
    break;
  case -31L: cout << "InvalidDigitizerStatus" << endl;
    break;
  case -32L: cout << "UnsupportedTrace" << endl;
    break;
  case -33L: cout << "InvalidProbe" << endl;
    break;
  case -34L: cout << "UnsupportedBaseAddress" << endl;
    break;
  case -99L: cout << "NotYetImplemented" << endl;
    break;
  default: cout << "UnspecifiedError" << endl;
    break;
  }
  */
  if (ret != CAEN_DGTZ_Success) exit(ret);
}


int main(int argc, char * argv[])
{
  int handle;
  //CAEN_DGTZ_ErrorCode ret;

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
  Params.ChannelMask = 0x3;
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
  /*  cout << "  ModelName: " << BoardInfo.ModelName << endl;
  cout << "  Model: " << BoardInfo.Model << endl;
  cout << "  Channels: " << BoardInfo.Channels << endl;
  cout << "  FormFactor: " << BoardInfo.FormFactor << endl;
  cout << "  FamilyCode: " << BoardInfo.FamilyCode << endl;
  cout << "  ROC_FirmwareRel: " << BoardInfo.ROC_FirmwareRel << endl;
  cout << "  AMC_FirmwareRel: " << BoardInfo.AMC_FirmwareRel << endl;
  cout << "  SerialNumber: " << BoardInfo.SerialNumber << endl;
  cout << "  PCB_Revision: " << BoardInfo.PCB_Revision << endl;
  cout << "  ADC_NBits: " << BoardInfo.ADC_NBits << endl;
  cout << "  CommHandle: " << BoardInfo.CommHandle << endl;
  cout << "  VMEHandle: " << BoardInfo.VMEHandle << endl;
  cout << "  License: " << BoardInfo.License << endl;*/

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
      printf("NumEvents %n",NumEvents);
      for (int ch = 0; ch < 8; ch++)
	{
	  if (!(Params.ChannelMask & (1<<ch))) continue;
	  
	  for (uint32_t ev = 0; ev < NumEvents[ch]; ev++) {
	    //cout << "Event " << ev << ": " << (Events[ch][ev].Energy) << endl;
	    printf("Event %d: %d",ev,(Events[ch][ev].Energy));
	  }
	}
    }

  CheckErrorCode(CAEN_DGTZ_SWStopAcquisition(handle));
  CheckErrorCode(CAEN_DGTZ_CloseDigitizer(handle));
  CheckErrorCode(CAEN_DGTZ_FreeReadoutBuffer(&buffer));
  CheckErrorCode(CAEN_DGTZ_FreeDPPEvents(handle,Events));
  CheckErrorCode(CAEN_DGTZ_FreeDPPWaveforms(handle, Waveform));
  
  //cout << "Exiting with status code: " << ret << endl;
  return 0;//ret;
}
