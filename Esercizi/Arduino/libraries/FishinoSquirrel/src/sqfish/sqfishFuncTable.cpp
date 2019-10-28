#include "sqfish.h"

namespace sqfish
{
	////////////////////////////////////////// CLEANUP STUFFS /////////////////////////////////////////////////////////

	// cleanup for user data into table info
	static SQInteger _tableInfoCleanup(SQUserPointer ptr, SQInteger size)
	{
		TableInfo **info = (TableInfo **)ptr;
		if(*info)
			delete(*info);
		*info = NULL;
		return 0;
	}

	/////////////////////////////////////////////// OverloadInfo ///////////////////////////////////////////////////////

	// find an overload by signature
	OverloadInfo const *OverloadsInfo::get(SignatureType const &signature, bool strict) const
	{
		for(size_t i = 0; i < _overloads.getCount(); i++)
			if(!utils::CompareTypeSignatures(signature, _overloads[i].getSignature()))
				return &_overloads[i];
			
		// not found and requested exact match, return null
		if(strict)
			return NULL;
		
		// not found perfect match, try loosening types
		size_t foundIdx = -1;
		int maxDiff = signature.size();

		for(size_t i = 0; i < _overloads.getCount(); i++)
		{
			int diffs = utils::CompareTypeSignatures(signature, _overloads[i].getSignature());
			if(diffs >= 0 && diffs <= maxDiff)
			{
				maxDiff = diffs;
				foundIdx = i;
			}
		}
		if(foundIdx !=  (size_t)-1)
			return &_overloads[foundIdx];

		return NULL;
	}

	OverloadInfo const *OverloadsInfo::get(SignatureID const *signature) const
	{
		for(size_t i = 0; i < _overloads.getCount(); i++)
		{
			if(signature[0] != _overloads[i].getSignature()[0])
				continue;
			if(memcmp(signature + 1, _overloads[i].getSignature() + 1, signature[0]))
				continue;
			return &_overloads[i];
		}
		return NULL;
	}

	// create an overload by signature -- return NULL if already there
	OverloadInfo *OverloadsInfo::create(SignatureID const *signature)
	{
		// if already there, return NULL
		if(get(signature))
			return NULL;

		// add the new overload info
		return &_overloads.add(OverloadInfo(signature));
	}

	/////////////////////////////////////////////// FunctionInfo ///////////////////////////////////////////////////////

	// find an overload by signature
	OverloadInfo const *FunctionInfo::getOverloadInfo(SignatureType const &signature, bool strict) const
	{
		return _overloads.get(signature, strict);
	}

	// create an overload by signature -- return NULL if already there
	OverloadInfo *FunctionInfo::createOverloadInfo(SignatureID const *signature)
	{
		return _overloads.create(signature);
	}

	// create function and overload info for a function
	bool FunctionsInfo::create(LITSTR name, SignatureID const *signature, FunctionInfo *&functionInfo, OverloadInfo *&overloadInfo)
	{
		functionInfo = NULL;
		for(size_t i = 0; i < _functions.getCount(); i++)
			if(_functions[i].getName()->equal(name))
			{
				functionInfo = &_functions[i];
				break;
			}
		if(functionInfo == NULL)
			functionInfo = &_functions.add(FunctionInfo(name));

		// now create the overload info
		overloadInfo = functionInfo->createOverloadInfo(signature);
		return(overloadInfo != NULL);
	}
	
	// get a function by name
	FunctionInfo const *FunctionsInfo::get(const char *name) const
	{
		for(size_t i = 0; i < _functions.getCount(); i++)
		{
			if(!strcmp(_functions[i].getName()->str(), name))
				return &_functions[i];
		}
		return NULL;
	}
			
	///////////////////////////////////////////////// VarInfo ////////////////////////////////////////////////////////

	VarInfo *VarsInfo::add(LITSTR name)
	{
		// just in case... look if there's already the same var
		// shouldn't be necessary, but...
		for(size_t i = 0; i < _infos.getCount(); i++)
		{
			if(_infos[i].varName->equal(name))
				return &_infos[i];
		}
		return &_infos.add(VarInfo(name));
	}
	
