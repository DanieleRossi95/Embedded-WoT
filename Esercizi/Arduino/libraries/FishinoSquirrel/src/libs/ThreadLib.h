#ifndef __THREAD_LIB_H
#define __THREAD_LIB_H

#include "../Squirrel.h"

#ifdef __cplusplus
extern "C" {
#endif

SQUIRREL_API SQInteger registerThreadLib(HSQUIRRELVM v);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
