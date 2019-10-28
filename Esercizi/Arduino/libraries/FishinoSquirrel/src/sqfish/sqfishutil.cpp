#include "sqfish.h"

#include <typeinfo>

namespace sqfish
{
	SQInteger testrel(SQUserPointer p,SQInteger size)
	{
		RLOG("Class release hook called");
		return 1;
	}
	
	namespace utils
	{

		// return a string with object type name
		const char *ObjTypeName(SQObjectType typ)
		{
			switch(typ)
			{
				case OT_NULL:
					return "OT_NULL";
				case OT_INTEGER:
					return "OT_INTEGER";
				case OT_FLOAT:
					return "OT_FLOAT";
				case OT_STRING:
					return "OT_STRING";
				case OT_TABLE:
					return "OT_TABLE";
				case OT_ARRAY:
					return "OT_ARRAY";
				case OT_USERDATA:
					return "OT_USERDATA";
				case OT_CLOSURE:
					return "OT_CLOSURE";
				case OT_NATIVECLOSURE:
					return "OT_NATIVECLOSURE";
				case OT_GENERATOR:
					return "OT_GENERATOR";
				case OT_USERPOINTER:
					return "OT_USERPOINTER";
				case OT_CLASS:
					return "OT_CLASS";
				case OT_INSTANCE:
					return "OT_INSTANCE";
				case OT_WEAKREF:
					return "OT_WEAKREF";
				default:
					return "UNKNOWN";
			}
		}
	
		// dump stack types
		std::string dumpStackTypes(HSQUIRRELVM vm)
		{
			std::string res;
			for(int i = 0; i < sq_gettop(vm); i++)
				res += std::string(ObjTypeName(sq_gettype(vm, -1 - i))) + "\n";
			return res;
		}
	
		// get type char signature from a squirrel type
		SignatureID GetTypeSignatureFromId(SQObjectType tag)
		{
			switch(tag)
			{
				case OT_INTEGER:
					return getTypeId<SQInteger>();
					
				case OT_FLOAT:
					return getTypeId<SQFloat>();
					
				case OT_BOOL:
					return getTypeId<SQBool>();
	
				case OT_STRING:
					return getTypeId<SQChar *>();
					
				case OT_INSTANCE:
					return getTypeId<INSTANCE_TYPE>();

				case OT_NULL:
				case OT_TABLE:
				case OT_ARRAY:
				case OT_USERDATA:
				case OT_CLOSURE:
				case OT_NATIVECLOSURE:
				case OT_GENERATOR:
				case OT_USERPOINTER:
				case OT_CLASS:
				case OT_WEAKREF:
				default:
					return getTypeId<DUMMY_TYPE>();
			}
		}
	
		// get type signature string from params on squirrel stack
		SignatureType GetParamsSignatureFromStack(HSQUIRRELVM vm)
		{
			SQInteger nParams = sq_gettop(vm);
			SignatureType signature;
			signature.reserve(nParams);
			for(int i = 0; i < nParams - 1; i++)
			{
				SignatureID id = GetTypeSignatureFromId(sq_gettype(vm, -nParams + i + 1));
				
				// if it's an object instance, get its type
				if(id == getTypeId<INSTANCE_TYPE>())
					signature.push_back(getInstanceType(-nParams + i + 1, vm));
				// otherwise if it's a standard squirrel type, use it
				// (use it even if it's a DUMMY_TYPE)
				else
					signature.push_back(id);
			}
			return signature;
		}
	
		// some ready-to-use type ids
		const SignatureID charID		= getTypeId<char>();
		const SignatureID shortID		= getTypeId<short>();
		const SignatureID intID			= getTypeId<int>();
		const SignatureID longID		= getTypeId<long>();
		const SignatureID longlongID	= getTypeId<long long>();

		const SignatureID ucharID		= getTypeId<unsigned char>();
		const SignatureID ushortID		= getTypeId<unsigned short>();
		const SignatureID uintID		= getTypeId<unsigned int>();
		const SignatureID ulongID		= getTypeId<unsigned long>();
		const SignatureID ulonglongID	= getTypeId<unsigned long long>();

		const SignatureID boolID		= getTypeId<bool>();
			
		const SignatureID floatID		= getTypeId<float>();
		const SignatureID doubleID		= getTypeId<double>();
		const SignatureID longdoubleID	= getTypeId<long double>();
		
