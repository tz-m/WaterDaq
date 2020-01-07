#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include "TCanvas.h"

#include <string>
#include <iostream>

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

void CheckErrorCode(CAEN_DGTZ_ErrorCode ret, std::string caller)
{
  switch (ret) {
    case 0: break;
    case -1L: std::cout << caller << ": CommError" << std::endl;//printf("%s: CommError\n",caller);
      break;
    case -2L: std::cout << caller << ": GenericError" << std::endl;//printf("%s: GenericError\n",caller);
      break;
    case -3L: std::cout << caller << ": InvalidParam" << std::endl;
      break;
    case -4L: std::cout << caller << ": InvalidLinkType" << std::endl;
      break;
    case -5L: std::cout << caller << ": InvalidHandler" << std::endl;
      break;
    case -6L: std::cout << caller << ": MaxDevicesError" << std::endl;
      break;
    case -7L: std::cout << caller << ": BadBoardType" << std::endl;
      break;
    case -8L: std::cout << caller << ": BadInterruptLev" << std::endl;
      break;
    case -9L: std::cout << caller << ": BadEventNumber" << std::endl;
      break;
    case -10L: std::cout << caller << ": ReadDeviceRegisterFail" << std::endl;
      break;
    case -11L: std::cout << caller << ": WriteDeviceRegisterFail" << std::endl;
      break;
    case -13L: std::cout << caller << ": InvalidChannelNumber" << std::endl;
      break;
    case -14L: std::cout << caller << ": ChannelBusy" << std::endl;
      break;
    case -15L: std::cout << caller << ": FPIOModeInvalid" << std::endl;
      break;
    case -16L: std::cout << caller << ": WrongAcqMode" << std::endl;
      break;
    case -17L: std::cout << caller << ": FunctionNotAllowed" << std::endl;
      break;
    case -18L: std::cout << caller << ": Timeout" << std::endl;
      break;
    case -19L: std::cout << caller << ": InvalidBuffer" << std::endl;
      break;
    case -20L: std::cout << caller << ": EventNotFound" << std::endl;
      break;
    case -21L: std::cout << caller << ": InvalidEvent" << std::endl;
      break;
    case -22L: std::cout << caller << ": OutOfMemory" << std::endl;
      break;
    case -23L: std::cout << caller << ": CalibrationError" << std::endl;
      break;
    case -24L: std::cout << caller << ": DigitizerNotFound" << std::endl;
      break;
    case -25L: std::cout << caller << ": DigitizerAlreadyOpen" << std::endl;
      break;
    case -26L: std::cout << caller << ": DigitizerNotReady" << std::endl;
      break;
    case -27L: std::cout << caller << ": InterruptNotConfigured" << std::endl;
      break;
    case -28L: std::cout << caller << ": DigitizerMemoryCorrupted" << std::endl;
      break;
    case -29L: std::cout << caller << ": DPPFirmwareNotSupported" << std::endl;
      break;
    case -30L: std::cout << caller << ": InvalidLicense" << std::endl;
      break;
    case -31L: std::cout << caller << ": InvalidDigitizerStatus" << std::endl;
      break;
    case -32L: std::cout << caller << ": UnsupportedTrace" << std::endl;
      break;
    case -33L: std::cout << caller << ": InvalidProbe" << std::endl;
      break;
    case -34L: std::cout << caller << ": UnsupportedBaseAddress" << std::endl;
      break;
    case -99L: std::cout << caller << ": NotYetImplemented" << std::endl;
      break;
    default: std::cout << caller << ": UnspecifiedError" << std::endl;
      break;
    }
  if (ret != CAEN_DGTZ_Success) exit(ret);
}


