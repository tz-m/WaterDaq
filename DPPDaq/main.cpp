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
#include "TLegend.h"

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
#include <curses.h>

#include "date.h"

static volatile bool keep_continue = true;

uint64_t ExtractBits(uint64_t value, uint64_t nbits, uint64_t startbit)
{
  // value = the value from you which you would like to extract a subset of bits
  // nbits = the number of bits you want to extract
  // startbit = the starting bit number you want. For example, if you want to start with the third bit, use startbit=2.
  return (((1 << nbits) - 1) & (value >> startbit));
}

struct Ev_t {
  uint32_t ev;
  int ch;
  uint32_t Format;
  uint64_t TimeTag;
  uint16_t Energy;
  int16_t Extras;
  uint32_t Extras2;
  uint16_t fine_time;
  uint16_t ETT_MSB;
  uint64_t ETT;
  int lost_event;
  int rollover_event;
  int fake_event;
  int input_saturation;
  int lost_trig;
  int tot_trig;
  int match_coinc;
  int nomatch_coinc;
  int tt_reset;
  Ev_t() {};
  Ev_t(uint32_t i_ev, int channel, CAEN_DGTZ_DPP_PHA_Event_t event) : ev(i_ev), ch(channel), Format(event.Format), TimeTag(event.TimeTag), Energy(event.Energy), Extras(event.Extras), Extras2(event.Extras2) {
    fine_time = ExtractBits(Extras2,16,0);
    //ETT_MSB = ExtractBits(Extras2,16,16);
    //ETT = (ETT_MSB << 31) + TimeTag;
    //lost_event = ExtractBits(Extras,1,0);
    //rollover_event = ExtractBits(Extras,1,1);
    //fake_event = ExtractBits(Extras,1,3);
    //input_saturation = ExtractBits(Extras,1,4);
    //lost_trig = ExtractBits(Extras,1,5);
    //tot_trig = ExtractBits(Extras,1,6);
    match_coinc = ExtractBits(Extras,1,7);
    nomatch_coinc = ExtractBits(Extras,1,8);
    //tt_reset = ExtractBits(Extras,1,2);
  }
};

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
  CAEN_DGTZ_PulsePolarity_t PulsePolarity[8];
  uint32_t ShapedTrigWidth[8];

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
    case -1L: std::cout << caller << ": CommError" << std::endl;
      break;
    case -2L: std::cout << caller << ": GenericError" << std::endl;
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
    }
  if (ret != CAEN_DGTZ_Success) exit(ret);
}

std::string AsBinary(uint32_t value)
{
  uint32_t bits_needed = 32;
  for (uint32_t expo = 1; expo <= 32; ++expo)
    {
      if (value <= std::pow(2,expo)-1)
        {
          bits_needed = expo;
          break;
        }
    }
  std::stringstream ss; ss << "0b";
  for (uint32_t b = bits_needed; b > 0; --b)
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
  timevec[0] = static_cast<int>(ymd.year());
  timevec[1] = static_cast<unsigned>(ymd.month());
  timevec[2] = static_cast<unsigned>(ymd.day());
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
  s << std::setfill('0') << std::setw(4) << static_cast<int>(ymd.year());
  s << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.month());
  s << std::setfill('0') << std::setw(2) << static_cast<unsigned>(ymd.day());
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
  return nullptr;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
  return std::find(begin, end, option) != end;
}

