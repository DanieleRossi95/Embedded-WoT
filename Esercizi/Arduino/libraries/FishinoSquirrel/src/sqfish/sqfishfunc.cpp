#include "sqfish.h"

namespace sqfish
{
	// squirrel function handler
	// this one is NOT a template, as the params-dependent code is handled by function handlers
	SQInteger squirrelFuncHandler(HSQUIRRELVM vm)
	{
		// get the function info passed as the free variable
		FunctionInfo const *functionInfo;
		if(!SQ_SUCCEEDED(sq_getuserpointer(vm, -1, (void **)&functionInfo)))
			return sq_throwerror(vm, _SC("couldn't get the function info data"));
		
		// pops the free var, leaving just args on stack
		sq_poptop(vm);

		// build signature from actual parameters
		SignatureType signature = utils::GetParamsSignatureFromStack(vm);
		
		// get the overload info from function info and signature
		OverloadInfo const *overloadInfo = functionInfo->getOverloadInfo(signature, false);
		
		// if not there, throw a squirrel exception
		if(!overloadInfo)
			return sq_throwerror(vm, _SC("wrong number of parameters or bad parameters"));
		
		// call the handler
		return overloadInfo->call(vm);
	}

}; // end sqfish namespace