int main(int /*argc*/, char ** /*argv*/)
{
  int handle;
  //CAEN_DGTZ_ErrorCode ret;

  //buffers to store data
  char *buffer = NULL;
  CAEN_DGTZ_DPP_PHA_Event_t *Events[8];
  //CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=NULL;

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
  Params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List;
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
  
  CheckErrorCode(CAEN_DGTZ_OpenDigitizer(Params.LinkType,0,0,Params.VMEBaseAddress,&handle),"OpenDigitizer");
  
  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CheckErrorCode(CAEN_DGTZ_GetInfo(handle,&BoardInfo),"GetInfo");
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

  uint32_t value;

  CheckErrorCode(CAEN_DGTZ_Reset(handle),"Reset");
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8000,0x004E0115),"WriteRegister(0x8000)");
  CheckErrorCode(CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime),"SetDPPAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED),"SetAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength),"SetRecordLength");
  CheckErrorCode(CAEN_DGTZ_SetIOLevel(handle, Params.IOlev),"SetIOLevel");
  CheckErrorCode(CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT),"SetExtTriggerInputMode");
  CheckErrorCode(CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask),"SetChannelEnableMask");
  CheckErrorCode(CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0),"SetDPPEventAggregation");
  CheckErrorCode(CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled),"SetRunSynchronizationMode");
  CheckErrorCode(CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams),"SetDPPParameters");

  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x810C, &value),"ReadRegister(0x810C)");
  value |= 1UL << 30;
  value &= ~(1UL << 31);
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x810C, value),"WriteRegister(0x810C)");

  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x811C, &value),"ReadRegister(0x811C)");
  value |= 1UL << 10;
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x811C, value),"WriteRegister(0x811C");

  for (int i = 0; i < 8; i++)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_SetChannelDCOffset(handle, i, 0xCCCC),"SetChannelDCOffset");
          CheckErrorCode(CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, 1000),"SetDPPPreTriggerSize");
          CheckErrorCode(CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity),"SetChannelPulsePolarity");

          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x1080+i*0x100, &value),"ReadRegister(0x1080)");
          value |= 1UL << 24;
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1080+i*0x100, value),"WriteRegister(0x1080)");
        }
    }

  uint32_t AllocatedSize;
  CheckErrorCode(CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize),"MallocReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_MallocDPPEvents(handle, (void**)Events, &AllocatedSize),"MallocDPPEvents");
  //CheckErrorCode(CAEN_DGTZ_MallocDPPWaveforms(handle, &Waveform, &AllocatedSize),"MallocDPPWaveforms");

  // Start acquisition
  uint32_t BufferSize;
  CheckErrorCode(CAEN_DGTZ_SWStartAcquisition(handle),"SWStartAcquisition");

  for (int i_evt = 0; i_evt<100; i_evt++)
    {
      CheckErrorCode(CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize),"ReadData");
      if (BufferSize == 0) { i_evt--; continue; }
      CheckErrorCode(CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)Events, NumEvents),"GetDPPEvents");
      std::cout << "NumEvents " << NumEvents << "  i_evt " << i_evt << std::endl;
      for (int ch = 0; ch < 8; ch++)
        {
          if (!(Params.ChannelMask & (1<<ch))) continue;

          for (uint32_t ev = 0; ev < NumEvents[ch]; ev++) {
              if (Events[ch][ev].Energy > 0 && Events[ch][ev].Energy < 32768)
                  std::cout << "Channel " << ch << ", Event " << ev << ": " << Events[ch][ev].Energy << std::endl;
            }
        }
    }

  CheckErrorCode(CAEN_DGTZ_SWStopAcquisition(handle),"SWStopAcquisition");
  CheckErrorCode(CAEN_DGTZ_FreeReadoutBuffer(&buffer),"FreeReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_FreeDPPEvents(handle,(void**)Events),"FreeDPPEvents");
  //CheckErrorCode(CAEN_DGTZ_FreeDPPWaveforms(handle, Waveform),"FreeDPPWaveforms");
  CheckErrorCode(CAEN_DGTZ_CloseDigitizer(handle),"CloseDigitizer");

  
  return 0;
}
