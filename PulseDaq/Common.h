#ifndef COMMON_H
#define COMMON_H

#ifndef LINUX
#define LINUX
#endif

#include <sys/types.h>
#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <iomanip>
#include <signal.h>
#include <bitset>
#include <deque>
#include <fstream>
#include <sstream>

#include "../date/include/date/date.h"

#include "CAENVMElib.h"

#include "TMath.h"

uint32_t BitMask(uint32_t val, uint32_t offset, uint32_t N);
void handler(int s);
void checkApiCall(CVErrorCodes err, std::string s);

#endif
