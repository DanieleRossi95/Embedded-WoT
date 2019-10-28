#ifndef __SQFISH__H
#define __SQFISH__H

#include <stdlib.h>
#include <typeinfo>

#include "../Squirrel.h"

#ifdef UPP_TEST
	#include <string>
	#include <vector>
	#include <map>
	#include <list>
	#include <Core/Core.h>
	using namespace Upp;
#else
	#include <FishinoStl.h>
	#define DEBUG_LEVEL_INFO
	#include <FishinoDebug.h>
#endif

// enable/disable RLOGs
#define ENABLE_RLOG

#ifdef RLOG
	#ifndef ENABLE_RLOG
		#undef RLOG
		#define RLOG(x)
	#endif
#else
	#define RLOG(x)
#endif


#include "../squirrel/squtils.h"

#include "sqfishSmallVector.h"
#include "sqfishutil.h"
#include "sqfishStack.h"
#include "sqfishFuncTable.h"
#include "sqfishfunc.h"
#include "sqfishvar.h"
#include "sqfishclass.h"
#include "sqfishtable.h"

namespace sqfish
{
}; // end namespace sqfish

#endif