	// get a variable from list
	VarInfo const *VarsInfo::get(const char *name) const
	{
		for(size_t i = 0; i < _infos.getCount(); i++)
		{
			if(_infos[i].varName->equal(name))
				return &_infos[i];
		}
		return NULL;
	}

	///////////////////////////////////////////////// TableInfo ////////////////////////////////////////////////////////

	// create function and overload info  a function
	bool TableInfo::createFunctionInfo(LITSTR name, SignatureID const *signature, FunctionInfo *&functionInfo, OverloadInfo *&overloadInfo)
	{
		return _functions.create(name, signature, functionInfo, overloadInfo);
	}
			
	// retrieve (or create) a TableInfo data inside the table
	TableInfo *TableInfo::get(HSQOBJECT &table, HSQUIRRELVM vm)
	{
		TableInfo *res = NULL;
		
		// push the table object
		sq_pushobject(vm, table);

		// first try to get the data
		sq_pushstring(vm, _SC("__sqfish__"), -1, SQTrue);
		if(SQ_SUCCEEDED(sq_rawget(vm, -2)))
		{
			// found, get the data
			TableInfo **info;
			if(!SQ_SUCCEEDED(sq_getuserdata(vm, -1, (void **)&info, NULL)))
			{
				RLOG("Error getting __sqfish__");
			}
			else
				res = *info;
			
			// pop the user data
			sq_poptop(vm);
		}
		else
		{
			// not found, create it
			sq_pushstring(vm, _SC("__sqfish__"), -1, SQTrue);
			TableInfo **info = (TableInfo **)sq_newuserdata(vm, sizeof(*info));
			res = new TableInfo;
			*info = res;
			sq_setreleasehook(vm, -1, _tableInfoCleanup);

			// this field shall be static
			// (not useful for tables, but needed for classes)
			if(!SQ_SUCCEEDED(sq_newslot(vm, -3, SQTrue)))
			{
				RLOG("FAILED TO CREATE __sqfish__ MEMBER");
			}
		}
		
		// pop the table object
		sq_poptop(vm);
		return res;
	}

	TableInfo *TableInfo::get(SQInteger sp, HSQUIRRELVM vm)
	{
		HSQOBJECT obj;
		sq_getstackobj(vm, sp, &obj);
		return get(obj, vm);
	}

	///////////////////////////////////////////////// ClassInfo ////////////////////////////////////////////////////////

	// get a constructor overload from signature
	OverloadInfo const *ClassInfo::getConstructorInfo(SignatureType const &signature) const
	{
		
		return _constructors.get(signature, false);
	}
	
	// create a constructor overload from signature
	OverloadInfo *ClassInfo::createConstructorInfo(SignatureID const *signature)
	{
		return _constructors.create(signature);
	}
	
	// retrieve a ClassInfo data inside a class
	ClassInfo *ClassInfo::get(HSQOBJECT &cls, HSQUIRRELVM vm)
	{
		ClassInfo *res = NULL;
		
		// push the class object
		sq_pushobject(vm, cls);

		// first try to get the data
		sq_pushstring(vm, _SC("__sqfish__"), -1, SQTrue);
		if(SQ_SUCCEEDED(sq_get(vm, -2)))
		{
			// found, get the data
			ClassInfo **info;
			if(!SQ_SUCCEEDED(sq_getuserdata(vm, -1, (void **)&info, NULL)))
			{
//				RLOG("Error getting __sqfish__");
			}
			else
				res = *info;
		}
		// pop the user data and the class
		sq_pop(vm, 2);
		return res;
	}
		
	ClassInfo *ClassInfo::get(SQInteger sp, HSQUIRRELVM vm)
	{
		HSQOBJECT obj;
		sq_getstackobj(vm, sp, &obj);
		return get(obj, vm);
	}

