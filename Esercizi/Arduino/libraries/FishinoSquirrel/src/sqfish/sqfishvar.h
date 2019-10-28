#ifndef __SQFISH__VAR__H
#define __SQFISH__VAR__H

#include "sqfish.h"

namespace sqfish
{
	
	template<typename C, typename T> bool  setMemberVariableHandler(VarAddr const &addr, HSQUIRRELVM vm)
	{
		// get class instance from stack
		C *instance = (C *)utils::getInstance(vm);
		
		// check the instance
		if(!instance)
			return false;
		
		// get the value
		T val = GetParam<T>(vm, -1);

		// casto address to member pointer
		T C::*memberPtr = (T C::*)addr.member;
		
		// set into class
		instance->*memberPtr = val;

		// pop key and value
		sq_pop(vm, 2);
		
		return true;
	}

	template<typename C, typename T> bool  getMemberVariableHandler(VarAddr const &addr, HSQUIRRELVM vm)
	{
		// get class instance from stack
		C *instance = (C *)utils::getInstance(vm);
		
		// check the instance
		if(!instance)
			return false;
		
		// casto address to member pointer
		T C::*memberPtr = (T C::*)addr.member;
		
		// get the variable
		T val = instance->*memberPtr;
		
		// pop the key
		sq_pop(vm, 1);
		
		// send it to squirrel
		PushValue(vm, val);
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename T> bool  setGlobalVariableHandler(VarAddr const &addr, HSQUIRRELVM vm)
	{
		// get the value
		T val = GetParam<T>(vm, -1);

		// casto address to variable
		T *globalPtr = (T *)addr.global;
		
		// set it
		*globalPtr = val;

		// pop key and value
		sq_pop(vm, 2);
		
		return true;
	}

	template<typename T> bool  getGlobalVariableHandler(VarAddr const &addr, HSQUIRRELVM vm)
	{
		// casto address to variable
		T *globalPtr = (T *)addr.global;
		
		// get the variable
		T val = *globalPtr;
		
		// pop the key
		sq_pop(vm, 1);
		
		// send it to squirrel
		PushValue(vm, val);
		
		return true;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// adds a class member variable
	template<typename C, typename T>bool  addVariable(HSQOBJECT &obj, LITSTR sqName, T C::*val, HSQUIRRELVM vm)
	{
		// retrieve (or create) a ClassInfo data inside a class
		ClassInfo *classInfo = ClassInfo::get(obj, vm);

		// get (or replace) the VarInfo item for this variable
		VarInfo *varInfo = classInfo->addVar(sqName);
		varInfo->setFunc = setMemberVariableHandler<C, T>;
		varInfo->getFunc = getMemberVariableHandler<C, T>;
		varInfo->varAddr.member = (SQFISH_MEMBER_VARIABLE)val;
		
		return true;
	}

	// adds a static member variable or a global variable
	template<typename T>bool  addVariable(HSQOBJECT &obj, LITSTR sqName, T *val, HSQUIRRELVM vm)
	{
		// retrieve (or create) a ClassInfo data inside a class
		ClassInfo *classInfo = ClassInfo::get(obj, vm);

		// get (or replace) the VarInfo item for this variable
		VarInfo *varInfo = classInfo->addVar(sqName);
		varInfo->setFunc = setGlobalVariableHandler<T>;
		varInfo->getFunc = getGlobalVariableHandler<T>;
		varInfo->varAddr.global = (SQFISH_GLOBAL_VARIABLE)val;
		
		return true;
	}

}; // end namespace sqfish

#endif