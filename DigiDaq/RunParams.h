#include "AgMD2.h"
#include <string>

class RunParams
{
  std::string resourceName; bool setRN = false;
  ViInt64 numRecords; bool setNR = false;
  ViInt64 recordSize; bool setRS = false;
  ViReal64 sampleRate; bool setSR = false;
  // set timeout to -1 for no timeout
  ViInt32 timeoutInMS; bool setTIM = false;
  ViReal64 triggerDelay; bool setTD = false;
  ViConstString optChar; bool setOC = false;
  bool draw; bool setDRAW = false;
  ViReal64 dutyCycle; bool setDuty = false;

 public:
  RunParams() {}

  void Reset()
  {
    setRN = false;
    setNR = false;
    setRS = false;
    setSR = false;
    setTIM = false;
    setTD = false;
    setOC = false;
    setDRAW = false;
    setDuty = false;
  }
  
  std::string GetResourceName()   { return resourceName; }
  ViInt64 GetNumRecords()    { return numRecords;   }
  ViInt64 GetRecordSize()    { return recordSize;   }
  ViReal64 GetSampleRate()   { return sampleRate;   }
  ViInt32 GetTimeoutInMS()   { return timeoutInMS;  }
  ViReal64 GetTriggerDelay() { return triggerDelay; }
  ViConstString GetOptChar() { return optChar;      }
  bool GetDraw() { return draw; }
  ViReal64 GetDutyCycle() { return dutyCycle; }

  void SetResourceName(std::string rn)
  {
    resourceName = rn;
    setRN = true;
    optChar = "Simulate=false, DriverSetup= Model=U5309A";
    setOC = true;
  }
  void SetNumRecords(ViInt64 nr)     { numRecords = nr;   setNR = true;  }
  void SetRecordSize(ViInt64 rs)     { recordSize = rs;   setRS = true;  }
  void SetSampleRate(ViReal64 sr)    { sampleRate = sr;   setSR = true;  }
  void SetTimeoutInMS(ViInt32 t)     { timeoutInMS = t;   setTIM = true; }
  void SetTriggerDelay(ViReal64 td)  { triggerDelay = td; setTD = true;  }
  void SetDraw(bool d) { draw = d; setDRAW = true; }
  void SetDutyCycle(ViReal64 d) { dutyCycle = d; setDuty = true; }

  bool Complete() { return (setRN && setNR && setRS && setSR && setTIM && setTD && setOC && setDRAW && setDuty); }

  void Print()
  {
    printf("Incomplete or invalid RunParams:\n");
    if (!setRN)  printf("  ResourceName\n");
    if (!setNR)  printf("  NumRecords\n");
    if (!setRS)  printf("  RecordSize\n");
    if (!setSR)  printf("  SampleRate\n");
    if (!setTIM) printf("  TimeoutInMS\n");
    if (!setTD)  printf("  TriggerDelay\n");
    if (!setOC)  printf("  OptChar = %s\n",optChar);
    if (!setDRAW) printf("  Draw\n");
    if (!setDuty) printf("  DutyCycle\n");
    printf("\n");
  }
};