	// create a ClassInfo data inside a class
	ClassInfo *ClassInfo::create(HSQOBJECT &cls, HSQUIRRELVM vm)
	{
		ClassInfo *res = NULL;
		
		// push the class object
		sq_pushobject(vm, cls);
		
		// set user data size for class
		sq_setclassudsize(vm, -1, sizeof(InstanceInfo));

		// create the user data
		sq_pushstring(vm, _SC("__sqfish__"), -1, SQTrue);
		ClassInfo **info = (ClassInfo **)sq_newuserdata(vm, sizeof(*info));
		res = new ClassInfo;
		*info = res;
		sq_setreleasehook(vm, -1, _tableInfoCleanup);

		// this field shall be static
		// (not useful for tables, but needed for classes)
		sq_newslot(vm, -3, SQTrue);
		
		// pop the class object
		sq_poptop(vm);
		return res;
	}
	
	ClassInfo *ClassInfo::create(SQInteger sp, HSQUIRRELVM vm)
	{
		HSQOBJECT obj;
		sq_getstackobj(vm, sp, &obj);
		return create(obj, vm);
	}
	
	/////////////////////////////////////////////////////////////////////////////////

	// get metametod for tables and classes
	// it manages getting of class and table registered objects
	static SQInteger _getMetamethod(HSQUIRRELVM vm)
	{
		// get function name
		const char *name;
		sq_getstring(vm, -1, &name);
		
		// get type of requesting object
		SQObjectType type = sq_gettype(vm, -2);

		// get the tableinfo object for this table
		TableInfo const *tableInfo = TableInfo::get(-2, vm);
		
		// locate the function, if it exists
		FunctionInfo const *functionInfo = tableInfo->getFunction(name);
		
		// if found, create and return the closure
		if(functionInfo)
		{
			// remove function name from stack
			// (we leave the original table there)
			sq_poptop(vm);
	
			// now we shall build the resulting closure
			// push the function info as a free variable
			sq_pushuserpointer(vm, (void *)functionInfo);
			
			// create the closure
			sq_newclosure(vm, squirrelFuncHandler, 1);
			
			// result returned
			return 1;
		}
		
		// if object is an instance we're not done!
		if(type == OT_INSTANCE)
		{
			// not found, we shall look for a variable
			ClassInfo const *classInfo = (ClassInfo const *)tableInfo;
			VarInfo const *varInfo = classInfo->getVar(name);
			if(varInfo)
			{
				// call the templated handler
				bool res = varInfo->getFunc(varInfo->varAddr, vm);
				if(res)
					return 1;
			}
			
			// still not found, walk back on parent class, if any
			HSQOBJECT baseClass;
			sq_getclass(vm, -2);
			sq_getstackobj(vm, -1, &baseClass);
			sq_poptop(vm);
				
			while(1)
			{
				sq_pushobject(vm, baseClass);
				sq_getbase(vm, -1);
				
				// if no base class, we're done with error
				if(sq_gettype(vm, -1) == OT_NULL)
				{
					// drop both the NULL and the class
					sq_pop(vm, 2);
					break;
				}
				sq_getstackobj(vm, -1, &baseClass);
				sq_pop(vm, 2);
				
				// get class info for this class
				classInfo = ClassInfo::get(baseClass, vm); 
				
				// locate the function, if it exists
				functionInfo = classInfo->getFunction(name);
				if(functionInfo)
				{
					// remove function name from stack
					// (we leave the original table there)
					sq_poptop(vm);
			
					// now we shall build the resulting closure
					// push the function info as a free variable
					sq_pushuserpointer(vm, (void *)functionInfo);
					
					// create the closure
					sq_newclosure(vm, squirrelFuncHandler, 1);
					
					// result returned
					return 1;
				}
		
				// not found, we shall look for a variable before following the walk back
				VarInfo const *varInfo = classInfo->getVar(name);
				if(varInfo)
				{
					// call the templated handler
					bool res = varInfo->getFunc(varInfo->varAddr, vm);
					if(res)
						return 1;
				}
			
			}
		}
		
		// if comes up to here, the name wasn't found

		// remove the name from stack
		sq_poptop(vm);
		
		// signal the error
		sq_pushnull(vm);
		return sq_throwobject(vm);
		
	}

