#ifndef __FISHINOSQUIRREL_ARDUINOLIB_H
#define __FISHINOSQUIRREL_ARDUINOLIB_H

#include <Arduino.h>

#include "../Squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQInteger registerArduinoLib(HSQUIRRELVM v) ;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
