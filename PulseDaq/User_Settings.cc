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
{
}

bool Settings::IsValid()
{
  // verbose is optional, so not a requirement for validation
  return (setNumEvents && setDelay && setConfigFile);
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

void Settings::SetConfigFile(std::string filename)
{
  setConfigFile = true;
  ReadConfigFile(filename);
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

void Settings::ReadConfigFile(std::string fname="config.txt")
{
  std::string line;
  std::ifstream configfile(fname.c_str(),std::ifstream::in);
  if (configfile.is_open())
    {
      while (getline(configfile,line))
	{
	  if (line.size()<=1 || line.at(0)=='#') continue;

	  size_t divider = line.find_first_of(' ');
	  std::string parname = line.substr(0,divider);

	  std::string vals = line.substr(divider+1,std::string::npos);

	  std::vector<uint32_t> valvec;
	  std::string val;
	  std::istringstream valstream(vals);
	  while (getline(valstream,val,' '))
	    {
	      valvec.push_back(stoul(val,nullptr,16));
	    }
	  
	  if (parname == "VX1718_USB_CHANNEL")
	    {
	      config_vx1718_usb_channel = valvec.at(0);
	    }
	  else if (parname == "MQDC32_BASE")
	    {
	      config_mqdc32_base = valvec.at(0);
	    }
	  else if (parname == "MQDC32_CHANNEL_CHARGE")
	    {
	      for (auto v : valvec)
		{
		  config_mqdc32_channel_charge.push_back(v);
		}					 
	    }
	  else if (parname == "VX1290A_BASE")
	    {
	      config_vx1290a_base = valvec.at(0);
	    }
	  else if (parname == "VX1290A_CHANNEL_LE")
	    {
	      for (auto v : valvec)
		{
		  config_vx1290a_channel_le.push_back(v);
		}
	    }
	  else if (parname == "VX1290A_CHANNEL_MAX")
	    {
	      for (auto v : valvec)
		{
		  config_vx1290a_channel_max.push_back(v);
		}
	    }
	  else if (parname == "VX1290A_WINDOW_WIDTH")
	    {
	      config_vx1290a_window_width = valvec.at(0);
	    }
	  else if (parname == "VX1290A_WINDOW_OFFSET")
	    {
	      config_vx1290a_window_offset = valvec.at(0);
	    }
	}
    }

  std::cout << "Reading channels from MQDC32: ";
  for (auto c : MQDC32_CHANNEL_CHARGE())
    {
      std::cout << c << " ";
    }
  std::cout << std::endl;
}