	// set metametod for classes
	// it manages setting of class members
	// on stack:
	//		-1		value
	//		-2		name
	//		-3		class instance
	SQInteger _setMetamethod(HSQUIRRELVM vm)
	{
		ClassInfo *classInfo = ClassInfo::get(-3, vm);
		
		// get variable name and its descriptor from map
		const char *name;
		sq_getstring(vm, -2, &name);
		
		// get the variable info
		VarInfo const *varInfo = classInfo->getVar(name);
		
		// if found, set it
		if(varInfo)
		{
			// call the templated handler
			bool res = varInfo->setFunc(varInfo->varAddr, vm);
		
			if(res)
			{
				// no result returned
				return 0;
			}
		}
		
		// if not found, walk back on parent classes
		HSQOBJECT baseClass;
		sq_getclass(vm, -3);
		sq_getstackobj(vm, -1, &baseClass);
		sq_poptop(vm);
			
		while(1)
		{
			sq_pushobject(vm, baseClass);
			sq_getbase(vm, -1);
			
			// if no base class, we're done with error
			if(sq_gettype(vm, -1) == OT_NULL)
			{
				// drop NULL, the class and the value
				sq_pop(vm, 3);
				break;
			}
			sq_getstackobj(vm, -1, &baseClass);
			// remove base class and class
			sq_pop(vm, 2);
			
			// get class info for this class
			classInfo = ClassInfo::get(baseClass, vm); 
			
			// locate the variable, if it exists
			varInfo = classInfo->getVar(name);
			if(varInfo)
			{
				// call the templated handler
				bool res = varInfo->setFunc(varInfo->varAddr, vm);
			
				if(res)
				{
					// no result returned
					return 0;
				}
			}
		}
		
		// if comes up to here, the name wasn't found
	
		// remove the name and value from stack
		sq_pop(vm, 2);
		
		// signal the error
		sq_pushnull(vm);
		return sq_throwobject(vm);
	};
	
	// add the _get metamethod to selected table or class
	// the _get metamethod will fetch create the closure on the fly just before calling
	// metamethod MUST be set into table's delegate
	bool addGetMethod(HSQUIRRELVM vm, HSQOBJECT &container)
	{
		sq_pushobject(vm, container);
		// for tables, set the method into its delegate
		if(sq_gettype(vm, -1) == OT_TABLE)
		{
			sq_getdelegate(vm, -1);
			sq_remove(vm, -2);
		}
		sq_pushstring(vm, _SC("_get"), -1, SQTrue);
		if(SQ_SUCCEEDED(sq_rawget(vm, -2)))
		{
			sq_pop(vm, 2);
			return true;
		}
		// not found, create it
		sq_pushstring(vm, _SC("_get"), -1, SQTrue);
		
		// create the closure
		sq_newclosure(vm, _getMetamethod, 0);
		
		// create the slot
		if(!SQ_SUCCEEDED(sq_newslot(vm, -3, SQFalse)))
		{
			sq_pop(vm, 1);
			return false;
		}
		sq_poptop(vm);

		return true;
	}
	
	// add the _set metamethod to selected class
	// the _set metamethod will set a member variable
	bool addSetMethod(HSQUIRRELVM vm, HSQOBJECT &container)
	{
		sq_pushobject(vm, container);
		sq_pushstring(vm, _SC("_set"), -1, SQTrue);
		if(SQ_SUCCEEDED(sq_rawget(vm, -2)))
		{
			// already there, just do nothing
			sq_pop(vm, 2);
			return true;
		}
		// not found, create it
		sq_pushstring(vm, _SC("_set"), -1, SQTrue);
		
		// create the closure
		sq_newclosure(vm, _setMetamethod, 0);
		
		// create the slot
		if(!SQ_SUCCEEDED(sq_newslot(vm, -3, SQFalse)))
		{
			sq_pop(vm, 1);
			return false;
		}
		sq_poptop(vm);

		return true;
	}

}; // end sqfish namespace