		const SignatureID pcharID		= getTypeId<char *>();
		const SignatureID cpcharID		= getTypeId<const char *>();
		const SignatureID stringID		= getTypeId<std::string>();
		const SignatureID rstringID		= getTypeId<std::string &>();
		const SignatureID crstringID	= getTypeId<std::string const &>();
			
		// loose compare of signature strings
		// it returns the number of substituted types (i, b, f to n)
		// -1 if argument count is different
		// -2 if types are not compatibles
		// 0 for perfect match
		int CompareTypeSignatures(SignatureType const &given, SignatureID const *target)
		{
			if(given.size() != target[0])
				return -1;
			size_t len = given.size();
			target++;
			int diffs = 0;
			for(size_t i = 0; i < len; i++)
			{
				SignatureID cg = given[i];
				SignatureID ct = target[i];
				if(cg == ct)
					continue;
				// unknown type parameters are accepted
				if(ct == getTypeId<DUMMY_TYPE>())
					continue;
				
				bool intTarget = (
					ct == charID  || ct == shortID  || ct == intID  || ct == longID  || ct == longlongID ||
					ct == ucharID || ct == ushortID || ct == uintID || ct == ulongID || ct == ulonglongID
				);
				bool boolTarget = (ct == boolID);
				bool floatTarget = (ct == floatID || ct == doubleID || ct == longdoubleID);
				bool strTarget = (ct == pcharID || ct == cpcharID || ct == stringID || ct == rstringID || ct == crstringID);

				bool intSource = (
					cg == charID  || cg == shortID  || cg == intID  || cg == longID  || cg == longlongID ||
					cg == ucharID || cg == ushortID || cg == uintID || cg == ulongID || cg == ulonglongID
				);
				bool boolSource = (cg == boolID);
				bool floatSource = (cg == floatID || cg == doubleID || cg == longdoubleID);
				bool strSource = (cg == pcharID || cg == cpcharID || cg == stringID || cg == rstringID || cg == crstringID);
				
				// if converting from int to int type, count a difference
				if(intSource && intTarget)
				{
					diffs++;
					continue;
				}
				// if converting from int to bool type, do not take as different
				if(intSource && boolTarget)
					continue;
				// if converting from bool to int type, count a difference
				if(boolSource && intTarget)
				{
					diffs++;
					continue;
				}
				// if converting from float to float, do not count as different
				if(floatSource && floatTarget)
					continue;
				// if converting from int to float, don't count as different
				if(intSource && floatTarget)
					continue;
				// if converting grom float to int, count as different
				if(floatSource && intTarget)
				{
					diffs++;
					continue;
				}
				// strings are counted as same type
				if(strSource && strTarget)
					continue;
				
				// no known conversion, just return incompatible types
				return -2;
			}
			return diffs;
		}
	
		// get an unique ID for a table -- used to register functions
		// separately on each table (in case we've got same func registered in different tables)
		std::string GetTableId(HSQOBJECT &obj, HSQUIRRELVM vm)
		{
			sq_pushobject(vm, obj);
			sq_tostring(vm, -1);
			const char *str;
			sq_getstring(vm, -1, &str);
			std::string res = str;
			sq_pop(vm, 2);
			return res;
		}
	
		// gets the instance variable (instance object at requested position in stack)
		SignatureID getInstanceType(SQInteger idx, HSQUIRRELVM vm)
		{
			if(idx < 0)
				idx--;
			
			// get user pointer for instance
			InstanceInfo *instanceInfo = NULL;
			if(!SQ_SUCCEEDED(sq_getinstanceup(vm, idx, (void **)&instanceInfo, NULL)))
				return getTypeId<DUMMY_TYPE>();
			if(!instanceInfo)
				return getTypeId<DUMMY_TYPE>();
			return instanceInfo->_type;
		}
		
		// gets the instance variable (instance object at -top, before params)
		void *getInstance(HSQUIRRELVM vm)
		{
			SQInteger top = sq_gettop(vm);
			
			// get user pointer for instance
			InstanceInfo *instanceInfo = NULL;
			if(!SQ_SUCCEEDED(sq_getinstanceup(vm, -top, (void **)&instanceInfo, NULL)))
				return NULL;
			if(!instanceInfo)
				return NULL;
			return instanceInfo->_theInstance;
		}

		// gets the instance variable for a given instance object
		void *getInstance(HSQOBJECT &obj, HSQUIRRELVM vm)
		{
			sq_pushobject(vm, obj);
			
			// get user pointer for instance
			InstanceInfo *instanceInfo = NULL;
			bool res = SQ_SUCCEEDED(sq_getinstanceup(vm, -1, (void **)&instanceInfo, NULL));
			sq_poptop(vm);
			if(!res || !instanceInfo)
				return NULL;
			return instanceInfo->_theInstance;
		}

