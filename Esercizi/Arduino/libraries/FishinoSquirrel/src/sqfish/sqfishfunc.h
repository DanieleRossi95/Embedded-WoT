#ifndef __SQFISH__FUNC__H
#define __SQFISH__FUNC__H

#include <assert.h>
//#include <memory.h>

namespace sqfish
{
	// squirrel function handler
	// this one is NOT a template, as the params-dependent code is handled by function handlers
	SQInteger squirrelFuncHandler(HSQUIRRELVM vm);

	// generic handler for member functions (void RES version)
	template<typename C, typename RES, typename... PARMS> typename utils::enable_void<RES, SQInteger>::type  memberFuncHandler(RES(C::*func)(PARMS...), HSQUIRRELVM vm) {

		// get the instance pointer
		C *instance = (C *)utils::getInstance(vm);
		if(!instance)
			return sq_throwerror(vm, _SC("Member functions must be called on instances"));

		// calls the function
		StackBind(vm, instance, func);

		return 0;
	}

	// generic handler for member functions (non-void RES version)
	template<typename C, typename RES, typename... PARMS> typename utils::disable_void<RES, SQInteger>::type  memberFuncHandler(RES(C::*func)(PARMS...), HSQUIRRELVM vm) {

		// get the instance pointer
		C *instance = (C *)utils::getInstance(vm);
		if(!instance)
			return sq_throwerror(vm, _SC("Member functions must be called on instances"));

		// calls the function
		RES res = StackBind(vm, instance, func);

		// push return value
		PushValue<RES>(vm, res);
		return 1;
	}

	// add a member function and return its squirrel handle
	template<typename C, typename RES, typename... PARMS> bool  addMember(HSQOBJECT &container, LITSTR name, RES (C::*func)(PARMS...), HSQUIRRELVM vm) {

		// get the signature string for parameters
		SignatureID const *signature = utils::GetSignatureString<PARMS...>();
		
		// get info field from container
		ClassInfo *classInfo = ClassInfo::get(container, vm);

		// create function and overload info for this func
		FunctionInfo *functionInfo;
		OverloadInfo *overloadInfo;
		if(!classInfo->createFunctionInfo(name, signature, functionInfo, overloadInfo))
			return false;
		
		// setup function handlers
		overloadInfo->setMemberHandlers((SQFISH_FUNC_MEMBER_HANDLER)memberFuncHandler<C, RES, PARMS...>, (SQFISH_MEMBER_FUNCTION)func);
		
		// the _get metamethod will take care of building the closure
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// generic handler for global functions (void RES version)
	template<typename RES, typename... PARMS> typename utils::enable_void<RES, SQInteger>::type  globalFuncHandler(RES(*func)(PARMS...), HSQUIRRELVM vm) {

		// calls the function
		StackBind(vm, func);

		return 0;
	}

	// generic handler for global functions (non-void RES version)
	template<typename RES, typename... PARMS> typename utils::disable_void<RES, SQInteger>::type  globalFuncHandler(RES(*func)(PARMS...), HSQUIRRELVM vm) {

		// hack -- param evaluation order is compiler-dependent
		// find parameter evaluation order
		RES res = StackBind(vm, func);

		// push return value
		PushValue<RES>(vm, res);
		return 1;
	}
	
	template<typename RES, typename... PARMS> bool  addGlobal(HSQOBJECT &container, LITSTR name, RES(*func)(PARMS...), HSQUIRRELVM vm) {

		// get the signature string for parameters
		SignatureID const *signature = utils::GetSignatureString<PARMS...>();
		
		// get info field from container
		TableInfo *tableInfo = TableInfo::get(container, vm);
		
		// create function and overload info for this func
		FunctionInfo *functionInfo;
		OverloadInfo *overloadInfo;
		if(!tableInfo->createFunctionInfo(name, signature, functionInfo, overloadInfo))
			return false;
		
		// setup function handlers
		overloadInfo->setGlobalHandlers((SQFISH_FUNC_GLOBAL_HANDLER)globalFuncHandler<RES, PARMS...>, (SQFISH_GLOBAL_FUNCTION)func);

		// the _get metamethod will take care of building the closure
		return true;
	}

}; // end namespace sqfish

#endif
