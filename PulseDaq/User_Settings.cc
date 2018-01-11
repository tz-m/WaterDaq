#include "User_Settings.h"

Settings::Settings()
  : setVerbose(false),
    setNumEvents(false),
    setDelay(false),
    setInteractive(false),
    setTDC(false),
    verb(false),
    num(1),
    del(0),
    inter(1),
    tdc(false)
{}

bool Settings::IsValid()
{
  // verbose is optional, so not a requirement for validation
  return (setNumEvents && setDelay);
}


void Settings::SetVerbose(bool v)
{
  setVerbose = true;
  verb = v;
}

void Settings::SetNumEvents(uint32_t n)
{
  setNumEvents = true;
  num = n;
}

void Settings::SetDelay(uint32_t d)
{
  setDelay = true;
  del = d;
}

void Settings::SetInteractive(uint32_t i)
{
  setInteractive = true;
  inter = i;
}

void Settings::SetUseTDC(bool d)
{
  setTDC = true;
  tdc = d;
}

bool Settings::Verbose()
{
  return verb;
}

uint32_t Settings::NumEvents()
{
  return num;
}

uint32_t Settings::Delay()
{
  return del;
}

uint32_t Settings::Interactive()
{
  return inter;
}

bool Settings::UseInteractive()
{
  return setInteractive;
}

bool Settings::UseTDC()
{
  return tdc;
}
