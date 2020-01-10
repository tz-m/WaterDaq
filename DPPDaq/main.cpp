#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include "TFile.h"
#include "TH1.h"
#include "TVectorD.h"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <csignal>
#include <algorithm>

#include "date.h"

static volatile bool keep_continue = true;

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

typedef struct {
  uint32_t InputDynamicRange[8];
  uint32_t PreTriggerSize[8];
  uint32_t ChannelDCOffset[8];

  // Board Configuration Parmeters 0x8000
  uint32_t AutoDataFlush;
  uint32_t SaveDecimated;
  uint32_t TrigPropagation;
  uint32_t DualTrace;
  uint32_t AnProbe1;
  uint32_t AnProbe2;
  uint32_t WaveformRecording;
  uint32_t EnableExtras2;
  uint32_t DigVirtProbe1;
  uint32_t DigVirtProbe2;
} AdditionalChannelParams_t;

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

uint32_t ExtractBits(uint32_t value, uint32_t nbits, uint32_t startbit)
{
  // value = the value from you which you would like to extract a subset of bits
  // nbits = the number of bits you want to extract
  // startbit = the starting bit number you want. For example, if you want to start with the third bit, use startbit=2.
  return (((1 << nbits) - 1) & (value >> startbit));
}

std::string AsBinary(uint32_t value)
{
  int bits_needed = 32;
  for (int expo = 1; expo <= 32; ++expo)
    {
      if (value <= std::pow(2,expo)-1)
        {
          bits_needed = expo;
          break;
        }
    }
  std::stringstream ss; ss << "0b";
  for (int b = bits_needed; b > 0; --b)
    {
      ss << ExtractBits(value,1,b-1);
    }
  return ss.str();
}

TVectorD MakeTimeVec(std::chrono::system_clock::time_point tp)
{
  auto dp = date::floor<date::days>(tp);
  auto ymd = date::year_month_day{dp};
  auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
  TVectorD timevec(7);
  timevec[0] = (int)ymd.year();
  timevec[1] = (unsigned)ymd.month();
  timevec[2] = (unsigned)ymd.day();
  timevec[3] = time.hours().count();
  timevec[4] = time.minutes().count();
  timevec[5] = time.seconds().count();
  timevec[6] = time.subseconds().count();
  return timevec;
}

std::string MakeTimeString()
{
  auto tp = std::chrono::system_clock::now();
  auto dp = date::floor<date::days>(tp);
  auto ymd = date::year_month_day{dp};
  auto time = date::make_time(std::chrono::duration_cast<std::chrono::milliseconds>(tp-dp));
  std::stringstream s;
  s << std::setfill('0') << std::setw(4) << (int)ymd.year();
  s << std::setfill('0') << std::setw(2) << (unsigned)ymd.month();
  s << std::setfill('0') << std::setw(2) << (unsigned)ymd.day();
  s << "_";
  s << std::setfill('0') << std::setw(2) << time.hours().count();
  s << std::setfill('0') << std::setw(2) << time.minutes().count();
  s << std::setfill('0') << std::setw(2) << time.seconds().count();
  return s.str();
}

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