int main(int argc, char ** argv)
{
  std::stringstream setupnote;
  setupnote << "NOTE: This code makes several assumptions about setup which must be satisfied for accurate results."
            << " First, the AFG output of the scope must be set with the following parameters: Exponential Rise, Frequency 5kHz, Period 200 us, Amplitude 1.25 Vpp, Offset -1.25V."
            << " The AFT output should be Tee'd three ways to the digitizer GPI, the LED pulser trigger input, and CH4 of the digitizer."
            << " The GPI is required to reset the clock on each trigger, otherwise \"coincident\" events will not have sensible TimeTags."
            << " The LED pulser gate width is set short, and the gate delay is set clockwise all the way."
            << " The pulser gate bottom goes to TRG-IN on the digitizer."
            << " Most importantly, the digitizer channel inputs should be set to CH0 and CH1 for the two input post-CSP PMT signals."
            << std::endl;

  std::stringstream usage;
  usage << "Usage: ./DPPDaq -disp wxyz" << std::endl;
  usage << "   w: Analogue Trace 1 (0=Input, 1=RC-CR, 2=RC-CR2, 3=Trapezoid)" << std::endl;
  usage << "   x: Analogue Trace 2 (0=Input, 1=Threshold, 2=Trap-Base, 3=Baseline)" << std::endl;
  usage << "   y: Digital Trace 1  (0=Peaking, 1=Armed, 2=Peak Run, 3=Pile-up," << std::endl;
  usage << "                        4=Peaking, 5=Trg Validation Window, 6=Baseline Freeze, 7=Trg Holdoff," << std::endl;
  usage << "                        8=Trg Validation, 9=Acq Busy, a=Zero Cross Window, b=Ext Trg, c=Busy)" << std::endl;
  usage << "   z: Digital Trace 2  (0=Trigger)" << std::endl;
  usage << "E.g. \"./DPPDaq -disp 20b0\" will display RC-CR2, Input, Ext Trg, and Trigger." << std::endl;

  TApplication tapp("tapp",&argc,argv);

  long int count_events = -1;
  long int count_seconds = -1;
  bool disp = false;
  char* dispopt;
  uint32_t w, x, y, z;
  std::vector<std::string> axes_lim_input;
  std::vector<double> axes_lim = {0,-8192,20000,16384};
  if (cmdOptionExists(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-axes"))
    {
      // force axis drawing ranges
      char * result = getCmdOption(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-axes");
      if (result == nullptr)
        {
          std::cout << "Provide axes limits." << std::endl;
          std::cout << "Usage: ./DPPDaq -axes [xmin],[ymin],[xmax],[ymax]" << std::endl;
          std::cout << "    Note: no spaces between values, e.g. 0,-8192,20000,16384" << std::endl;
          std::cout << "    Note 2: This functionality is not very well tested for safety. I.e. DON'T GET IT WRONG because it will most likely crash, or worse, give bad information." << std::endl;
          return -1;
        }
      char *token;
      while ((token = strsep(&result, ","))) axes_lim_input.push_back(token);
      std::cout << axes_lim_input[0] << "," << axes_lim_input[1] << "," << axes_lim_input[2] << "," << axes_lim_input[3] << std::endl;
      for (size_t i = 0; i < axes_lim_input.size(); ++i)
        {
          if (axes_lim_input[i]!="")
            {
              axes_lim[i] = std::stod(axes_lim_input[i]);
            }
        }
    }
  if (cmdOptionExists(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-t"))
    {
      // time the run
      char * result = getCmdOption(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-t");
      if (result == nullptr)
        {
          std::cout << "Provide a number." << std::endl;
          std::cout << "Usage: ./DPPDaq -t [number of seconds to run]" << std::endl;
          return -1;
        }
      count_seconds = atol(result);
      std::cout << "Timed Run: " << count_seconds << " seconds." << std::endl;
    }
  if (cmdOptionExists(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-n"))
    {
      // count events
      char * result = getCmdOption(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-n");
      if (result == nullptr)
        {
          std::cout << "Provide a number." << std::endl;
          std::cout << "Usage: ./DPPDaq -n [number of events to record]" << std::endl;
          return -1;
        }
      count_events = atol(result);
      std::cout << "Limited Run: " << count_events << " events." << std::endl;
    }
  if (cmdOptionExists(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-disp"))
    {
      dispopt = getCmdOption(tapp.Argv(),tapp.Argv()+tapp.Argc(),"-disp");
      if (dispopt == nullptr || strlen(dispopt) != 4 ||
          (!((dispopt[0]=='0' || dispopt[0]=='1' || dispopt[0]=='2' || dispopt[0]=='3') &&
             (dispopt[1]=='0' || dispopt[1]=='1' || dispopt[1]=='2' || dispopt[1]=='3') &&
             (dispopt[2]=='0' || dispopt[2]=='1' || dispopt[2]=='2' || dispopt[2]=='3' ||
              dispopt[2]=='4' || dispopt[2]=='5' || dispopt[2]=='6' || dispopt[2]=='7' ||
              dispopt[2]=='8' || dispopt[2]=='9' || dispopt[2]=='a' || dispopt[2]=='A' ||
              dispopt[2]=='b' || dispopt[2]=='B' || dispopt[2]=='c' || dispopt[2]=='C') &&
             (dispopt[3]=='0'))))
        {
          std::cout << "Invalid choice of display options." << std::endl << usage.str();
          return -1;
        }

      if (dispopt[0] >= '0' && dispopt[0] <= '3') w = static_cast<uint32_t>(dispopt[0]-'0');
      if (dispopt[1] >= '0' && dispopt[1] <= '3') x = static_cast<uint32_t>(dispopt[1]-'0');
      if (dispopt[2] >= '0' && dispopt[2] <= '9') y = static_cast<uint32_t>(dispopt[2]-'0');
      if (dispopt[2] >= 'a' && dispopt[2] <= 'c') y = static_cast<uint32_t>(dispopt[2]-'a'+10);
      if (dispopt[2] >= 'A' && dispopt[2] <= 'C') y = static_cast<uint32_t>(dispopt[2]-'A'+10);
      if (dispopt[3] == '0') z = static_cast<uint32_t>(dispopt[3]-'0');

      disp = true;
      std::cout << "Display histograms and traces using options " << dispopt << " and axis limits " << axes_lim[0] << "," << axes_lim[1] << "," << axes_lim[2] << "," << axes_lim[3] << "." << std::endl;
    }

  std::string w_name, x_name, y_name, z_name;
  if (disp)
    {
      switch(w)
        {
        case 0: w_name = "Input"; break;
        case 1: w_name = "RC-CR"; break;
        case 2: w_name = "RC-CR2"; break;
        case 3: w_name = "Trapezoid"; break;
        default:
          std::cout << "Unknown display option w=" << w << ". Please try something else." << std::endl << usage.str();
          return -1;
          break;
        }
      switch(x)
        {
        case 0: x_name = "Input"; break;
        case 1: x_name = "Threshold"; break;
        case 2: x_name = "Trapezoid-Baseline"; break;
        case 3: x_name = "Baseline"; break;
        default:
          std::cout << "Unknown display option x=" << x << ". Please try something else." << std::endl << usage.str();
          return -1;
          break;
        }
      switch (y)//0=Peaking, 1=Armed, 2=Peak Run, 3=Pile-up, 4=Peaking, 5=Trg Validation Window, 6=Baseline Freeze, 7=Trg Holdoff, 8=Trg Validation, 9=Acq Busy, a=Zero Cross Window, b=Ext Trg, c=Busy
        {
        case 0: y_name = "Peaking"; break;
        case 1: y_name = "Armed"; break;
        case 2: y_name = "Peak Run"; break;
        case 3: y_name = "Pile-up"; break;
        case 4: y_name = "Peaking"; break;
        case 5: y_name = "Trigger Validation Window"; break;
        case 6: y_name = "Baseline Freeze"; break;
        case 7: y_name = "Trigger Holdoff"; break;
        case 8: y_name = "Trigger Validation"; break;
        case 9: y_name = "Acquisition Busy"; break;
        case 10: y_name = "Zero Crossing Window"; break;
        case 11: y_name = "External Trigger"; break;
        case 12: y_name = "Busy"; break;
        default:
          std::cout << "Unknown display option y=" << y << ". Please try something else." << std::endl << usage.str();
          return -1;
          break;
        }
      switch (z)// 0=Trigger
        {
        case 0: z_name = "Trigger"; break;
        default:
          std::cout << "Unknown display option z=" << z << ". Please try something else." << std::endl << usage.str();
          return -1;
          break;
        }
    }

  std::string thisTimeString = MakeTimeString();

  std::stringstream configfilename;
  configfilename << "config_DPPDaq_" << thisTimeString << ".txt";
  std::cout << "Config filename " << configfilename.str() << std::endl;
  std::ofstream fs(configfilename.str());

  if (disp) fs << "Display options: " << dispopt << std::endl;
  fs << setupnote.str();

  int handle;

  //buffers to store data
  char *buffer = nullptr;
  CAEN_DGTZ_DPP_PHA_Event_t *Events[8];
  CAEN_DGTZ_DPP_PHA_Waveforms_t *Waveform=nullptr;

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
  Params.AcqMode = CAEN_DGTZ_DPP_ACQ_MODE_Mixed;
  Params.RecordLength = 10000;// Number of samples at 2ns per sample
  Params.ChannelMask = (1<<0) + (0<<1) + (1<<2) + (0<<3) + (1<<4) + (0<<5) + (0<<6) + (0<<7); // {1=enable, 0=disable} << =left.bit.shift {channel number}
  Params.EventAggr = 0;//0 = automatic
  Params.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive;

  for (int ch = 0; ch < 8; ++ch)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      DPPParams.thr[ch] = 25;// discriminator threshold (LSB)
      DPPParams.k[ch] = 6000;// trap rise time (ns)
      DPPParams.m[ch] = 1000;// trap flat top (ns)
      DPPParams.M[ch] = (ch==0)?120000:115000;// Exponential decay time of the preamp (ns)
      DPPParams.ftd[ch] = static_cast<int>(0.4*DPPParams.m[ch]);// flat top delay ("PEAKING TIME"), 80% of flat top is a good value
      DPPParams.a[ch] = 0x8;// RC-CR2 smoothing factor
      DPPParams.b[ch] = 104;//input rise time (ns)
      DPPParams.trgho[ch] = 20000;// Trigger hold-off
      DPPParams.nsbl[ch] = 3;// Num samples in baseline averaging, 0b11 = 256 samples (512ns)
      DPPParams.nspk[ch] = 3;// peak mean 0x2 = 0b10 corresponds to 16 samples
      DPPParams.pkho[ch] = 960;// peak hold-off
      DPPParams.blho[ch] = 8000;// baseline hold-off
      DPPParams.enf[ch] = 1.0;// energy normalisation factor
      DPPParams.decimation[ch] = 0;
      DPPParams.dgain[ch] = 0;
      DPPParams.otrej[ch] = 0;
      DPPParams.trgwin[ch] = 0;// enable/disable rise time discriminator
      DPPParams.twwdt[ch] = 104;// rise time validation window
      MoreChanParams.InputDynamicRange[ch] = 0;
      MoreChanParams.PreTriggerSize[ch] = 2000;// ns
      MoreChanParams.ChannelDCOffset[ch] = static_cast<int>((1-0.1)*0xFFFF);// use formula (1 - percent_offset)*0xFFFF where percent_offset is the place you want the baseline within the full range, e.g. 0.2 for 20%
      MoreChanParams.PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityPositive;
      MoreChanParams.ShapedTrigWidth[ch] = 12;
      if (ch == 4)
        {// external trigger into ch4
          DPPParams.k[ch] = 1000;
          DPPParams.m[ch] = 100;
          DPPParams.nspk[ch] = 0;
          DPPParams.trgho[ch] = 20000;
          MoreChanParams.InputDynamicRange[ch] = 0;
          MoreChanParams.PulsePolarity[ch] = CAEN_DGTZ_PulsePolarityNegative;
          MoreChanParams.ChannelDCOffset[ch] = static_cast<int>((1-0.9)*0xFFFF);
          DPPParams.M[ch] = 20000;
          DPPParams.pkho[ch] = 40;
          DPPParams.ftd[ch] = static_cast<int>(0.4*DPPParams.m[ch]);
          MoreChanParams.ShapedTrigWidth[ch] = 88;
        }
    }

  MoreChanParams.AutoDataFlush = 0;
  MoreChanParams.SaveDecimated = 0;
  MoreChanParams.TrigPropagation = 1;
  MoreChanParams.DualTrace = (disp)?1:0;
  MoreChanParams.AnProbe1 = (disp)?w:0;// 0=input, 1=RC-CR, 2=RC-CR2, 3=trapezoid
  MoreChanParams.AnProbe2 = (disp)?x:0;// 0=input, 1=threshold, 2=trap-base, 3=baseline
  MoreChanParams.WaveformRecording = (disp)?1:0;
  MoreChanParams.EnableExtras2 = 1;
  MoreChanParams.DigVirtProbe1 = (disp)?y:0;// 0=peaking, 3=pileup, 5=trig valid window, 7=trig holdoff, 8=trig validation, 10=zero cross window, 11=ext trig, 12=busy (other options available)
  MoreChanParams.DigVirtProbe2 = (disp)?z:0;// 0=Trigger
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

  CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,0,0,Params.VMEBaseAddress,&handle);
  if (ret == -1L)// If a CommError, try again with different USB link number. It seems to switch between 0 and 1 depending on unknown factors.
    {
      ret = CAEN_DGTZ_OpenDigitizer(Params.LinkType,1,0,Params.VMEBaseAddress,&handle);
    }
  CheckErrorCode(ret,"OpenDigitizer");// If there is still an error, crash gracefully.

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
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8000,static_cast<uint32_t>(boardcfg.to_ulong())),"SetBoardConfiguration");
  CheckErrorCode(CAEN_DGTZ_SetDPPAcquisitionMode(handle, Params.AcqMode, CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime),"SetDPPAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED),"SetAcquisitionMode");
  CheckErrorCode(CAEN_DGTZ_SetRecordLength(handle, Params.RecordLength),"SetRecordLength");//This value is Ns (number of samples, at 2ns per sample. So Ns=10k is 20us)
  CheckErrorCode(CAEN_DGTZ_SetIOLevel(handle, Params.IOlev),"SetIOLevel");
  CheckErrorCode(CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT),"SetExtTriggerInputMode");
  CheckErrorCode(CAEN_DGTZ_SetChannelEnableMask(handle, Params.ChannelMask),"SetChannelEnableMask");
  CheckErrorCode(CAEN_DGTZ_SetRunSynchronizationMode(handle, CAEN_DGTZ_RUN_SYNC_Disabled),"SetRunSynchronizationMode");
  CheckErrorCode(CAEN_DGTZ_SetDPPParameters(handle, Params.ChannelMask, &DPPParams),"SetDPPParameters");
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x817C, 1),"WriteEnableExternalTrigger");

  // Global Trigger Mask
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x810C, &value),"ReadRegister(0x810C)");
  //value |= 1UL << 30;// Enable external trigger
  value &= ~(1UL << 30);//Disable external trigger
  value &= ~(1UL << 31);// Disable software trigger
  value &= ~(1UL << 0);// Channels 0 and 1 do not contribute to global trigger generation
  //value |= 1UL << 0;// Channels 0 and 1 contribute to global trigger generation
  //value |= 1UL << 27;// Supposedly set TRG-IN as gate. says so in the DT5730 manual, but absent in the DPP-PHA registers document.
  //value |= 30 << 20;// Set coincidence window
  //value |= 1UL << 24;// set majority level
  value = 0;// disable global trigger entirely
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x810C, value),"WriteRegister(0x810C)");

  // Front Panel I/O Control
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x811C, &value),"ReadRegister(0x811C)");
  //value |= 1UL << 10;// Trigger is synchronized with the whole duration of the TRG-IN signal
  value &= ~(1UL << 10);// Trigger is synchronised with the edge of TRG-IN
  //value &= ~(1UL << 11);// Trig in processed by motherboard then sent to mezzanines
  value |= 1UL << 11;// Trig in sent directly to mezzanines
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x811C, value),"WriteRegister(0x811C");

  // Trigger Validation Mask
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x8180, &value),"ReadTriggerValidationMask_Couple0");
  value |= 1UL << 0;// Enable couple 0 trigger validation signal
  //value |= 1UL << 1;// same couple 1
  value |= 1UL << 2;// same couple 2
  value &= ~(1UL << 9); value |= 1UL << 8;// Set trigger validation AND
  //value &= ~(1UL << 9); value &= ~(1UL << 8);// set trigger validation OR
  //value |= 1UL << 9; value &= ~(1UL << 8);//Set trigger validation majority
  //value |= 1 << 10;// Set majority level
  value |= 1UL << 30;// External trigger creates validation signal
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x8180, value),"WriteTriggerValidationMask_Couple0");
  CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x8184, &value),"ReadTriggerValidationMask_Couple0");
  value |= 1UL << 1;
  value |= 1UL << 2;
  value &= ~(1UL << 9); value |= 1UL << 8;
  value |= 1UL << 30;
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x8184, value),"WriteTriggerValidationMask_Couple1");
  //CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x8110, value),"WriteGPOMask");

  for (uint32_t i = 0; i < 8; ++i)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_SetChannelDCOffset(handle, i, MoreChanParams.ChannelDCOffset[i]),"SetChannelDCOffset");
          CheckErrorCode(CAEN_DGTZ_SetDPPPreTriggerSize(handle, static_cast<int>(i), MoreChanParams.PreTriggerSize[i]/2),"SetDPPPreTriggerSize");
          CheckErrorCode(CAEN_DGTZ_SetChannelPulsePolarity(handle, i, MoreChanParams.PulsePolarity[i]),"SetChannelPulsePolarity");

          // DPP Algorithm Control
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x1080+i*0x100, &value),"ReadRegister(0x1080)");
          //value |= 1UL << 24;// Disable self trigger
          //value &= ~(1UL << 24);// Enable self trigger
          value &= ~(1UL << 19); value |= 1UL << 18;// Enable coincidence mode
          //value |= 1UL << 27;// Readout pile-up events
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1080+i*0x100, value),"WriteRegister(0x1080)");

          // DPP Algorithm Control 2
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle, 0x10A0+i*0x100, &value),"ReadDPPAlg2");
          value |= 1UL << 29;// Enable BLR optimization
          //value &= ~(1UL << 2);// Disable local shaped trigger
          value |= 1UL << 2;// Enable local shaped trigger
          //value |= 1UL << 0; value |= 1UL << 1; // set local shaped trigger mode OR
          //value &= ~(1UL << 1); value &= ~(1UL << 0);// set local shaped trigger mode AND
          value &= ~(1UL << 1); value |= 1UL << 0;// set local shaped trigger mode even channel of the couple
          value &= ~(1UL << 5); value |= 1UL << 4;// Set trig validation mode Motherboard
          //value |= 1UL << 5; value &= ~(1UL << 4);// Set trig validation mode AND
          //value |= 1UL << 5; value |= 1UL << 4;// Set trig validation mode OR
          value |= 1UL << 6;// Enable local trigger validation mode
          //value &= ~(1UL << 6);// Disable local trigger validation mode)
          value &= ~(1UL << 10); value |= 1UL << 9; value &= ~(1UL << 8);// extended and fine timestamps in extras2
          value |= 1UL << 19; // tag correlated events in extras
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x10A0+i*0x100, value),"WriteDPPAlg2");

          // set input dynamic range
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1028+i*0x100, MoreChanParams.InputDynamicRange[i]),"SetInputDynamicRange");

          // set shaped trigger width
          CheckErrorCode(CAEN_DGTZ_WriteRegister(handle, 0x1084+i*0x100, MoreChanParams.ShapedTrigWidth[i]),"WriteShapedTriggerWidth");
        }
    }
  if (!disp)
    {
      CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x8000,&value),"ReadBoardConfiguration");
      value &= ~(1UL << 16);// disable waveform recording
      CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x8000,value),"WriteBoardConfiguration");
    }

  CheckErrorCode(CAEN_DGTZ_SetDPPEventAggregation(handle, Params.EventAggr, 0),"SetDPPEventAggregation");

  uint32_t AllocatedSize;
  CheckErrorCode(CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize),"MallocReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_MallocDPPEvents(handle, reinterpret_cast<void**>(Events), &AllocatedSize),"MallocDPPEvents");
  if (disp) CheckErrorCode(CAEN_DGTZ_MallocDPPWaveforms(handle, reinterpret_cast<void**>(&Waveform), &AllocatedSize),"MallocDPPWaveforms");


  /*Check All Parameters*/

  //Global
  CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value),"GetRecordLength");
  fs << "GetRecordLength: " << value << std::endl;
  CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value),"GetNumEventsPerAggregate");
  fs << "GetNumEventsPerAggregate: " << value << std::endl;

  //Channel
  for (uint32_t i = 0; i < 8; ++i)
    {
      if (Params.ChannelMask & (1<<i)) {
          CheckErrorCode(CAEN_DGTZ_GetRecordLength(handle,&value,i),"GetRecordLengthChannelI");
          fs << "Ch" << i << " Record Length; " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1028+i*0x100,&value),"ReadChannelIInputDynamicRange");
          fs << "Ch" << i << " Input Dynamic Range: " << ((value==0)?"2 Vpp":((value==1)?"0.5 Vpp":"Invalid")) << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetNumEventsPerAggregate(handle,&value,i),"GetNumEventsPerAggregateChannelI");
          fs << "Ch" << i << " NumEventsPerAggregate: " << value << std::endl;
          CheckErrorCode(CAEN_DGTZ_GetDPPPreTriggerSize(handle,static_cast<int>(i),&value),"GetDPPPreTriggerSize");
          fs << "Ch" << i << " PreTrigger: " << value*2 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x104C+i*0x100,&value),"ReadFineGainChannelI");
          fs << "Ch" << i << " Fine Gain: " << value << " (if =250, fg is probably 1.0)" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1054+i*0x100,&value),"ReadRC-CR2SmootingFactorChannelI");
          fs << "Ch" << i << " RC-CR2 Smoothing Factor: 0x" << std::hex << value << std::dec << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1058+i*0x100,&value),"ReadInputRiseTimeChannelI");
          fs << "Ch" << i << " Input Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x105C+i*0x100,&value),"ReadTrapRiseTimeChannelI");
          fs << "Ch" << i << " Trap Rise Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1060+i*0x100,&value),"ReadTrapFlatTopChannelI");
          fs << "Ch" << i << " Trap Flat Top: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1064+i*0x100,&value),"ReadPeakingTimeChannelI");
          fs << "Ch" << i << " Peaking Time: " << value*8 << " ns" << std::endl;
          CheckErrorCode(CAEN_DGTZ_ReadRegister(handle,0x1068+i*0x100,&value),"ReadDecayTimeChannelI");
          fs << "Ch" << i << " Decay Time: " << static_cast<double>(value)*0.008 << " microseconds" << std::endl;
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
          CheckErrorCode(CAEN_DGTZ_ReadTemperature(handle,static_cast<int>(i),&value),"ReadTemperature");
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

  //Synchronise ADC clocks
  CheckErrorCode(CAEN_DGTZ_WriteRegister(handle,0x813C,0),"SyncADCClocks");

  // Calibrate ADCs
  for (uint32_t ch = 0; ch < 8; ++ch)
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

  std::array<std::shared_ptr<TH1I>,8> h_vec;
  std::array<std::shared_ptr<TH1D>,8> TTS_vec;

  // Start acquisition
  uint32_t BufferSize;

  std::signal(SIGINT, [](int) { keep_continue = false; });

  CheckErrorCode(CAEN_DGTZ_SWStartAcquisition(handle),"SWStartAcquisition");

  int i_evt = 0;
  long int i_sec = 0;
  std::array<int,8> chan_pulses={};
  std::array<int,8> trgCnt={};
  std::array<int,8> purCnt={};
  int Nb = 0;

  std::array<std::shared_ptr<TCanvas>,8> canv_wf_vec;
  std::array<std::shared_ptr<TCanvas>,8> canv_hist_vec;
  std::array<std::shared_ptr<TCanvas>,8> canv_TTS_vec;

  int num_enabled_channels = 0;
  for (int ch = 0; ch < 8; ++ch)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      if (disp)
        {
          canv_wf_vec[ch] = std::make_shared<TCanvas>((static_cast<std::string>("wf_ch")+std::to_string(ch)).c_str(),"",1600,900);
          canv_hist_vec[ch] = std::make_shared<TCanvas>((static_cast<std::string>("hist_ch")+std::to_string(ch)).c_str(),"",1600,900);
          canv_TTS_vec[ch] = std::make_shared<TCanvas>((static_cast<std::string>("TTS_ch")+std::to_string(ch)).c_str(),"",1600,900);
        }
      h_vec[ch] = std::make_shared<TH1I>((static_cast<std::string>("h_ch")+std::to_string(ch)).c_str(),";ADC Channel;",16384,0,16384);
      TTS_vec[ch] = std::make_shared<TH1D>((static_cast<std::string>("TTS_ch")+std::to_string(ch)).c_str(),";Time (ns);",4000,500,700);
      ++num_enabled_channels;
    }

  auto start_tp = std::chrono::system_clock::now();
  TVectorD starttimevec = MakeTimeVec(start_tp);
  auto PrevRateTime = start_tp;

  std::array<int,1024> goodEvent{};
  std::array<std::array<Ev_t,8>,1024> reinterpret_Events;

  std::array<int,8> intime_purCnt{};
  std::array<int,8> outtime_purCnt{};
  std::array<int,8> intime_matchCnt{};
  std::array<int,8> outtime_trgCnt{};
  int intime_trgCnt_ch4 = 0;

  while(keep_continue)
    {
      CheckErrorCode(CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize),"ReadData");
      if (BufferSize == 0) continue;
      Nb += BufferSize;
      CheckErrorCode(CAEN_DGTZ_GetDPPEvents(handle, buffer, BufferSize, reinterpret_cast<void**>(Events), NumEvents),"GetDPPEvents");

      uint32_t maxEvent = 0;
      for (int ch = 0; ch < 8; ++ch)
        {
          if (!(Params.ChannelMask & (1<<ch))) continue;

          uint32_t energy, ev, saveev;

          for (ev = 0; ev < NumEvents[ch]; ++ev)
            {
              if (ev > 1023) break;
              auto readevent = Events[ch][ev];
              ++trgCnt[ch];
              energy = readevent.Energy;
              int match = ExtractBits(readevent.Extras,2,7);// read match_coinc and nomatch_coinc together for speed
              if (energy > static_cast<uint32_t>(DPPParams.thr[ch]) && energy < 16383)
                {
                  if (match == 0)
                    {
                      h_vec[ch]->Fill(energy);
                      ++chan_pulses[ch];
                      reinterpret_Events[ev][ch] = Ev_t(ev,ch,readevent);
                      goodEvent[ev]=1;
                      saveev = ev;
                      if (ev > maxEvent) maxEvent = ev;
                      ++intime_matchCnt[ch];
                      if (ch==4) ++intime_trgCnt_ch4;
                    }
                  else
                    {
                      ++outtime_trgCnt[ch];
                      ++purCnt[ch];
                    }
                }
              else
                {
                  if (match == 0)
                    {
                      ++intime_purCnt[ch];
                      ++purCnt[ch];
                    }
                  else
                    {
                      ++outtime_trgCnt[ch];
                      ++outtime_purCnt[ch];
                      ++purCnt[ch];
                    }
                }
            }

          if (disp && energy > static_cast<uint32_t>(DPPParams.thr[ch]) && energy < 16383)
            {
              int size;
              std::vector<Int_t> time_x;
              int16_t *WaveLine_an1;
              int16_t *WaveLine_an2;
              uint8_t *DigitalWaveLine_d1;
              uint8_t *DigitalWaveLine_d2;

              std::vector<Int_t> WL_an1, WL_an2, DWL_d1, DWL_d2;

              CheckErrorCode(CAEN_DGTZ_DecodeDPPWaveforms(handle, &Events[ch][saveev], Waveform),"DecodeDPPWaveforms");

              // Use waveform data here...
              size = static_cast<int>(Waveform->Ns); // Number of samples

              WaveLine_an1 = Waveform->Trace1; // First trace (ANALOG_TRACE_1)
              WaveLine_an2 = Waveform->Trace2; // Second Trace ANALOG_TRACE_2 (if single trace mode, it is a sequence of zeroes)
              DigitalWaveLine_d1 = Waveform->DTrace1; // First Digital Trace (DIGITALPROBE1)
              DigitalWaveLine_d2 = Waveform->DTrace2; // Second Digital Trace (for DPP-PHA it is ALWAYS Trigger)

              for (int pt = 0; pt < size; ++pt)
                {
                  time_x.push_back(pt*2);
                  WL_an1.push_back(static_cast<Int_t>(WaveLine_an1[pt]));
                  WL_an2.push_back(static_cast<Int_t>(WaveLine_an2[pt]));
                  DWL_d1.push_back(static_cast<Int_t>(DigitalWaveLine_d1[pt]));
                  DWL_d2.push_back(static_cast<Int_t>(DigitalWaveLine_d2[pt]));
                }

              canv_wf_vec[ch]->cd();

              TGraph an1(size,time_x.data(),WL_an1.data());
              an1.SetTitle((static_cast<std::string>("Channel")+std::to_string(ch)+";Time (ns);ADC Value").c_str());
              an1.SetLineWidth(3);
              an1.SetLineColor(kBlack);
              an1.GetYaxis()->SetRangeUser(axes_lim[1],axes_lim[3]);
              an1.GetXaxis()->SetRangeUser(axes_lim[0],axes_lim[2]);
              TGraph an2(size,time_x.data(),WL_an2.data());
              an2.SetTitle("");
              an2.SetLineWidth(3);
              an2.SetLineColor(kRed);
              an2.GetXaxis()->SetLabelSize(0);
              an2.GetXaxis()->SetTickLength(0);
              an2.GetYaxis()->SetLabelSize(0);
              an2.GetYaxis()->SetTickLength(0);
              an2.GetYaxis()->SetRangeUser(axes_lim[1],axes_lim[3]);
              an2.GetXaxis()->SetRangeUser(axes_lim[0],axes_lim[2]);
              TGraph d1(size,time_x.data(),DWL_d1.data());
              d1.SetTitle("");
              d1.SetLineWidth(3);
              d1.SetLineColor(kBlue);
              d1.GetXaxis()->SetLabelSize(0);
              d1.GetXaxis()->SetTickLength(0);
              d1.GetYaxis()->SetLabelSize(0);
              d1.GetYaxis()->SetTickLength(0);
              d1.GetXaxis()->SetRangeUser(axes_lim[0],axes_lim[2]);
              TGraph d2(size,time_x.data(),DWL_d2.data());
              d2.SetTitle("");
              d2.SetLineWidth(3);
              d2.SetLineColor(kGreen);
              d2.GetXaxis()->SetLabelSize(0);
              d2.GetXaxis()->SetTickLength(0);
              d2.GetYaxis()->SetLabelSize(0);
              d2.GetYaxis()->SetTickLength(0);
              d2.GetXaxis()->SetRangeUser(axes_lim[0],axes_lim[2]);

              TPad pad1("pad_an1","",0,0,1,1);
              TPad pad2("pad_an2","",0,0,1,1);
              TPad pad3("pad_d1","",0,0,1,1);
              TPad pad4("pad_d2","",0,0,1,1);
              pad2.SetFillStyle(4000);
              pad2.SetFrameFillStyle(0);
              pad3.SetFillStyle(4000);
              pad3.SetFrameFillStyle(0);
              pad4.SetFillStyle(4000);
              pad4.SetFrameFillStyle(0);

              TLegend leg(0.7,0.7,0.9,0.9);
              leg.AddEntry(&an1,w_name.c_str(),"lf");
              leg.AddEntry(&an2,x_name.c_str(),"lf");
              leg.AddEntry(&d1,y_name.c_str(),"lf");
              leg.AddEntry(&d2,z_name.c_str(),"lf");

              pad1.Draw();
              pad1.cd();
              an1.Draw("al");
              leg.Draw();

              pad2.Draw();
              pad2.cd();
              an2.Draw("al");

              pad3.Draw();
              pad3.cd();
              d1.Draw("al");

              pad4.Draw();
              pad4.cd();
              d2.Draw("al");

              canv_wf_vec[ch]->Modified();
              canv_wf_vec[ch]->Update();

              if (ch != 4)
                {
                  canv_hist_vec[ch]->cd();
                  h_vec[ch]->Draw();
                  canv_hist_vec[ch]->Modified();
                  canv_hist_vec[ch]->Update();
                }

              if (TTS_vec[ch]->GetEntries()>0 && ch != 4)
                {
                  canv_TTS_vec[ch]->cd();
                  TTS_vec[ch]->Draw();
                  canv_TTS_vec[ch]->Modified();
                  canv_TTS_vec[ch]->Update();
                }
            }
        }

      for (uint32_t ev = 0; ev < maxEvent; ++ev)
        {
          if (goodEvent[ev]==0) continue;

          auto reEv = reinterpret_Events[ev];
          auto ev0 = reEv[0];
          auto ev2 = reEv[2];
          auto ev4 = reEv[4];

          int accept0 = ev0.match_coinc + ev0.nomatch_coinc + (ev0.Energy>0)?0:1, accept2 = ev2.match_coinc + ev2.nomatch_coinc + (ev2.Energy>0)?0:1;
          uint64_t time0 = ev0.TimeTag, time2 = ev2.TimeTag, time4 = ev4.TimeTag;
          uint16_t fine0 = ev0.fine_time, fine2 = ev2.fine_time, fine4 = ev4.fine_time;
          double ts0 = time0+fine0*0.001, ts2 = time2+fine2*0.001, ts4 = time4+fine4*0.001;
          //std::cout << "ts0=" << ts0 << "  ts2=" << ts2 << "  ts4=" << ts4 << std::endl;
          if (accept0 == 0)
            {
              TTS_vec[0]->Fill((ts0-ts4)*2);// 2ns clock time
            }
          if (accept2 == 0)
            {
              TTS_vec[2]->Fill((ts2-ts4)*2);
            }
          goodEvent[ev]=0;
        }

      ++i_evt;

      if (count_events > 0 && i_evt >= count_events) break;
      auto now_tp = std::chrono::system_clock::now();
      i_sec = std::chrono::duration_cast<std::chrono::seconds>(now_tp-start_tp).count();
      if (count_seconds > 0 && i_sec > count_seconds) break;

      double elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(now_tp-PrevRateTime).count());
      if (elapsed > 1000)
        {
          std::cout << "\r" << "Ev " << i_evt << ", Readout rate = " << static_cast<double>(Nb)/(static_cast<double>(elapsed*1048.576f)) << " MB/s\e[K\n";
          for (int ch = 0; ch < 8; ++ch)
            {
              if (!(Params.ChannelMask & (1<<ch))) continue;
              std::cout << "     Ch" << ch << " pulses = " << std::setw(8) << chan_pulses[ch] << ", "
                        << "Total Trig Rate = " << std::fixed << std::setprecision(3) << std::setw(6) << static_cast<double>(trgCnt[ch])/static_cast<double>(elapsed) << " kHz, "
                        << "In-time Trig Rate = " << std::fixed << std::setprecision(3) << std::setw(6) << static_cast<double>(intime_matchCnt[ch])/static_cast<double>(elapsed) << " kHz, "
                        << "Out-time Trig Rate = " << std::fixed << std::setprecision(3) << std::setw(6) << static_cast<double>(outtime_trgCnt[ch])/static_cast<double>(elapsed) << " kHz, "
                        << "Total Pile-up = " << std::fixed << std::setprecision(2) << std::setw(5) << static_cast<double>(purCnt[ch]*100)/static_cast<double>(trgCnt[ch]) << "%, "
                        << "In-time Pile-up = " << std::fixed << std::setprecision(2) << std::setw(5) << static_cast<double>(intime_purCnt[ch]*100)/static_cast<double>(intime_trgCnt_ch4) << "%, "
                        << "Out-time Pile-up = " << std::fixed << std::setprecision(2) << std::setw(5) << static_cast<double>(outtime_purCnt[ch]*100)/static_cast<double>(outtime_trgCnt[ch]) << "%\e[K\n";
              trgCnt[ch] = 0;
              purCnt[ch] = 0;
              intime_purCnt[ch] = 0;
              outtime_purCnt[ch] = 0;
              intime_matchCnt[ch] = 0;
              outtime_trgCnt[ch] = 0;
            }
          intime_trgCnt_ch4 = 0;
          Nb = 0;
          std::cout << "\r\e[A";
          for (int ch = 0; ch < 8; ++ch)
            {
              if (!(Params.ChannelMask & (1<<ch))) continue;
              std::cout << "\e[A";
            }
          std::cout << std::flush;
          PrevRateTime = std::chrono::system_clock::now();
        }
    }
  for (int ch = 0; ch < 8; ++ch)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      std::cout << "\n";
    }

  auto end_tp = std::chrono::system_clock::now();
  TVectorD endtimevec = MakeTimeVec(end_tp);

  starttimevec.Write("starttime");
  endtimevec.Write("endtime");
  for (int ch = 0; ch < 8; ++ch)
    {
      if (!(Params.ChannelMask & (1<<ch))) continue;
      if (h_vec[ch]->GetEntries() > 0) h_vec[ch]->Write();
      if (TTS_vec[ch]->GetEntries() > 0) TTS_vec[ch]->Write();
    }
  fout->Close();
  fs.close();

  CheckErrorCode(CAEN_DGTZ_SWStopAcquisition(handle),"SWStopAcquisition");
  CheckErrorCode(CAEN_DGTZ_FreeReadoutBuffer(&buffer),"FreeReadoutBuffer");
  CheckErrorCode(CAEN_DGTZ_FreeDPPEvents(handle,reinterpret_cast<void**>(Events)),"FreeDPPEvents");
  CheckErrorCode(CAEN_DGTZ_FreeDPPWaveforms(handle, Waveform),"FreeDPPWaveforms");
  CheckErrorCode(CAEN_DGTZ_CloseDigitizer(handle),"CloseDigitizer");

  std::cout << std::endl;
  std::cout << "Recorded " << i_evt << " events in " << i_sec << " seconds." << std::endl;

  return 0;
}
