#include "../date/include/date/date.h"
#include <chrono>
#include "TFile.h"
#include "TTreeReader.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TDatime.h"

void Draw(std::string filename) 
{
  gROOT->Reset();

  TFile * f = TFile::Open(filename.c_str(),"READ");
  TTreeReader reader("pulse",f);
  TTreeReaderValue<UInt_t> b_qdc_adc(reader,"qdc_adc");
  TTreeReaderValue<UInt_t> b_tdc_tdc1(reader,"tdc_tdc1");
  TTreeReaderValue<UInt_t> b_tdc_tdc2(reader,"tdc_tdc2");
  TTreeReaderValue<Int_t> b_year(reader,"year");
  TTreeReaderValue<Int_t> b_month(reader,"month");
  TTreeReaderValue<Int_t> b_day(reader,"day");
  TTreeReaderValue<Int_t> b_hour(reader,"hour");
  TTreeReaderValue<Int_t> b_minute(reader,"minute");
  TTreeReaderValue<Int_t> b_second(reader,"second");
  TTreeReaderValue<Int_t> b_millisecond(reader,"millisecond");

  bool start = true;
  Int_t startyear, startmonth, startday, starthour, startminute, startsecond, startmillisecond;
  std::chrono::system_clock::time_point starttime;
  unsigned long long tm = 0;
  using namespace date;
  using namespace std::chrono;

  TGraph * gr_charge = new TGraph();
  TGraph * gr_riset = new TGraph();
  int n = 0;

  std::vector<Double_t> qdcvec;

  while (reader.Next())
    {
      if (start)
	{
	  startyear = *b_year;
	  startmonth = *b_month;
	  startday = *b_day;
	  starthour = *b_hour;
	  startminute = *b_minute;
	  startsecond = *b_second;
	  startmillisecond = *b_millisecond;
	  auto ymd = year(startyear)/startmonth/startday;
	  starttime = sys_days(ymd) + hours(starthour) + minutes(startminute) + seconds(startsecond) + milliseconds(startmillisecond);
	  start = false;
	}
      Int_t y,mo,d,h,mi,s,ms;
      y = *b_year; mo = *b_month; d = *b_day; 
      h = *b_hour; mi = *b_minute; s = *b_second; ms = *b_millisecond;
      auto ymd = year(y)/mo/d;
      std::chrono::system_clock::time_point tp = sys_days(ymd) + hours(h) + minutes(mi) + seconds(s) + milliseconds(ms);
      //std::cout << "tm=" << std::chrono::duration_cast<std::chrono::milliseconds>(tp-starttime) << std::endl;
      auto tdiff = std::chrono::duration_cast<std::chrono::minutes>(tp-starttime);
      //std::cout << "int tm=" << tdiff.count() << std::endl;
      TDatime datime(y,mo,d,h,mi,s);
      
      unsigned int risetime = (*b_tdc_tdc1 > *b_tdc_tdc2) ? *b_tdc_tdc1 - *b_tdc_tdc2 : *b_tdc_tdc2 - *b_tdc_tdc1;
      gr_charge->SetPoint(n,datime.Convert(),*b_qdc_adc);
      gr_riset->SetPoint(n,datime.Convert(),0.025*(risetime));
      ++n;
      qdcvec.push_back((Double_t)(*b_qdc_adc));     
    }

  std::cout << "Mean QDC = " << TMath::Mean(qdcvec.begin(),qdcvec.end()) << "  StdDev QDC = " << TMath::StdDev(qdcvec.begin(),qdcvec.end()) << "  Num Vals = " << qdcvec.size() << std::endl;

  TCanvas * canv = new TCanvas("c","c",1200,900);
  canv->Divide(1,2);
  canv->cd(1);
  gr_charge->SetMarkerColor(kBlue);
  gr_charge->SetMarkerStyle(20);
  gr_charge->SetMarkerSize(0.25);
  gr_charge->SetTitle(";Time (UTC);Pulse Charge (ADC)");
  gr_charge->Draw("AP");
  gPad->Modified(); gPad->Update();
  gr_charge->GetXaxis()->SetTimeDisplay(1);
  gr_charge->GetXaxis()->SetTimeFormat("%H:%M");
  gPad->Modified(); gPad->Update();
  canv->cd(2);
  gr_riset->SetMarkerColor(kRed);
  gr_riset->SetMarkerStyle(20);
  gr_riset->SetMarkerSize(0.25);
  gr_riset->SetTitle(";Time (UTC);Rise Time (ns)");
  gr_riset->Draw("AP");
  gPad->Modified(); gPad->Update();
  gr_riset->GetXaxis()->SetTimeDisplay(1);
  gr_riset->GetXaxis()->SetTimeFormat("%H:%M");
  gPad->Modified(); gPad->Update();
  
}