int main(int argc, char ** argv)
{
  long int count_events = -1;
  long int count_seconds = -1;
  if (cmdOptionExists(argv,argv+argc,"-t"))
    {
      // time the run
      count_seconds = atol(getCmdOption(argv,argv+argc,"-t"));
    }
  if (cmdOptionExists(argv,argv+argc,"-n"))
    {
      // count events
      count_events = atol(getCmdOption(argv,argv+argc,"-n"));
    }


  int handle;

  //buffers to store data
  char *buffer = NULL;
  CAEN_DGTZ_DPP_PHA_Event_t *Events[8];
  //CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=NULL;

  // digitizer configuration parameters
  CAEN_DGTZ_DPP_PHA_Params_t DPPParams;
  DigitizerParams_t Params;
  AdditionalChannelParams_t MoreChanParams;

  // set parameters
  uint32_t NumEvents[8];
  memset(&Params, 0, sizeof(DigitizerParams_t));
  memset(&DPPParams, 0, sizeof(CAEN_DGTZ_DPP_PHA_Params_t));
  memset(&MoreChanParams, 0, sizeof(AdditionalChannelParams_t));

  //communication parameters
  Params.LinkType = CAEN_DGTZ_USB;
  Params.VMEBaseAddress = 0;
  Params.IOlev = CAEN_DGTZ_IOLevel_NIM;

  //acquisition parameters
  Params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_List;
  Params.RecordLength = 10000;// Number of samples at 2ns per sample
  Params.ChannelMask = (1<<0) + (1<<1) + (0<<2) + (0<<3) + (0<<4) + (0<<5) + (0<<6) + (0<<7); // {1=enable, 0=disable} << =left.bit.shift {channel number}
  Params.EventAggr = 0;//0 = automatic
  Params.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive;

  for (int ch = 0; ch < 8; ch++)
    {
      DPPParams.thr[ch] = 50;// discriminator threshold (LSB)
      DPPParams.k[ch] = 6000;// trap rise time (ns)
      DPPParams.m[ch] = 1000;// trap flat top (ns)
      DPPParams.M[ch] = 110000;// Exponential decay time of the preamp (ns)
      DPPParams.ftd[ch] = 0.8*DPPParams.m[ch];// flat top delay ("PEAKING TIME"), 80% of flat top is a good value
      DPPParams.a[ch] = 16;// RC-CR2 smoothing factor
      DPPParams.b[ch] = 312;//input rise time (ns)
      DPPParams.trgho[ch] = 496;// Trigger hold-off
      DPPParams.nsbl[ch] = 3;// Num samples in baseline averaging
      DPPParams.nspk[ch] = 2;// peak mean 0x2 = 0b10 corresponds to 16 samples
      DPPParams.pkho[ch] = 960;// peak hold-off
      DPPParams.blho[ch] = 500;// baseline hold-off
      DPPParams.enf[ch] = 1.0;// energy normalisation factor
      DPPParams.decimation[ch] = 0;
      DPPParams.dgain[ch] = 0;
      DPPParams.otrej[ch] = 0;
      DPPParams.trgwin[ch] = 1;// enable/disable rise time discriminator
      DPPParams.twwdt[ch] = 312;// rise time validation window
      MoreChanParams.InputDynamicRange[ch] = 0;
      MoreChanParams.PreTriggerSize[ch] = 1000;
      MoreChanParams.ChannelDCOffset[ch] = (1-0.2)*0xFFFF;// use formula (1 - percent_offset)*0xFFFF where percent_offset is the place you want the baseline within the full range, e.g. 0.2 for 20%
    }

  MoreChanParams.AutoDataFlush = 1;
  MoreChanParams.SaveDecimated = 0;
  MoreChanParams.TrigPropagation = 1;
  MoreChanParams.DualTrace = 0;
  MoreChanParams.AnProbe1 = 0;
  MoreChanParams.AnProbe2 = 0;
  MoreChanParams.WaveformRecording = 0;
  MoreChanParams.EnableExtras2 = 0;
  MoreChanParams.DigVirtProbe1 = 0;
  MoreChanParams.DigVirtProbe2 = 0;
  std::bitset<32> boardcfg(0);
  boardcfg |= MoreChanParams.AutoDataFlush << 0;
  boardcfg |= MoreChanParams.SaveDecimated << 1;
  boardcfg |= MoreChanParams.TrigPropagation << 2;
  boardcfg |= 1 << 4;// required
  boardcfg |= 1 << 8;// required
  boardcfg |= MoreChanParams.DualTrace << 11;
  boardcfg |= MoreChanParams.AnProbe1 << 12;
  boardcfg |= MoreChanParams.AnProbe2 << 14;
  boardcfg |= MoreChanParams.WaveformRecording << 16;
  boardcfg |= MoreChanParams.EnableExtras2 << 17;
  boardcfg |= 1 << 18;// required
  boardcfg |= 1 << 19;// required
  boardcfg |= MoreChanParams.DigVirtProbe1 << 20;
  boardcfg |= MoreChanParams.DigVirtProbe2 << 26;
  std::cout << "Board Configuration: " << boardcfg << " (" << std::hex << boardcfg.to_ulong() << std::dec << ")" << std::endl;


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

  uint32_t value;

  ////////////////////////////////
  //Write configuration to board//
  ////////////////////////////////

  CheckErrorCode(CAEN_DGTZ_Reset(handle),"Reset");
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8000,boardcfg.to_ulong()),"SetBoardConfiguration");
  CheckErrorCode(CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime),"SetDPPAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED),"SetAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength),"SetRecordLength");//This value is Ns (number of samples, at 2ns per sample. So Ns=10k is 20us)
  CheckErrorCode(CAEN_DGTZ_SetIOLevel(handle, Params.IOlev),"SetIOLevel");
  CheckErrorCode(CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT),"SetExtTriggerInputMode");
  CheckErrorCode(CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask),"SetChannelEnableMask");
  CheckErrorCode(CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled),"SetRunSynchronizationMode");
  CheckErrorCode(CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams),"SetDPPParameters");

  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x810C, &value),"ReadRegister(0x810C)");
  value |= 1UL << 30;// Enable external trigger
  value &= ~(1UL << 31);// Disable software trigger
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x810C, value),"WriteRegister(0x810C)");

  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x811C, &value),"ReadRegister(0x811C)");
  value |= 1UL << 10;// Trigger is synchronized with the whole duration of the TRG-IN signal
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x811C, value),"WriteRegister(0x811C");

  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x817C, 0),"WriteEnableExternalTrigger");

  for (int i = 0; i < 8; i++)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_SetChannelDCOffset(handle, i, MoreChanParams.ChannelDCOffset[i]),"SetChannelDCOffset");
          CheckErrorCode(CAEN_DGTZ_SetDPPPreTriggerSize(handle, i, MoreChanParams.PreTriggerSize[i]),"SetDPPPreTriggerSize");
          CheckErrorCode(CAEN_DGTZ_SetChannelPulsePolarity(handle, i, Params.PulsePolarity),"SetChannelPulsePolarity");

          // Disable self trigger (set bit 24 to 1)
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x1080+i*0x100, &value),"ReadRegister(0x1080)");
          value |= 1UL << 24;
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1080+i*0x100, value),"WriteRegister(0x1080)");

          // set input dynamic range
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1028+i*0x100, MoreChanParams.InputDynamicRange[i]),"SetInputDynamicRange");
        }
    }
  CheckErrorCode(CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0),"SetDPPEventAggregation");

  uint32_t AllocatedSize;
  CheckErrorCode(CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize),"MallocReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_MallocDPPEvents(handle, (void**)Events, &AllocatedSize),"MallocDPPEvents");
  //CheckErrorCode(CAEN_DGTZ_MallocDPPWaveforms(handle, &Waveform, &AllocatedSize),"MallocDPPWaveforms");


  /*Check All Parameters*/

  //Global
  CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value),"GetRecordLength");
  std::cout << "GetRecordLength: " << value << std::endl;
  CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value),"GetNumEventsPerAggregate");
  std::cout << "GetNumEventsPerAggregate: " << value << std::endl;

  //Channel
  for (int i = 0; i < 8; i++)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value,i),"GetRecordLengthChannelI");
          std::cout << "Ch" << i << " Record Length; " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1028+i*0x100,&value),"ReadChannelIInputDynamicRange");
          std::cout << "Ch" << i << " Input Dynamic Range: " << ((value==0)?"2 Vpp":((value==1)?"0.5 Vpp":"Invalid")) << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value,i),"GetNumEventsPerAggregateChannelI");
          std::cout << "Ch" << i << " NumEventsPerAggregate: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetDPPPreTriggerSize(handle,i,&value),"GetDPPPreTriggerSize");
          std::cout << "Ch" << i << " PreTrigger: " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x104C+i*0x100,&value),"ReadFineGainChannelI");
          std::cout << "Ch" << i << " Fine Gain: " << value << " (if =250, fg is probably 1.0)" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1054+i*0x100,&value),"ReadRC-CR2SmootingFactorChannelI");
          std::cout << "Ch" << i << " RC-CR2 Smoothing Factor: " << std::hex << value << std::dec << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1058+i*0x100,&value),"ReadInputRiseTimeChannelI");
          std::cout << "Ch" << i << " Input Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x105C+i*0x100,&value),"ReadTrapRiseTimeChannelI");
          std::cout << "Ch" << i << " Trap Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1060+i*0x100,&value),"ReadTrapFlatTopChannelI");
          std::cout << "Ch" << i << " Trap Flat Top: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1064+i*0x100,&value),"ReadPeakingTimeChannelI");
          std::cout << "Ch" << i << " Peaking Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1068+i*0x100,&value),"ReadDecayTimeChannelI");
          std::cout << "Ch" << i << " Decay Time: " << value*8/1000 << " microseconds" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x106C+i*0x100,&value),"ReadTriggerThresholdChannelI");
          std::cout << "Ch" << i << " Trig Threshold: " << value << " LSB (threshold in mV = {LSB}*Vpp/ADC_Nbits)" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1070+i*0x100,&value),"ReadRiseTimeValidationWindowChannelI");
          std::cout << "Ch" << i << " Rise Time Validation Window: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1074+i*0x100,&value),"ReadTriggerHoldOffChannelI");
          std::cout << "Ch" << i << " Trigger Hold-Off: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1078+i*0x100,&value),"ReadPeakHoldOffChannelI");
          std::cout << "Ch" << i << " Peak Hold-Off: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1080+i*0x100,&value),"ReadDPPAlgControlChannelI");
          std::cout << "Ch" << i << " DPPAlgControl: 0x" << std::hex << value << std::dec << "  " << AsBinary(value) << std::endl;
          std::cout << "    Trapezoid Rescaling: " << ExtractBits(value,6,0) << std::endl;
          std::cout << "    Decimation: " << AsBinary(ExtractBits(value,2,8)) << std::endl;
          std::cout << "    Decimation Gain: " << AsBinary(ExtractBits(value,2,10)) << std::endl;
          std::cout << "    Peak Mean: " << AsBinary(ExtractBits(value,2,12)) << std::endl;
          std::cout << "    Invert Input: " << ExtractBits(value,1,16) << std::endl;
          std::cout << "    Trigger Mode: " << AsBinary(ExtractBits(value,2,18)) << std::endl;
          std::cout << "    Baseline averaging window: " << AsBinary(ExtractBits(value,3,20)) << std::endl;
          std::cout << "    Disable Self Trigger: " << ExtractBits(value,1,24) << std::endl;
          std::cout << "    Enable Roll-over: " << ExtractBits(value,1,26) << std::endl;
          std::cout << "    Enable Pile-up: " << ExtractBits(value,1,27) << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1084+i*0x100,&value),"ReadShapedTrigWidthChannelI");
          std::cout << "Ch" << i << " Shaped Trig Width: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1088+i*0x100,&value),"ReadChannelIStatus");
          std::cout << "Ch" << i << " Status: " << "SPI_busy=" << ExtractBits(value,1,2) << " ADC_calib_done=" << ExtractBits(value,1,3) << " ADC_power_down=" << ExtractBits(value,1,8) << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetChannelDCOffset(handle,i,&value),"GetChannelDCOffset");
          std::cout << "Ch" << i << " DC Offset: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x10A0+i*0x100,&value),"ReadDPPAlgControl2ChannelI");
          std::cout << "Ch" << i << " DPPAlgControl2: 0x" << std::hex << value << std::dec << "  " << AsBinary(value) << std::endl;
          std::cout << "    Local Shaped Trigger Mode: " << AsBinary(ExtractBits(value,2,0)) << std::endl;
          std::cout << "    Enable Local Shaped Trigger: " << ExtractBits(value,1,2) << std::endl;
          std::cout << "    Local Trigger Validation Mode: " << AsBinary(ExtractBits(value,2,4)) << std::endl;
          std::cout << "    Enable Local Trigger Validation: " << ExtractBits(value,1,6) << std::endl;
          std::cout << "    Extras2 options: " << AsBinary(ExtractBits(value,3,8)) << std::endl;
          std::cout << "    Veto Source: " << AsBinary(ExtractBits(value,2,14)) << std::endl;
          std::cout << "    Trigger Counter Rate Step: " << AsBinary(ExtractBits(value,2,16)) << std::endl;
          std::cout << "    Baseline Calculation Always: " << AsBinary(ExtractBits(value,1,18)) << std::endl;
          std::cout << "    Tag corr/uncorr: " << ExtractBits(value,1,19) << std::endl;
          std::cout << "    BLR Optimization: " << ExtractBits(value,1,29) << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadTemperature(handle,i,&value),"ReadTemperature");
          std::cout << "Ch" << i << " Temperature: " << value << " degC" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x10D4+i*0x100,&value),"ReadVetoWidthChannelI");
          std::cout << "Ch" << i << " Veto Width: " << ExtractBits(value,16,0) << " in steps of " << AsBinary(ExtractBits(value,2,16)) << std::endl;
        }
    }
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8000,&value),"ReadBoardConfiguration");
  std::cout << "Read board configuration " << AsBinary(value) << std::endl;
  std::cout << "   AutoDataFlush=" << ExtractBits(value,1,0) << std::endl;
  std::cout << "   SaveDecimated=" << ExtractBits(value,1,1) << std::endl;
  std::cout << "   TrigPropagation=" << ExtractBits(value,1,2) << std::endl;
  std::cout << "   DualTrace=" << ExtractBits(value,1,11) << std::endl;
  std::cout << "   AnProbe1=" << ExtractBits(value,2,12) << std::endl;
  std::cout << "   AnProbe2=" << ExtractBits(value,2,14) << std::endl;
  std::cout << "   WaveformRecording=" << ExtractBits(value,1,16) << std::endl;
  std::cout << "   EnableExtras2=" << ExtractBits(value,1,17) << std::endl;
  std::cout << "   DigVirtProbe1=" << ExtractBits(value,4,20) << std::endl;
  std::cout << "   DigVirtProbe2=" << ExtractBits(value,3,26) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x800C,&value),"ReadAggregateOrganisation");
  std::cout << "Aggregate Organisation: 0x" << std::hex << value << std::dec << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8100,&value),"ReadAcquisitionControl");
  std::cout << "Acquisition Control: 0x" << std::hex << value << std::dec << std::endl;
  std::cout << "    Start/Stop Mode: " << AsBinary(ExtractBits(value,2,0)) << std::endl;
  std::cout << "    Acquisition Start/Arm: " << ExtractBits(value,1,2) << std::endl;
  std::cout << "    PLL Reference Clock Source: " << ExtractBits(value,1,6) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8104,&value),"CheckAcquisitionStatus");
  std::cout << "Acquisition Status: " << AsBinary(value) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x810C,&value),"ReadGlobalTriggerMask");
  std::cout << "Global Trigger Mask: 0x" << std::hex << value << std::dec << " (" << AsBinary(value) << ")" << std::endl;
  std::cout << "    Couples Contribute To Global Trigger: " << AsBinary(ExtractBits(value,4,0)) << std::endl;
  std::cout << "    Majority Coincidence Window: " << ExtractBits(value,4,20)*8 << " ns" << std::endl;
  std::cout << "    Majority Level: " << ExtractBits(value,3,24) << std::endl;
  std::cout << "    External Trigger Enabled: " << ExtractBits(value,1,30) << std::endl;
  std::cout << "    Software Trigger Enabled: " << ExtractBits(value,1,31) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x811C,&value),"ReadFrontPanelIOControl");
  std::cout << "Front Panel I/O Control: 0x" << std::hex << value << std::dec << std::endl;
  std::cout << "    LEMO I/O Level: " << (ExtractBits(value,1,0)==0?"NIM":(ExtractBits(value,1,0)==1?"TTL":"Unknown")) << std::endl;
  std::cout << "    TRG-IN Control: " << ExtractBits(value,1,10) << std::endl;
  std::cout << "    TRG-IN to Mezzanine: " << ExtractBits(value,1,11) << std::endl;
  std::cout << "    Force GPO: " << ExtractBits(value,1,14) << std::endl;
  std::cout << "    GPO Mode: " << ExtractBits(value,1,15) << std::endl;
  std::cout << "    GPO Mode Selection: " << AsBinary(ExtractBits(value,2,16)) << std::endl;
  std::cout << "    Motherboard Virtual Probe Selection to GPO: " << AsBinary(ExtractBits(value,2,18)) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8120,&value),"ReadChannelEnableMask");
  std::cout << "Channel Enable Mask: " << AsBinary(value) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x817C,&value),"ReadDisableExternalTrigger");
  std::cout << "Disable External Trigger: " << value << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8180,&value),"ReadTriggerValidationMaskCouple0");
  std::cout << "Trigger Validation Mask Couple 0: 0x" << std::hex << value << std::dec << " (" << AsBinary(value) << ")" << std::endl;
  std::cout << "    Couples which participate in trigger generation: " << AsBinary(ExtractBits(value,8,0)) << std::endl;
  std::cout << "    Operation Mask: " << AsBinary(ExtractBits(value,2,8)) << std::endl;
  std::cout << "    Majority Level: " << ExtractBits(value,3,10) << std::endl;
  std::cout << "    External Trigger: " << ExtractBits(value,1,30) << std::endl;
  std::cout << "    Software Trigger: " << ExtractBits(value,1,31) << std::endl;

  // Calibrate ADCs
  for (int ch = 0; ch < 8; ch++)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1088+ch*0x100,&value),"CheckChannelIStatus");
      while (ExtractBits(value,1,2))
        {
          std::cout << "Channel " << ch << " busy, wait 5 seconds and retry ADC calibration..." << std::endl;
          sleep(5);
        }
      CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x809C,1),"ADCCalibrate");
      CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1088,&value),"CheckChannelIADCCalibrationStatus");
      while (!ExtractBits(value,1,3)) CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1088,&value),"PollChannelIADCCalibrationStatus");
      std::cout << "Finished Calibrating ADC Channel " << ch << std::endl;
    }

  std::stringstream filename;
  filename << "DPPDaq_" << MakeTimeString() << ".root";
  std::cout << filename.str() << std::endl;
  TFile * fout = TFile::Open(filename.str().c_str(),"RECREATE");
  TH1I * h_ch0 = new TH1I("h_ch0",";ADC Channel;",16384,0,16384);
  TH1I * h_ch1 = new TH1I("h_ch1",";ADC Channel;",16384,0,16384);

  // Start acquisition
  uint32_t BufferSize;

  std::signal(SIGINT, [](int) { keep_continue = false; });

  CheckErrorCode(CAEN_DGTZ_SWStartAcquisition(handle),"SWStartAcquisition");

  auto start_tp = std::chrono::system_clock::now();
  TVectorD starttimevec = MakeTimeVec(start_tp);

  int i_evt = 0;
  while(keep_continue)
    {
      CheckErrorCode(CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize),"ReadData");
      if (BufferSize == 0) continue;
      CheckErrorCode(CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)Events, NumEvents),"GetDPPEvents");
      std::cout << "NumEvents " << NumEvents << "  i_evt " << i_evt << std::endl;
      for (int ch = 0; ch < 8; ch++)
        {
          if (!(Params.ChannelMask & (1<<ch))) continue;

          for (uint32_t ev = 0; ev < NumEvents[ch]; ev++) {
              uint32_t energy = Events[ch][ev].Energy;
              if (energy > 10 && energy < 32768)
                {
                  //std::cout << "Channel " << ch << ", Event " << ev << ": " << energy << std::endl;
                  if (ch==0) h_ch0->Fill(energy);
                  if (ch==1) h_ch1->Fill(energy);
                }
            }
        }
      ++i_evt;

      if (count_events > 0 && i_evt > count_events) break;
      auto now_tp = std::chrono::system_clock::now();
      if (count_seconds > 0 && std::chrono::duration_cast<std::chrono::seconds>(now_tp-start_tp).count() > count_seconds) break;
    }

  auto end_tp = std::chrono::system_clock::now();
  TVectorD endtimevec = MakeTimeVec(end_tp);

  starttimevec.Write("starttime");
  endtimevec.Write("endtime");
  h_ch0->Write();
  h_ch1->Write();
  fout->Close();

  CheckErrorCode(CAEN_DGTZ_SWStopAcquisition(handle),"SWStopAcquisition");
  CheckErrorCode(CAEN_DGTZ_FreeReadoutBuffer(&buffer),"FreeReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_FreeDPPEvents(handle,(void**)Events),"FreeDPPEvents");
  //CheckErrorCode(CAEN_DGTZ_FreeDPPWaveforms(handle, Waveform),"FreeDPPWaveforms");
  CheckErrorCode(CAEN_DGTZ_CloseDigitizer(handle),"CloseDigitizer");


  return 0;
}
