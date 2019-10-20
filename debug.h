#ifndef __debug_h_included__
#define  __debug_h_included__

// #define __IMU__DEBUG__

#ifdef __IMU__DEBUG__
#include <stdlib.h>
#include "LUFA/Drivers/Peripheral/Serial.h"
#define DPRINTF(...) printf (__VA_ARGS__)
#define DEBUGSPEED   2000000
#else
#define DPRINTF(...)
#endif

#endif
