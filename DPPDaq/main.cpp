#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"

#include "TFile.h"
#include "TH1.h"
#include "TVectorD.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TPad.h"
#include "TROOT.h"
#include "TGraph.h"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <csignal>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <memory>

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
  TApplication tapp("tapp",&argc,argv);

  long int count_events = -1;
  long int count_seconds = -1;
  bool disp = false;
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
  if (cmdOptionExists(argv,argv+argc,"-disp")) disp = true;

  std::string thisTimeString = MakeTimeString();

  std::stringstream configfilename;
  configfilename << "config_DPPDaq_" << thisTimeString << ".txt";
  std::cout << "Config filename " << configfilename.str() << std::endl;
  std::ofstream fs(configfilename.str());

  int handle;

  //buffers to store data
  char *buffer = NULL;
  CAEN_DGTZ_DPP_PHA_Event_t *Events[8];
  CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=NULL;

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
  if (disp) Params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
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
  MoreChanParams.DualTrace = (disp)?1:0;
  MoreChanParams.AnProbe1 = (disp)?0:0;// 0=input, 1=RC-CR, 2=RC-CR2, 3=trapezoid
  MoreChanParams.AnProbe2 = (disp)?2:0;// 0=input, 1=threshold, 2=trap-base, 3=baseline
  MoreChanParams.WaveformRecording = (disp)?1:0;
  MoreChanParams.EnableExtras2 = 0;
  MoreChanParams.DigVirtProbe1 = (disp)?0:0;// 0=peaking, 3=pileup, 5=trig valid window, 7=trig holdoff, 8=trig validation, 10=zero cross window, 11=ext trig, 12=busy (other options available)
  MoreChanParams.DigVirtProbe2 = 0;// 0=Trigger
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
  fs << "Board Configuration: " << boardcfg << " (" << std::hex << boardcfg.to_ulong() << std::dec << ")" << std::endl;


  CheckErrorCode(CAEN_DGTZ_OpenDigitizer(Params.LinkType,0,0,Params.VMEBaseAddress,&handle),"OpenDigitizer");

  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CheckErrorCode(CAEN_DGTZ_GetInfo(handle,&BoardInfo),"GetInfo");
  fs << "  ModelName: " << BoardInfo.ModelName << std::endl;
  fs << "  Model: " << BoardInfo.Model << std::endl;
  fs << "  Channels: " << BoardInfo.Channels << std::endl;
  fs << "  FormFactor: " << BoardInfo.FormFactor << std::endl;
  fs << "  FamilyCode: " << BoardInfo.FamilyCode << std::endl;
  fs << "  ROC_FirmwareRel: " << BoardInfo.ROC_FirmwareRel << std::endl;
  fs << "  AMC_FirmwareRel: " << BoardInfo.AMC_FirmwareRel << std::endl;
  fs << "  SerialNumber: " << BoardInfo.SerialNumber << std::endl;
  fs << "  PCB_Revision: " << BoardInfo.PCB_Revision << std::endl;
  fs << "  ADC_NBits: " << BoardInfo.ADC_NBits << std::endl;
  fs << "  CommHandle: " << BoardInfo.CommHandle << std::endl;
  fs << "  VMEHandle: " << BoardInfo.VMEHandle << std::endl;
  fs << "  License: " << BoardInfo.License << std::endl;

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

          // Enable BLR optimization
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x10A0+i*0x100, &value),"ReadDPPAlg2");
          value |= 1UL << 29;
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x10A0+i*0x100, value),"WriteDPPAlg2");

          // set input dynamic range
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1028+i*0x100, MoreChanParams.InputDynamicRange[i]),"SetInputDynamicRange");
        }
    }
  CheckErrorCode(CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0),"SetDPPEventAggregation");

  uint32_t AllocatedSize;
  CheckErrorCode(CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize),"MallocReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_MallocDPPEvents(handle, (void**)Events, &AllocatedSize),"MallocDPPEvents");
  CheckErrorCode(CAEN_DGTZ_MallocDPPWaveforms(handle, (void**)&Waveform, &AllocatedSize),"MallocDPPWaveforms");


  /*Check All Parameters*/

  //Global
  CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value),"GetRecordLength");
  fs << "GetRecordLength: " << value << std::endl;
  CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value),"GetNumEventsPerAggregate");
  fs << "GetNumEventsPerAggregate: " << value << std::endl;

  //Channel
  for (int i = 0; i < 8; i++)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value,i),"GetRecordLengthChannelI");
          fs << "Ch" << i << " Record Length; " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1028+i*0x100,&value),"ReadChannelIInputDynamicRange");
          fs << "Ch" << i << " Input Dynamic Range: " << ((value==0)?"2 Vpp":((value==1)?"0.5 Vpp":"Invalid")) << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value,i),"GetNumEventsPerAggregateChannelI");
          fs << "Ch" << i << " NumEventsPerAggregate: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetDPPPreTriggerSize(handle,i,&value),"GetDPPPreTriggerSize");
          fs << "Ch" << i << " PreTrigger: " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x104C+i*0x100,&value),"ReadFineGainChannelI");
          fs << "Ch" << i << " Fine Gain: " << value << " (if =250, fg is probably 1.0)" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1054+i*0x100,&value),"ReadRC-CR2SmootingFactorChannelI");
          fs << "Ch" << i << " RC-CR2 Smoothing Factor: " << std::hex << value << std::dec << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1058+i*0x100,&value),"ReadInputRiseTimeChannelI");
          fs << "Ch" << i << " Input Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x105C+i*0x100,&value),"ReadTrapRiseTimeChannelI");
          fs << "Ch" << i << " Trap Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1060+i*0x100,&value),"ReadTrapFlatTopChannelI");
          fs << "Ch" << i << " Trap Flat Top: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1064+i*0x100,&value),"ReadPeakingTimeChannelI");
          fs << "Ch" << i << " Peaking Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1068+i*0x100,&value),"ReadDecayTimeChannelI");
          fs << "Ch" << i << " Decay Time: " << value*8/1000 << " microseconds" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x106C+i*0x100,&value),"ReadTriggerThresholdChannelI");
          fs << "Ch" << i << " Trig Threshold: " << value << " LSB (threshold in mV = {LSB}*Vpp/ADC_Nbits)" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1070+i*0x100,&value),"ReadRiseTimeValidationWindowChannelI");
          fs << "Ch" << i << " Rise Time Validation Window: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1074+i*0x100,&value),"ReadTriggerHoldOffChannelI");
          fs << "Ch" << i << " Trigger Hold-Off: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1078+i*0x100,&value),"ReadPeakHoldOffChannelI");
          fs << "Ch" << i << " Peak Hold-Off: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1080+i*0x100,&value),"ReadDPPAlgControlChannelI");
          fs << "Ch" << i << " DPPAlgControl: 0x" << std::hex << value << std::dec << "  " << AsBinary(value) << std::endl;
          fs << "    Trapezoid Rescaling: " << ExtractBits(value,6,0) << std::endl;
          fs << "    Decimation: " << AsBinary(ExtractBits(value,2,8)) << std::endl;
          fs << "    Decimation Gain: " << AsBinary(ExtractBits(value,2,10)) << std::endl;
          fs << "    Peak Mean: " << AsBinary(ExtractBits(value,2,12)) << std::endl;
          fs << "    Invert Input: " << ExtractBits(value,1,16) << std::endl;
          fs << "    Trigger Mode: " << AsBinary(ExtractBits(value,2,18)) << std::endl;
          fs << "    Baseline averaging window: " << AsBinary(ExtractBits(value,3,20)) << std::endl;
          fs << "    Disable Self Trigger: " << ExtractBits(value,1,24) << std::endl;
          fs << "    Enable Roll-over: " << ExtractBits(value,1,26) << std::endl;
          fs << "    Enable Pile-up: " << ExtractBits(value,1,27) << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1084+i*0x100,&value),"ReadShapedTrigWidthChannelI");
          fs << "Ch" << i << " Shaped Trig Width: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1088+i*0x100,&value),"ReadChannelIStatus");
          fs << "Ch" << i << " Status: " << "SPI_busy=" << ExtractBits(value,1,2) << " ADC_calib_done=" << ExtractBits(value,1,3) << " ADC_power_down=" << ExtractBits(value,1,8) << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetChannelDCOffset(handle,i,&value),"GetChannelDCOffset");
          fs << "Ch" << i << " DC Offset: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x10A0+i*0x100,&value),"ReadDPPAlgControl2ChannelI");
          fs << "Ch" << i << " DPPAlgControl2: 0x" << std::hex << value << std::dec << "  " << AsBinary(value) << std::endl;
          fs << "    Local Shaped Trigger Mode: " << AsBinary(ExtractBits(value,2,0)) << std::endl;
          fs << "    Enable Local Shaped Trigger: " << ExtractBits(value,1,2) << std::endl;
          fs << "    Local Trigger Validation Mode: " << AsBinary(ExtractBits(value,2,4)) << std::endl;
          fs << "    Enable Local Trigger Validation: " << ExtractBits(value,1,6) << std::endl;
          fs << "    Extras2 options: " << AsBinary(ExtractBits(value,3,8)) << std::endl;
          fs << "    Veto Source: " << AsBinary(ExtractBits(value,2,14)) << std::endl;
          fs << "    Trigger Counter Rate Step: " << AsBinary(ExtractBits(value,2,16)) << std::endl;
          fs << "    Baseline Calculation Always: " << AsBinary(ExtractBits(value,1,18)) << std::endl;
          fs << "    Tag corr/uncorr: " << ExtractBits(value,1,19) << std::endl;
          fs << "    BLR Optimization: " << ExtractBits(value,1,29) << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadTemperature(handle,i,&value),"ReadTemperature");
          fs << "Ch" << i << " Temperature: " << value << " degC" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x10D4+i*0x100,&value),"ReadVetoWidthChannelI");
          fs << "Ch" << i << " Veto Width: " << ExtractBits(value,16,0) << " in steps of " << AsBinary(ExtractBits(value,2,16)) << std::endl;
        }
    }
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8000,&value),"ReadBoardConfiguration");
  fs << "Read board configuration " << AsBinary(value) << std::endl;
  fs << "   AutoDataFlush=" << ExtractBits(value,1,0) << std::endl;
  fs << "   SaveDecimated=" << ExtractBits(value,1,1) << std::endl;
  fs << "   TrigPropagation=" << ExtractBits(value,1,2) << std::endl;
  fs << "   DualTrace=" << ExtractBits(value,1,11) << std::endl;
  fs << "   AnProbe1=" << ExtractBits(value,2,12) << std::endl;
  fs << "   AnProbe2=" << ExtractBits(value,2,14) << std::endl;
  fs << "   WaveformRecording=" << ExtractBits(value,1,16) << std::endl;
  fs << "   EnableExtras2=" << ExtractBits(value,1,17) << std::endl;
  fs << "   DigVirtProbe1=" << ExtractBits(value,4,20) << std::endl;
  fs << "   DigVirtProbe2=" << ExtractBits(value,3,26) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x800C,&value),"ReadAggregateOrganisation");
  fs << "Aggregate Organisation: 0x" << std::hex << value << std::dec << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8100,&value),"ReadAcquisitionControl");
  fs << "Acquisition Control: 0x" << std::hex << value << std::dec << std::endl;
  fs << "    Start/Stop Mode: " << AsBinary(ExtractBits(value,2,0)) << std::endl;
  fs << "    Acquisition Start/Arm: " << ExtractBits(value,1,2) << std::endl;
  fs << "    PLL Reference Clock Source: " << ExtractBits(value,1,6) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8104,&value),"CheckAcquisitionStatus");
  fs << "Acquisition Status: " << AsBinary(value) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x810C,&value),"ReadGlobalTriggerMask");
  fs << "Global Trigger Mask: 0x" << std::hex << value << std::dec << " (" << AsBinary(value) << ")" << std::endl;
  fs << "    Couples Contribute To Global Trigger: " << AsBinary(ExtractBits(value,4,0)) << std::endl;
  fs << "    Majority Coincidence Window: " << ExtractBits(value,4,20)*8 << " ns" << std::endl;
  fs << "    Majority Level: " << ExtractBits(value,3,24) << std::endl;
  fs << "    External Trigger Enabled: " << ExtractBits(value,1,30) << std::endl;
  fs << "    Software Trigger Enabled: " << ExtractBits(value,1,31) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x811C,&value),"ReadFrontPanelIOControl");
  fs << "Front Panel I/O Control: 0x" << std::hex << value << std::dec << std::endl;
  fs << "    LEMO I/O Level: " << (ExtractBits(value,1,0)==0?"NIM":(ExtractBits(value,1,0)==1?"TTL":"Unknown")) << std::endl;
  fs << "    TRG-IN Control: " << ExtractBits(value,1,10) << std::endl;
  fs << "    TRG-IN to Mezzanine: " << ExtractBits(value,1,11) << std::endl;
  fs << "    Force GPO: " << ExtractBits(value,1,14) << std::endl;
  fs << "    GPO Mode: " << ExtractBits(value,1,15) << std::endl;
  fs << "    GPO Mode Selection: " << AsBinary(ExtractBits(value,2,16)) << std::endl;
  fs << "    Motherboard Virtual Probe Selection to GPO: " << AsBinary(ExtractBits(value,2,18)) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8120,&value),"ReadChannelEnableMask");
  fs << "Channel Enable Mask: " << AsBinary(value) << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x817C,&value),"ReadDisableExternalTrigger");
  fs << "Disable External Trigger: " << value << std::endl;
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8180,&value),"ReadTriggerValidationMaskCouple0");
  fs << "Trigger Validation Mask Couple 0: 0x" << std::hex << value << std::dec << " (" << AsBinary(value) << ")" << std::endl;
  fs << "    Couples which participate in trigger generation: " << AsBinary(ExtractBits(value,8,0)) << std::endl;
  fs << "    Operation Mask: " << AsBinary(ExtractBits(value,2,8)) << std::endl;
  fs << "    Majority Level: " << ExtractBits(value,3,10) << std::endl;
  fs << "    External Trigger: " << ExtractBits(value,1,30) << std::endl;
  fs << "    Software Trigger: " << ExtractBits(value,1,31) << std::endl;

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
    }
  std::cout << "ADCs Calibrated..." << std::endl;

  std::stringstream filename;
  filename << "DPPDaq_" << thisTimeString << ".root";
  std::cout << "Output filename " << filename.str() << std::endl;
  TFile * fout = TFile::Open(filename.str().c_str(),"RECREATE");
  TH1I * h_ch0 = new TH1I("h_ch0",";ADC Channel;",16384,0,16384);
  TH1I * h_ch1 = new TH1I("h_ch1",";ADC Channel;",16384,0,16384);

  // Start acquisition
  uint32_t BufferSize;

  std::signal(SIGINT, [](int) { keep_continue = false; });

  CheckErrorCode(CAEN_DGTZ_SWStartAcquisition(handle),"SWStartAcquisition");

  auto start_tp = std::chrono::system_clock::now();
  TVectorD starttimevec = MakeTimeVec(start_tp);
  auto PrevRateTime = start_tp;

  int i_evt = 0;
  int i_sec = 0;
  std::unordered_map<int,int> chan_pulses;
  std::unordered_map<int,int> trgCnt;
  std::unordered_map<int,int> purCnt;
  int Nb = 0;

  std::unordered_map<int,TCanvas*> canv_wf_map;
  std::unordered_map<int,TCanvas*> canv_hist_map;
  for (int ch = 0; ch < 8; ch++)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      canv_wf_map[ch] = new TCanvas(((std::string)"wf_ch"+std::to_string(ch)).c_str(),"",1600,900);
      //canv_hist_map[ch] = new TCanvas(((std::string)"hist_ch"+std::to_string(ch)).c_str(),"",1600,900);
    }

  while(keep_continue)
    {
      CheckErrorCode(CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize),"ReadData");
      if (BufferSize == 0) continue;
      Nb += BufferSize;
      CheckErrorCode(CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, (void**)Events, NumEvents),"GetDPPEvents");
      for (int ch = 0; ch < 8; ch++)
        {
          if (!(Params.ChannelMask & (1<<ch))) continue;

          for (uint32_t ev = 0; ev < NumEvents[ch]; ev++)
            {
              trgCnt[ch]++;
              uint32_t energy = Events[ch][ev].Energy;
              if (energy > 10 && energy < 32768)
                {
                  if (ch==0) h_ch0->Fill(energy);
                  if (ch==1) h_ch1->Fill(energy);
                  chan_pulses[ch]++;
                }
              else
                {
                  purCnt[ch]++;
                }
              if (disp && ev == 0 && energy > 10 && energy < 32768)
                {
                  int size;
                  int *time_x;
                  int16_t *WaveLine_an1; Int_t *WL_an1;
                  int16_t *WaveLine_an2; Int_t *WL_an2;
                  uint8_t *DigitalWaveLine_d1; Int_t *DWL_d1;
                  uint8_t *DigitalWaveLine_d2; Int_t *DWL_d2;
                  CheckErrorCode(CAEN_DGTZ_DecodeDPPWaveforms(handle, &Events[ch][ev], Waveform),"DecodeDPPWaveforms");

                  // Use waveform data here...
                  size = (int)(Waveform->Ns); // Number of samples

                  time_x = (Int_t*)malloc(sizeof(Int_t)*size);
                  WL_an1 = (Int_t*)malloc(sizeof(Int_t)*size);
                  WL_an2 = (Int_t*)malloc(sizeof(Int_t)*size);
                  DWL_d1 = (Int_t*)malloc(sizeof(Int_t)*size);
                  DWL_d2 = (Int_t*)malloc(sizeof(Int_t)*size);

                  WaveLine_an1 = Waveform->Trace1; // First trace (ANALOG_TRACE_1)
                  WaveLine_an2 = Waveform->Trace2; // Second Trace ANALOG_TRACE_2 (if single trace mode, it is a sequence of zeroes)
                  DigitalWaveLine_d1 = Waveform->DTrace1; // First Digital Trace (DIGITALPROBE1)
                  DigitalWaveLine_d2 = Waveform->DTrace2; // Second Digital Trace (for DPP-PHA it is ALWAYS Trigger)

                  for (int pt = 0; pt < size; ++pt)
                    {
                      time_x[pt] = pt*2;
                      WL_an1[pt] = static_cast<Int_t>(WaveLine_an1[pt]);
                      WL_an2[pt] = static_cast<Int_t>(WaveLine_an2[pt]);
                      DWL_d1[pt] = static_cast<Int_t>(DigitalWaveLine_d1[pt]);
                      DWL_d2[pt] = static_cast<Int_t>(DigitalWaveLine_d2[pt]);
                    }

                  canv_wf_map[ch]->cd();

                  TGraph * an1 = new TGraph(size,time_x,WL_an1);
                  an1->SetTitle(((std::string)"Channel"+std::to_string(ch)+";Time (ns);ADC Value").c_str());
                  an1->SetLineColor(kBlack);
                  an1->GetYaxis()->SetRangeUser(-8192,16384);
                  TGraph * an2 = new TGraph(size,time_x,WL_an2);
                  an2->SetTitle("");
                  an2->SetLineColor(kRed);
                  an2->GetXaxis()->SetLabelSize(0);
                  an2->GetXaxis()->SetTickLength(0);
                  an2->GetYaxis()->SetLabelSize(0);
                  an2->GetYaxis()->SetTickLength(0);
                  an2->GetYaxis()->SetRangeUser(-8192,16384);
                  TGraph * d1 = new TGraph(size,time_x,DWL_d1);
                  d1->SetTitle("");
                  d1->SetLineColor(kBlue);
                  d1->GetXaxis()->SetLabelSize(0);
                  d1->GetXaxis()->SetTickLength(0);
                  d1->GetYaxis()->SetLabelSize(0);
                  d1->GetYaxis()->SetTickLength(0);
                  d1->GetYaxis()->SetRangeUser(-1,3);
                  TGraph * d2 = new TGraph(size,time_x,DWL_d2);
                  d2->SetTitle("");
                  d2->SetLineColor(kGreen);
                  d2->GetXaxis()->SetLabelSize(0);
                  d2->GetXaxis()->SetTickLength(0);
                  d2->GetYaxis()->SetLabelSize(0);
                  d2->GetYaxis()->SetTickLength(0);
                  d2->GetYaxis()->SetRangeUser(-1,3);

                  TPad * pad1 = new TPad("pad_an1","",0,0,1,1);
                  TPad * pad2 = new TPad("pad_an2","",0,0,1,1);
                  TPad * pad3 = new TPad("pad_d1","",0,0,1,1);
                  TPad * pad4 = new TPad("pad_d2","",0,0,1,1);
                  pad2->SetFillStyle(4000);
                  pad2->SetFrameFillStyle(0);
                  pad3->SetFillStyle(4000);
                  pad3->SetFrameFillStyle(0);
                  pad4->SetFillStyle(4000);
                  pad4->SetFrameFillStyle(0);

                  pad1->Draw();
                  pad1->cd();
                  an1->Draw("al");

                  pad2->Draw();
                  pad2->cd();
                  an2->Draw("al");

                  pad3->Draw();
                  pad3->cd();
                  d1->Draw("al");

                  pad4->Draw();
                  pad4->cd();
                  d2->Draw("al");

                  canv_wf_map[ch]->Modified();
                  canv_wf_map[ch]->Update();
                }
            }
        }
      ++i_evt;

      if (count_events > 0 && i_evt > count_events) break;
      auto now_tp = std::chrono::system_clock::now();
      i_sec = std::chrono::duration_cast<std::chrono::seconds>(now_tp-start_tp).count();
      if (count_seconds > 0 && i_sec > count_seconds) break;

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now_tp-PrevRateTime).count();
      if (elapsed > 1000)
        {
          std::cout << "\r" << "Pulses Recorded = (";
          for (int ch = 0; ch < 8; ch++)
            {
              if (!(Params.ChannelMask & (1<<ch))) continue;
              std::cout << std::setw(7) << chan_pulses[ch] << ",";
            }
          std::cout << "\b), Readout Rate = " << std::setw(7) << (double)Nb/((double)elapsed*1048.576f) << " MB/s, ";
          std::cout << "Trig/Pileup Rate = (";
          for (int ch = 0; ch < 8; ch++)
            {
              if (!(Params.ChannelMask & (1<<ch))) continue;
              std::cout << std::setw(7) << (double)trgCnt[ch]/(double)elapsed << " kHz / " << std::setw(7) << (double)purCnt[ch]*100/(double)trgCnt[ch] << "%,";
              trgCnt[ch] = 0;
              purCnt[ch] = 0;
            }
          std::cout << "\b)" << std::flush;

          Nb = 0;
          PrevRateTime = std::chrono::system_clock::now();
        }

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

  std::cout << std::endl;
  std::cout << "Recorded " << i_evt << " events in " << i_sec << " seconds." << std::endl;

  return 0;
}
