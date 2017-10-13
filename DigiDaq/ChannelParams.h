#include "AgMD2.h"
#include <cstdio>
#include <cstring>

class ChannelParams 
{
 private:
  ViBoolean useChannel; bool setUC = false;
  ViUInt8 channelNumber; bool setCNum = false;
  ViChar channelName[16]; bool setCNam = false;
  ViChar channelNickname[16]; bool setCNik = false;
  ViInt16 channelPolarity; bool setCP = false;
  ViReal64 channelRange; bool setCR = false;
  ViReal64 channelOffset; bool setCO = false;
  ViReal64 triggerLevel; bool setTL = false;
  ViInt32 triggerSlope; bool setTSl = false;
  ViChar triggerSource[16]; bool setTSo = false;
  ViBoolean activeTrigger; bool setAT = false;
  bool updated;
  
 public:
  ChannelParams(ViUInt8 num) 
    {
      channelNumber = num; 
      setCNum = true;
      if (num <= 0 || num >= 9)
	{
	  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	  printf("Channel number %i is invalid.\n",num);
	  printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	  exit(-1);
	}
      sprintf(channelName,"Channel%u",num); 
      setCNam = true;
      sprintf(triggerSource,"Internal%u",num); 
      setTSo = true;
      updated = true;
    }
  
  ViBoolean GetUseChannel() { return useChannel; }
  ViUInt8 GetChannelNumber() { return channelNumber; }
  ViChar * GetChannelName() { return channelName; }
  ViChar * GetChannelNickname() { return channelNickname; }
  ViInt16 GetChannelPolarity() { return channelPolarity; }
  ViReal64 GetChannelRange() { return channelRange; }
  ViReal64 GetChannelOffset() { return channelOffset; }
  
  ViReal64 GetTriggerLevel() { return triggerLevel; }
  ViInt32 GetTriggerSlope() { return triggerSlope; }
  ViChar * GetTriggerSource() { return triggerSource; }
  ViBoolean GetActiveTrigger() { return activeTrigger; }
  
  ViBoolean Updated() { return updated; }
  void DoneUpdating() { updated = false; }

  void UpdateUseChannel(ViBoolean use) 
  { 
    useChannel = use; 
    setUC = true; 
    updated = true; 
  }
  
  void UpdateChannelPolarity(ViInt16 pol)
  {
    setCP = true;
    if (pol != 1 && pol != -1)
      {
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Channel polarity %i is invalid.\n",pol);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	channelPolarity = 1;
	setCP = false;
      }
    else { channelPolarity = pol; }
    updated = true;
  }
  
  void UpdateChannelRange(ViReal64 r)
  {
    setCR = true;
    if (std::abs(r) > 5.0)
      {
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Channel range must be less than 5.0\n");
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	channelRange = 5.0;
	setCR = false;
      }
    else { channelRange = std::abs(r); }
    updated = true;
  }

  void UpdateChannelOffset(ViReal64 o)
  {
    // i don't know the limits for this...probably should check
    channelOffset = o;
    setCO = true;
    updated = true;
  }

  void UpdateTriggerLevel(ViReal64 tl)
  {
    triggerLevel = tl;
    setTL = true;
    updated = true;
  }

  void UpdateTriggerSlope(ViInt32 ts)
  {
    setTSl = true;
    if (ts != 0 && ts != 1)
      {
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Trigger slope must be either 0 (negative) or 1 (positive).\n");
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	triggerSlope = 1;
	setTSl = false;
      }
    else { triggerSlope = ts; }
    updated = true;
  }

  void UpdateChannelNickname(char * nn)
  {
    sprintf(channelNickname,"%s",nn);
    setCNik = true;
  }

  void UpdateActiveTrigger(ViBoolean at) 
  { 
    activeTrigger = at; 
    setAT = true;
    updated = true; 
  }

  bool Complete() { return setUC && setCNum && setCNam && setCNik && setCP && setCR && setCO && setTL && setTSl && setTSo && setAT; }
  
  void Print()
  {
    printf("Incomplete or invalid ChannelParams:\n");
    if (!setUC)   printf("  UseChannel\n");
    if (!setCNum) printf("  Channel number\n");
    if (!setCNam) printf("  Channel name\n");
    if (!setCNik) printf("  ChannelNickname\n");
    if (!setCP)   printf("  ChannelPolarity\n");
    if (!setCR)   printf("  ChannelRange\n");
    if (!setCO)   printf("  ChannelOffset\n");
    if (!setTL)   printf("  TriggerLevel\n");
    if (!setTSl)  printf("  TriggerSlope\n");
    if (!setTSo)  printf("  TriggerSource\n");
    if (!setAT)   printf("  ActiveTrigger\n");
  }
};
