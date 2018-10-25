#include "PulseDaq.h"

int main(int argc, char ** argv)
{
  // command line parameters
  Settings set;

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
	  set.SetNumEvents(std::stod(argv[i+1]));
	  ++i;
	}
      else if (!strcmp(argv[i],"-d"))
	{
	  if (i+2 > argc)
	    {
	      std::cout << "need to specify a delay (in microseconds)!" << std::endl;
	      return 0;
	    }
	  set.SetDelay(std::stod(argv[i+1]));
	  ++i;
	}
      else if (!strcmp(argv[i],"-v"))
	{
	  set.SetVerbose(true);
	}
      else if (!strcmp(argv[i],"-i"))
	{
	  if (i+2 > argc)
	    {
	      std::cout << "need to specify an averaging window width!" << std::endl;
	      return 0;
	    }
	  set.SetInteractive(std::stod(argv[i+1]));
	  ++i;
	}
      else if (!strcmp(argv[i],"-tdc"))
	{
	  set.SetUseTDC(true);
	}
    }
  if (!(set.IsValid()))
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
  TString filename = TString::Format("DAQ_%04i%02i%02i_%02i%02u%02u.root",(int)(ymd.year()),(unsigned)(ymd.month()),(unsigned)(ymd.day()),(unsigned int)(time.hours().count()),(unsigned int)(time.minutes().count()),(unsigned int)(time.seconds().count()));

  std::cout << "Started run: " << (int)(ymd.year()) << "/" << (unsigned)(ymd.month()) << "/" << (unsigned)(ymd.day()) << " " << (unsigned)(time.hours().count()) << ":" << (unsigned)(time.minutes().count()) << ":" << (unsigned)(time.seconds().count()) << std::endl;

  TFile * fileout = TFile::Open(filename,"RECREATE");
  UInt_t qdc_adc;
  Double_t qdc_pC;
  UInt_t qdc_overflow;
  UInt_t qdc_channel;
  UInt_t qdc_timestamp;
  UInt_t tdc_tdc1;
  UInt_t tdc_tdc2;
  UInt_t tdc_channel1;
  UInt_t tdc_channel2;
  UInt_t tdc_timestamp;
  Double_t qdc_charge;
  Double_t risetime;
  Int_t year;
  Int_t month;
  Int_t day;
  Int_t hour;
  Int_t minute;
  Int_t second;
  Int_t millisecond;
  TTree * tree = new TTree("pulse","Pulse Data");
  tree->Branch("qdc_adc",&qdc_adc,"qdc_adc/i");
  tree->Branch("qdc_pC",&qdc_pC,"qdc_pC/D");
  tree->Branch("qdc_overflow",&qdc_overflow,"qdc_overflow/i");
  tree->Branch("qdc_channel",&qdc_channel,"qdc_channel/i");
  tree->Branch("qdc_timestamp",&qdc_timestamp,"qdc_timestamp/i");
  tree->Branch("tdc_tdc1",&tdc_tdc1,"tdc_tdc1/i");
  tree->Branch("tdc_tdc2",&tdc_tdc2,"tdc_tdc2/i");
  tree->Branch("tdc_channel1",&tdc_channel1,"tdc_channel1/i");
  tree->Branch("tdc_channel2",&tdc_channel2,"tdc_channel2/i");
  tree->Branch("tdc_timestamp",&tdc_timestamp,"tdc_timestamp/i");
  tree->Branch("qdc_charge",&qdc_charge,"qdc_charge/D");
  tree->Branch("risetime",&risetime,"risetime/D");
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

  if (set.UseInteractive()) std::cout << "Displaying average of " << set.Interactive() << " pulses." << std::endl;
  std::deque<Double_t> windowQDC;
  std::deque<Double_t> windowTDC;

  Double_t mqdc, sqdc, mqdc_last, sqdc_last;
  Double_t mtdc, stdc, mtdc_last, stdc_last;

  Double_t ADCtopC = 500.0/3840.0;

  try
    {
      std::cout << "Initializing V1718..." << std::endl;
      checkApiCall(CAENVME_Init(cvV1718, 0, set.VX1718_USB_CHANNEL(), &handle),"CAENVME_Init");
      checkApiCall(CAENVME_SystemReset(handle),"CAENVME_SystemReset");
      usleep(1000000);
      std::cout << "             MQDC32..." << std::endl;
      checkApiCall(MQDC32_Setup(handle),"MQDC32_Setup");
      if (set.UseTDC())
	{
	  std::cout << "             VX1290A..." << std::endl;
	  checkApiCall(VX1290A_Setup(handle,set),"VX1290A_Setup");
	}
      
      while (n < set.NumEvents())
	{
	  if ((set.NumEvents()<100 || set.Delay() > 100000 || n%(set.NumEvents()/100)==0) && !(set.UseInteractive()))
	    std::cout << "\rReading event " << n << std::flush;
	  
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
	  checkApiCall(MQDC32_ReadEvent(handle,&head,&data,&eoe,set),"MQDC32_ReadEvent");

	  // Read VX1290A (which was triggered by the output of MQDC32)
	  VX1290A_GlobalHeader gh;
	  VX1290A_GlobalTrailer gt;
	  std::vector<VX1290A_TDCHeader> th;
	  std::vector<VX1290A_TDCMeasurement> tm;
	  std::vector<VX1290A_TDCError> te;
	  std::vector<VX1290A_TDCTrailer> tt;
	  VX1290A_GlobalTrigTime gtt;
	  if (set.UseTDC())
	    {
	      checkApiCall(VX1290A_ReadEvent(handle,&gh,&gt,&th,&tm,&te,&tt,&gtt,set),"VX1290A_ReadEvent");
	    }

	  /*
	  // Print parsed words if desired
	  if (set.Verbose())
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
	  */

	  
	  /////////////////////////////////////////////
	  // Acquire data here and fill output TTree //
	  /////////////////////////////////////////////
	  
	  std::vector<uint32_t> measuredQDCchannels;
	  for (auto d : data)
	    {
	      measuredQDCchannels.push_back(d.channel);
	    }
	  bool correctQDCchannels = (measuredQDCchannels.size() == set.MQDC32_CHANNEL_CHARGE().size());
	  for (auto d : set.MQDC32_CHANNEL_CHARGE())
	    {
	      int count = 0;
	      for (auto mc : measuredQDCchannels)
		{
		  if (mc == d) ++count;
		}
	      correctQDCchannels = correctQDCchannels && (count==1);
	    }
	  // Here, correctQDCchannels should be true if good data

	  std::vector<uint32_t> measuredTDCchannels;
	  for (auto t : tm)
	    {
	      measuredTDCchannels.push_back(t.channel);
	    }
	  bool correctTDCchannels = (measuredTDCchannels.size() == set.VX1290A_CHANNEL_LE().size()+set.VX1290A_CHANNEL_MAX().size()) && set.UseTDC();
	  for (auto d : set.VX1290A_CHANNEL_LE())
	    {
	      int count = 0;
	      for (auto mt : measuredTDCchannels)
		{
		  if (d == mt) ++count;
		}
	      correctTDCchannels = correctTDCchannels && (count==1);
	    }
	  for (auto d : set.VX1290A_CHANNEL_MAX())
	    {
	      int count = 0;
	      for (auto mt : measuredTDCchannels)
		{
		  if (d == mt) ++count;
		}
	      correctTDCchannels = correctTDCchannels && (count==1);
	    }
	  // Here, correctTDCchannels should be true if TDC is enabled and good data.

	  if (correctQDCchannels)
	    {
	      for (size_t id = 0; id < data.size(); ++id)
		{
		  qdc_adc = data[id].adc;
		  qdc_pC = qdc_adc*ADCtopC;
		  qdc_overflow = data[id].overflow;
		  qdc_channel = data[id].channel;
		  qdc_timestamp = eoe.timestamp;
		  qdc_charge = qdc_adc*ADCtopC;

		  if (correctTDCchannels)
		    {
		      size_t tmlocLE, tmlocMAX;
		      for (size_t it = 0; it < tm.size(); ++it)
			{
			  if (tm[it].channel == set.VX1290A_CHANNEL_LE()[id]) tmlocLE=it;
			  else if (tm[it].channel == set.VX1290A_CHANNEL_MAX()[id]) tmlocMAX=it;
			}
		      tdc_tdc1 = tm[tmlocLE].tdc_meas;
		      tdc_tdc2 = tm[tmlocMAX].tdc_meas;
		      tdc_channel1 = tm[tmlocLE].channel;
		      tdc_channel2 = tm[tmlocMAX].channel;
		      tdc_timestamp = gtt.trig_time;
		    }
		  else
		    {
		      tdc_tdc1 = 0;
		      tdc_tdc2 = 0;
		      tdc_channel1 = 0;
		      tdc_channel2 = 0;
		      tdc_timestamp = 0;
		    }
		  risetime = (std::max(tdc_tdc1,tdc_tdc2)-std::min(tdc_tdc1,tdc_tdc2))*0.025;

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
		  
		  if (!set.UseTDC() || (set.UseTDC() && correctTDCchannels)) tree->Fill();

		  if (data[id].channel == set.MQDC32_CHANNEL_CHARGE()[0])
		    {
		      if (n == 0) 
			{
			  mqdc = qdc_charge;
			  sqdc = 0;
			  mtdc = risetime;
			  stdc = 0;
			}
		      else
			{
			  mqdc_last = mqdc;
			  mqdc = mqdc_last + (qdc_charge - mqdc_last)/n;
			  sqdc_last = sqdc;
			  sqdc = sqdc_last + (qdc_charge - mqdc_last)*(qdc_charge - mqdc);
			  mtdc_last = mtdc;
			  mtdc = mtdc_last + (risetime - mtdc_last)/n;
			  stdc_last = stdc;
			  stdc = stdc_last + (risetime - mtdc_last)*(risetime - mtdc);
			}
		      
		      if (set.UseInteractive())
			{
			  windowQDC.push_back(qdc_charge);
			  windowTDC.push_back(risetime);
			  if (windowQDC.size() > set.Interactive())
			    {
			      windowQDC.pop_front();
			      windowTDC.pop_front();
			      if (n % set.Interactive() == 0) std::cout << "\rPulse " << n << ": Charge (pC) = " << std::setw(7) << std::setprecision(2) << std::fixed << TMath::Mean(windowQDC.begin(),windowQDC.end()) << " +/- " << std::setw(7) << std::setprecision(2) << std::fixed << TMath::StdDev(windowQDC.begin(),windowQDC.end()) << ",  Rise Time (ns) = " << std::setw(6) << std::setprecision(2) << std::fixed << TMath::Mean(windowTDC.begin(),windowTDC.end()) << " +/- " << std::setw(6) << std::setprecision(2) << std::fixed << TMath::StdDev(windowTDC.begin(),windowTDC.end()) << std::flush;
			    }
			}
		      if ((correctQDCchannels && !set.UseTDC()) || (correctQDCchannels && set.UseTDC() && correctTDCchannels)) ++n;
		      usleep(set.Delay());    
		    }
		}
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
  
  std::cout << "\nRead " << n << " events" << std::endl;
  std::cout << "Error in " << nT-n << " events" << std::endl;

  std::cout << "Mean Charge (pC) = " << std::setw(7) << std::setprecision(2) << std::fixed << mqdc << " +/- " << std::setw(7) << std::setprecision(2) << std::fixed << TMath::Sqrt((n>1)?sqdc/(n-1):0) << "  Mean Rise Time (ns) = " << std::setw(6) << std::setprecision(2) << std::fixed << mtdc << " +/- " << std::setw(6) << std::setprecision(2) << std::fixed << TMath::Sqrt((n>1)?stdc/(n-1):0) << std::endl;

  now = std::chrono::system_clock::now();
  dp = floor<days>(now);
  ymd = year_month_day{dp};
  time = make_time(std::chrono::duration_cast<std::chrono::milliseconds>(now-dp));

  std::cout << "Finished run: " << (int)(ymd.year()) << "/" << (unsigned)(ymd.month()) << "/" << (unsigned)(ymd.day()) << " " << (unsigned)(time.hours().count()) << ":" << (unsigned)(time.minutes().count()) << ":" << (unsigned)(time.seconds().count()) << std::endl;

  tree->Write();
  fileout->Close();
  
  return 0;
}
