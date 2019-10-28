#ifndef __FISHINOSQUIRREL_SPILIB_H
#define __FISHINOSQUIRREL_SPILIB_H

#include <Arduino.h>

#include "../Squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQInteger registerSPILib(HSQUIRRELVM v) ;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