		// resolve a table/class name from string in form of
		// [ROOTNAME/][/]atable.btable.ctable.aclass
		// ROOTNAME can be ROOT_TABLE, CONST_TABLE or REGISTRY_TABLE
		// if not present, it / is present the path starts from roottable
		// if / is NOT present, the path starts from current table
		bool resolveName(HSQUIRRELVM vm, HSQOBJECT &thisTable, const char *path, HSQOBJECT &destObj)
		{
			// get the start object, if path is absolute
			HSQOBJECT startTable = thisTable;
			if(!strncmp(path, "ROOT_TABLE/", 11))
			{
				sq_pushroottable(vm);
				sq_getstackobj(vm, -1, &startTable);
				sq_poptop(vm);
				path += 11;
			}
			else if(!strncmp(path, "CONST_TABLE/", 12))
			{
				sq_pushconsttable(vm);
				sq_getstackobj(vm, -1, &startTable);
				sq_poptop(vm);
				path += 12;
			}
			else if(!strncmp(path, "REGISTRY_TABLE/", 15))
			{
				sq_pushregistrytable(vm);
				sq_getstackobj(vm, -1, &startTable);
				sq_poptop(vm);
				path += 15;
			}
			else if(!strncmp(path, "/", 1))
			{
				sq_pushroottable(vm);
				sq_getstackobj(vm, -1, &startTable);
				sq_poptop(vm);
				path += 1;
			}
			else
			{
			}

			// now walk on up it finds the final object
			// or an error
			destObj = startTable;
			sq_pushobject(vm, destObj);
			while(*path)
			{
				int pLen;
				const char *pathEnd = strchr(path, '.');
				if(pathEnd)
					pLen = pathEnd - path;
				else
					pLen = strlen(path);
				sq_pushstring(vm, path, pLen, SQFalse);
				if(!SQ_SUCCEEDED(sq_get(vm, -2)))
				{
					RLOG("Failed to get it");
					sq_poptop(vm);
					return false;
				}
				sq_getstackobj(vm, -1, &destObj);
				sq_poptop(vm);
				path += pLen;
				if(*path)
					path++;
			}
			sq_poptop(vm);
			return true;
		}

		bool resolveName(HSQUIRRELVM vm, SQFishTable *thisTable, const char *path, HSQOBJECT &destObj)
		{
			return resolveName(vm, thisTable->_obj, path, destObj);
		}

		// register a class on registry table
		// classPath MUST be a full path from RootTable
		void registerClass(HSQUIRRELVM vm, SQFishTable *parent, const char *classPath, std::type_info const &id)
		{
			// build full class path
			std::string fullPath = parent->getFullPath();
			if(fullPath != "")
				fullPath + "." + classPath;
			else
				fullPath = classPath;
			// get/create 'classes' table from registry
			sq_pushregistrytable(vm);

			sq_pushstring(vm, _SC("classes"), -1, SQTrue);
			if(!SQ_SUCCEEDED(sq_get(vm, -2)))
			{
				sq_pushstring(vm, _SC("classes"), -1, SQTrue);
				sq_newtable(vm);
				sq_newslot(vm, -3, SQFalse);
				sq_pushstring(vm, _SC("classes"), -1, SQTrue);
				sq_get(vm, -2);
			}
			sq_pushstring(vm, _SC(id.name()), -1, SQFalse);
			sq_pushstring(vm, fullPath.c_str(), -1, SQFalse);
			sq_newslot(vm, -3, SQFalse);
			sq_poptop(vm);
		}
		
		// recover registered class name from id
		// return a FULL class path from roottable
		const char *getRegisteredClass(HSQUIRRELVM vm, std::type_info const &id)
		{
			sq_pushregistrytable(vm);
			sq_pushstring(vm, _SC("classes"), -1, SQTrue);
			if(!SQ_SUCCEEDED(sq_get(vm, -2)))
			{
				sq_poptop(vm);
				return "";
			}
			sq_pushstring(vm, _SC(id.name()), -1, SQFalse);
			if(!SQ_SUCCEEDED(sq_get(vm, -2)))
			{
				sq_pop(vm, 2);
				return "";
			}
			const char *res;
			sq_getstring(vm, -1, &res);
			sq_pop(vm, 3);
			return res;
		}

	}; // end namespace utils

}; // end namespace sqfish
