#ifndef MESYDAQ_H
#define MESYDAQ_H

#include "MQDC32.h"

#include <iostream>
#include <unistd.h>
#include <string>
#include <signal.h>

#include "TFile.h"
#include "TTree.h"

#include "CAENVMElib.h"

#include "../date/include/date/date.h"

uint32_t GetN(uint32_t val, uint32_t offset, uint32_t N)
{
  return ((val>>offset) & ((1<<N)-1));
}

bool IsHeader(uint32_t word)
{
  if (GetN(word, 30, 2) == 1) return true;
  return false;
}

bool IsData(uint32_t word)
{
  if ((GetN(word, 30, 2) == 0) && (GetN(word, 21, 9) == 32)) return true;
  return false;
}

bool IsEoE(uint32_t word)
{
  if (GetN(word, 30, 2) == 3) return true;
  return false;
}

struct Header {
  uint32_t num_words;
  uint32_t fill;
  uint32_t module_id;
  uint32_t subheader;
  uint32_t hsig;
  void Print() { 
    std::cout << "Header: num_words=" << std::setw(4) << num_words 
	      << " fill=" << std::setw(1) << fill 
	      << " module_id=" << std::setw(3) << module_id 
	      << " subheader=" << std::setw(2) << subheader 
	      << " hsig=" << std::setw(1) << hsig << std::endl; 
  }
};

void ParseHeaderWord(uint32_t word, Header * head)
{
  head->num_words = GetN(word,0,12);
  head->fill = GetN(word,12,3);
  head->module_id = GetN(word,16,8);
  head->subheader = GetN(word,24,6);
  head->hsig = GetN(word,30,2);
}

struct Data {
  uint32_t adc;
  uint32_t overflow;
  uint32_t channel;
  uint32_t fix;
  uint32_t dsig;
  void Print() { 
    std::cout << "adc=" << std::setw(4) << adc 
	      << " overflow=" << std::setw(1) << overflow 
	      << " channel=" << std::setw(2) << channel 
	      << " fix=" << std::setw(3) << fix 
	      << " dsig=" << std::setw(1) << dsig << std::endl; 
  }
};

void ParseDataWord(uint32_t word, Data * data)
{
  data->adc = GetN(word,0,12);
  data->overflow = GetN(word,15,1);
  data->channel = GetN(word,16,5);
  data->fix = GetN(word,21,9);
  data->dsig = GetN(word,30,2);
}

struct EoE {
  uint32_t timestamp;
  uint32_t esig;
  void Print() { 
    std::cout << "timestamp=" << std::setw(10) << timestamp 
	      << " esig=" << std::setw(1) << esig << std::endl; 
  }
};

void ParseEoEWord(uint32_t word, EoE * eoe)
{
  eoe->timestamp = GetN(word,0,30);
  eoe->esig = GetN(word,30,2);
}

struct EnableChannel {
  std::vector<bool> ch;
EnableChannel() : ch(32,false) {}
  bool q(uint32_t c) {
    if (c <= 31) return ch.at(c);
    return false;
  }
};

void checkApiCall(CVErrorCodes err, std::string s)
{
  if (err == cvSuccess) return;
  else 
    {
      std::cout << "Error in " << s << std::endl;
      throw err;
    }
}

void handler(int s)
{
  throw std::runtime_error(s+": Ctrl+C");
}


#endif
