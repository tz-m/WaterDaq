#include "PulseDaq.h"

int main(int argc, char ** argv)
{
  // Struct for storing enabled MQDC32 channels
  MQDC32_EnableChannel chans;

  // command line parameters
  uint32_t NumEvents;
  uint32_t Delay = 0.0;
  bool Verbose = false;

  // flags for required command line args
  bool chanflag = false;
  bool numflag = false;
  bool delayflag = false;

  // Parse command line args
  for (int i = 1; i < argc; ++i)
    {
      if (!strcmp(argv[i],"-n"))
	{
	  if (i+2 > argc)
	    {
	      std::cout << "need to specify a number!" << std::endl;
	      return 0;
	    }
	  NumEvents = std::stod(argv[i+1]);
	  numflag = true;
	  ++i;
	}
      else if (!strcmp(argv[i],"-d"))
	{
	  if (i+2 > argc)
	    {
	      std::cout << "need to specify a delay (in microseconds)!" << std::endl;
	      return 0;
	    }
	  Delay = std::stod(argv[i+1]);
	  delayflag = true;
	  ++i;
	}
      else if (!strcmp(argv[i],"-v"))
	{
	  Verbose = true;
	}
      else 
	{
	  chans.ch[std::stoul(argv[i])] = true;
	  chanflag = true;
	}
    }
  if (!(numflag && chanflag && delayflag))
    {
      std::cout << "I need more information!!!" << std::endl
		<< "  Tell me how many events to record:" << std::endl
		<< "     -n [number]" << std::endl
		<< "  and the channels which you want me to record" << std::endl
		<< "     chA chB chC ..." << std::endl
		<< "  and the delay time between recorded events" << std::endl
		<< "     -d [number of microseconds]" << std::endl;
      return 0;
    }

  using namespace date;
  auto now = std::chrono::system_clock::now();
  auto dp = floor<days>(now);
  auto ymd = year_month_day{dp};
  auto time = make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now-dp));
  TString filename = TString::Format("DAQ_%04i%02i%02i_%02i%02i%02i.root",(int)(ymd.year()),(unsigned)(ymd.month()),(unsigned)(ymd.day()),time.hours().count(),time.minutes().count(),(unsigned)time.seconds().count());

  TFile * fileout = TFile::Open(filename,"RECREATE");
  UInt_t adc;
  UInt_t overflow;
  UInt_t channel;
  //UInt_t timestamp;
  Int_t year;
  Int_t month;
  Int_t day;
  Int_t hour;
  Int_t minute;
  Int_t second;
  Int_t millisecond;
  TTree * tree = new TTree("pulse","Pulse Data");
  tree->Branch("adc",&adc,"adc/i");
  tree->Branch("overflow",&overflow,"overflow/i");
  tree->Branch("channel",&channel,"channel/i");
  //tree->Branch("timestamp",&timestamp,"timestamp/i");
  tree->Branch("year",&year,"year/I");
  tree->Branch("month",&month,"month/I");
  tree->Branch("day",&day,"day/I");
  tree->Branch("hour",&hour,"hour/I");
  tree->Branch("minute",&minute,"minute/I");
  tree->Branch("second",&second,"second/I");
  tree->Branch("millisecond",&millisecond,"millisecond/I");

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  sigaction(SIGINT, &sigIntHandler, NULL);

  int32_t handle;
  uint32_t n = 0;
  try
    {
      checkApiCall(CAENVME_Init(cvV1718, 0, 0, &handle),"CAENVME_Init");
      checkApiCall(MQDC32_Setup(handle),"MQDC32_Setup");
      checkApiCall(VX1290A_Setup(handle),"VX1290A_Setup");
      while (n < NumEvents)
	{
	  if (n % (NumEvents/100) == 0) std::cout << "Reading event " << n << std::endl;
	  unsigned char mask = 0;
	  while (!(mask & (1<<(cvIRQ1-1))))
	    {
	      checkApiCall(CAENVME_IRQCheck(handle, &mask),"CAENVME_IRQCheck");
	    }
	  uint32_t vector;
	  checkApiCall(CAENVME_IACKCycle(handle,cvIRQ1,&vector,cvD16),"CAENVME_IACKCycle");

	  MQDC32_Header head;
	  MQDC32_Data data;
	  MQDC32_EoE eoe;
	  checkApiCall(MQDC32_ReadEvent(handle,&head,&data,&eoe,chans),"MQDC32_ReadEvent");
	  if (Verbose)
	    {
	      head.Print();
	      data.Print();
	      eoe.Print();
	    }
	  
	  adc = data.adc;
	  overflow = data.overflow;
	  channel = data.channel;
	  
	  now = std::chrono::system_clock::now();
	  dp = floor<days>(now);
	  ymd = year_month_day{dp};
	  time = make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now-dp));
	  year = (int)(ymd.year());
	  month = (unsigned)(ymd.month());
	  day = (unsigned)(ymd.day());
	  hour = time.hours().count();
	  minute = time.minutes().count();
	  second = time.seconds().count();
	  millisecond = time.subseconds().count();
	  
	  tree->Fill();
	  
	  checkApiCall(MQDC32_Reset_Data_Buffer(handle),"MQDC32_Reset_Data_Buffer");
	  ++n;
	  usleep(Delay);
	}
  checkApiCall(CAENVME_End(handle),"CAENVME_End");
    }
  catch (CVErrorCodes err)
    {
      switch(err) {
      case cvBusError : std::cout << "Bus error" << std::endl;
	break;
      case cvCommError : std::cout << "Comm error" << std::endl;
	break;
      case cvGenericError : std::cout << "Generic error" << std::endl;
	break;
      case cvInvalidParam : std::cout << "Invalid param" << std::endl;
	break;
      case cvTimeoutError : std::cout << "Timeout error" << std::endl;
	break;
      default : std::cout << "Success" << std::endl;
	break;
      }
    }
  catch (std::runtime_error err)
    {
      std::cout << err.what() << std::endl;
      std::cout << "Read " << n << " events" << std::endl;
    }
  
  tree->Write();
  fileout->Close();
  
  return 0;
}
