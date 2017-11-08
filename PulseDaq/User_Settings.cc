#include "User_Settings.h"

Settings::Settings()
  : setVerbose(false),
    setNumEvents(false),
    setDelay(false),
    verb(false),
    num(1),
    del(0)
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
