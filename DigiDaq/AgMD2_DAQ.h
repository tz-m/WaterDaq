#include <chrono>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <signal.h>
#include <sstream>
#include <exception>
#include <time.h>
#include "AgMD2.h"

#include "TCanvas.h"
#include "TH1.h"
#include "TApplication.h"
#include "TGaxis.h"

#include "RunParams.h"
#include "ChannelParams.h"

class AgMD2_DAQ
{
 public:
  AgMD2_DAQ(){ };
  int app();
  void initialize_parameters();
  void calibrate();
  void configure_triggers();
  void configure_acquisition();
  void configure_channels();
  void initialize_driver();
  void checkApiCall(ViStatus status, char const* functionName);
  void Quit();
  ViSession GetSession() { return sess; }

 private:
  ViSession sess;
  void SetSession(ViSession s) {sess = s; }
  RunParams rp;
  std::map<ViUInt8, ChannelParams> cpm;

  struct Header
  {
    ViInt64 memsize;
    ViInt64 actualPoints;
    ViInt64 firstValidPoint;
    ViReal64 initialXOffset;
    ViReal64 initialXTimeSeconds;
    ViReal64 initialXTimeFraction;
    ViReal64 xIncrement;
    ViReal64 scaleFactor;
    ViReal64 scaleOffset;
    ViUInt8 channelNumber;
    int eventNumber;
    std::chrono::system_clock::time_point trigTime;
  };

};


