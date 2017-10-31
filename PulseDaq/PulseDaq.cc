#include "PulseDaq.h"

int main(int argc, char ** argv)
{
  // command line parameters
  uint32_t NumEvents;
  uint32_t Delay = 0.0;
  bool Verbose = false;

  // flags for required command line args
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
    }
  if (!(numflag && delayflag))
    {
      std::cout << "I need more information!!!" << std::endl
		<< "  Tell me how many events to record:" << std::endl
		<< "     -n [number]" << std::endl
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
  UInt_t qdc_adc;
  UInt_t qdc_overflow;
  UInt_t qdc_channel;
  UInt_t qdc_timestamp;
  UInt_t tdc_tdc1;
  UInt_t tdc_tdc2;
  UInt_t tdc_channel1;
  UInt_t tdc_channel2;
  UInt_t tdc_timestamp;
  Int_t year;
  Int_t month;
  Int_t day;
  Int_t hour;
  Int_t minute;
  Int_t second;
  Int_t millisecond;
  TTree * tree = new TTree("pulse","Pulse Data");
  tree->Branch("qdc_adc",&qdc_adc,"qdc_adc/i");
  tree->Branch("qdc_overflow",&qdc_overflow,"qdc_overflow/i");
  tree->Branch("qdc_channel",&qdc_channel,"qdc_channel/i");
  tree->Branch("qdc_timestamp",&qdc_timestamp,"qdc_timestamp/i");
  tree->Branch("tdc_tdc1",&tdc_tdc1,"tdc_tdc1/i");
  tree->Branch("tdc_tdc2",&tdc_tdc2,"tdc_tdc2/i");
  tree->Branch("tdc_channel1",&tdc_channel1,"tdc_channel1/i");
  tree->Branch("tdc_channel2",&tdc_channel2,"tdc_channel2/i");
  tree->Branch("tdc_timestamp",&tdc_timestamp,"tdc_timestamp/i");
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
  uint32_t nT = 0;
  try
    {
      std::cout << "Initializing V1718..." << std::endl;
      checkApiCall(CAENVME_Init(cvV1718, 0, 0, &handle),"CAENVME_Init");
      checkApiCall(CAENVME_SystemReset(handle),"CAENVME_SystemReset");
      usleep(1000000);
      std::cout << "             MQDC32..." << std::endl;
      checkApiCall(MQDC32_Setup(handle),"MQDC32_Setup");
      std::cout << "             VX1290A..." << std::endl;
      checkApiCall(VX1290A_Setup(handle),"VX1290A_Setup");

      while (n < NumEvents)
	{
	  if (NumEvents<100 || Delay>100000 || n%(NumEvents/100)==0)
	    std::cout << "Reading event " << n << std::endl;
	  
	  // The MQDC32 emits an IRQ1 when data is ready, so check for it
	  unsigned char mask = 0;
	  while (!(mask & (1<<(cvIRQ1-1))))
	    {
	      checkApiCall(CAENVME_IRQCheck(handle, &mask),"CAENVME_IRQCheck");
	    }
	  
	  // When the MQDC32 IRQ1 is found, acknowledge it and continue reading data
	  uint32_t vector;
	  checkApiCall(CAENVME_IACKCycle(handle,cvIRQ1,&vector,cvD16),"CAENVME_IACKCycle");

	  // Read MQDC32 output buffer and store in structs
	  MQDC32_Header head;
	  std::vector<MQDC32_Data> data;
	  MQDC32_EoE eoe;
	  checkApiCall(MQDC32_ReadEvent(handle,&head,&data,&eoe),"MQDC32_ReadEvent");
	 
	  // Read VX1290A (which was triggered by the output of MQDC32)
	  VX1290A_GlobalHeader gh;
	  VX1290A_GlobalTrailer gt;
	  std::vector<VX1290A_TDCHeader> th;
	  std::vector<VX1290A_TDCMeasurement> tm;
	  std::vector<VX1290A_TDCError> te;
	  std::vector<VX1290A_TDCTrailer> tt;
	  VX1290A_GlobalTrigTime gtt;
	  checkApiCall(VX1290A_ReadEvent(handle,&gh,&gt,&th,&tm,&te,&tt,&gtt),"VX1290A_ReadEvent");

	  // Print parsed words if desired
	  if (Verbose)
	    {
	      head.Print();
	      for (auto i : data) i.Print();
	      eoe.Print();
	      gh.Print();
	      gtt.Print();
	      for (auto i : th) i.Print();
	      for (auto i : tm) i.Print();
	      for (auto i : te) i.Print();
	      for (auto i : tt) i.Print();
	      gt.Print();
	    }
	  
	  /////////////////////////////////////////////
	  // Acquire data here and fill output TTree //
	  /////////////////////////////////////////////
	  if (data.size() == 1 && data[0].channel == MQDC32_CHANNEL_CHARGE &&
	      tm.size() == 2 && ((tm[0].channel == VX1290A_CHANNEL_LE && tm[1].channel == VX1290A_CHANNEL_MAX) || (tm[0].channel == VX1290A_CHANNEL_MAX && tm[1].channel == VX1290A_CHANNEL_LE)))
	    {
	      qdc_adc = data[0].adc;
	      qdc_overflow = data[0].overflow;
	      qdc_channel = data[0].channel;
	      qdc_timestamp = eoe.timestamp;
	      
	      if (tm[0].channel == VX1290A_CHANNEL_LE)
		{
		  tdc_tdc1 = tm[0].tdc_meas;
		  tdc_tdc2 = tm[1].tdc_meas;
		  tdc_channel1 = tm[0].channel;
		  tdc_channel2 = tm[1].channel;
		}
	      else if (tm[1].channel == VX1290A_CHANNEL_LE)
		{
		  tdc_tdc1 = tm[1].tdc_meas;
		  tdc_tdc2 = tm[0].tdc_meas;
		  tdc_channel1 = tm[1].channel;
		  tdc_channel2 = tm[0].channel;
		}
	      tdc_timestamp = gtt.trig_time;
	      
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

	      ++n;
	      usleep(Delay);
	    }
	  ///////////////////////////////////////////
	  // Finished writing data to output TTree //
	  ///////////////////////////////////////////


	  // Reset data buffer of MQDC32
	  checkApiCall(MQDC32_Reset_Data_Buffer(handle),"MQDC32_Reset_Data_Buffer");
	  // Do not reset VX1290A here. No point...

	  // Increment
	  ++nT;
	}
      checkApiCall(CAENVME_End(handle),"CAENVME_End");
    }
  catch (CVErrorCodes err)
    {
      switch(err) {
      case cvBusError : std::cout << "Bus error" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      case cvCommError : std::cout << "Comm error" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      case cvGenericError : std::cout << "Generic error" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      case cvInvalidParam : std::cout << "Invalid param" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      case cvTimeoutError : std::cout << "Timeout error" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      default : std::cout << "Success" << std::endl;
	checkApiCall(CAENVME_End(handle),"CAENVME_End");
	break;
      }
    }
  catch (std::runtime_error err)
    {
      std::cout << err.what() << std::endl;
      checkApiCall(CAENVME_End(handle),"CAENVME_End");
    }
  
  std::cout << "Read " << n << " events" << std::endl;
  std::cout << "Error in " << nT-n << " events" << std::endl;
  tree->Write();
  fileout->Close();
  
  return 0;
}
