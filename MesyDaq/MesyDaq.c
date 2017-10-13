#include "MesyDaq.h"

int main(int argc, char ** argv)
{
  TApplication app("daq",&argc,argv);
  app.ExitOnException();

  MesyDaq m;
  m.app();

  app.Run();

  return 0;
}

int MesyDaq::app()
{
  std::cout << "this is the app" << std::endl;
  return 1;
}
