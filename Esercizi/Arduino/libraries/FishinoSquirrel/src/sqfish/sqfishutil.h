#ifndef __SQFISH__UTIL__H
#define __SQFISH__UTIL__H

/*
#include <typeinfo>
#include <cxxabi.h>
#include <string>
*/

namespace sqfish
{
	// these are used to force constant, literal strings in sqfish binding names
	// which allows to store/copy just the pointer without having to manage
	// allocations and memory copies
	struct _literalstring
	{
		bool equal(_literalstring const *other) { return !strcmp((const char *)this, (const char *)other); }
		bool equal(const char *other) { return !strcmp((const char *)this, other); }
		const char *str(void) { return (const char *)this; }
		bool empty(void) { return *(const char *)this == 0; }
	};
	
	typedef _literalstring *LITSTR;
	
	constexpr LITSTR operator "" _LIT(const char *s, size_t) {
		return (LITSTR)s;
	}

	class SQFishTable;

	// type ids for custom classes
	typedef uint16_t SignatureID;
	typedef std::vector<SignatureID> SignatureType;
		
	// instance descriptor (stored on instance __theinstance__ user data
	// contains a pointer to the C++ instance and an 'owned' flag
	struct InstanceInfo
	{
		void *_theInstance;
		bool _owned;
		SignatureID _type;
	};
	
	namespace utils
	{
		// some tools to allow conditional compilation for templates
		
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// this one selects between void and non-void returning template functions
		template <typename T, typename Q> struct enable_void {};
		
		template<typename Q> struct enable_void<void, Q> {
			using type = Q;
		};
		
		template <typename T, typename Q> struct disable_void{
			using type = Q;
		};
		
		template<typename Q> struct disable_void<void, Q> {};
	

		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// the usual C++ way to complicate simple stuffs...
		// default construct a class if it has a default constructor
		// otherwise return a NULL pointer
		template<class T> class is_default_constructible {
				template<int x> class receive_size{};
				
				template<class U> static int sfinae(receive_size<sizeof U() > *);
				template<class U> static char sfinae( ... );
			
			public:
				enum { value = sizeof( sfinae<T>(0) ) == sizeof(int) };
		};
		
		template<class C, bool b> struct _defaultConstructor {
			C * New() { return new C; }
		};
		template<class C> struct _defaultConstructor<C, false> {
			C * New() { return NULL; }
		};
		
		template<class C> C *defaultConstruct(void) {
			return _defaultConstructor<C, is_default_constructible<C>::value>().New();
		}
	
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// the usual C++ way to complicate simple stuffs, part 2...
		// copy construct a class if it has a copy constructor
		// otherwise return a NULL pointer
		template<class T> class is_copiable {
				template<int x> class receive_size{};
				
				template<class U, U const *u> static int sfinae(receive_size<sizeof U(*u) > *);
				template<class U, U const *u> static char sfinae( ... );
			
			public:
				enum { value = sizeof( sfinae<T, (const T *)NULL>(0) ) == sizeof(int) };
		};
		
		template<class C, bool b> struct _copyConstructor {
			C * Copy(C const &c) { return new C(c); }
		};
		template<class C> struct _copyConstructor<C, false> {
			C * Copy(C const &c) { return NULL; }
		};
		
		template<class C> C *copyConstruct(C const &c) {
			return _copyConstructor<C, is_copiable<C>::value>().Copy(c);
		}
	
		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// those are used to guarantee left to right function arguments evaluation
		template<class R> struct ArgEvalOrderer
		{
			R ret;
		
			template<class... ARGS>  ArgEvalOrderer(R(*f)(ARGS...), ARGS... args) : ret{f(args...)}
			{}
		
			operator R() const
			{
				return ret;
			}
		};
		
		template<> struct ArgEvalOrderer<void>
		{
			typedef int(*INTFUNC)(...);
			int ret;
			template<class... ARGS>  ArgEvalOrderer(void(*f)(ARGS...), ARGS... args) : ret{((INTFUNC)f)(args...)}
			{}
		};

		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// those are used to remove some stuffs on template types
		template<class T> struct remove_all { typedef T type; };
		template<class T> struct remove_all<T*> : remove_all<T> {};
		template<class T> struct remove_all<T&> : remove_all<T> {};
		template<class T> struct remove_all<T&&> : remove_all<T> {};
		template<class T> struct remove_all<T const> : remove_all<T> {};
		template<class T> struct remove_all<T volatile> : remove_all<T> {};
		template<class T> struct remove_all<T const volatile> : remove_all<T> {};

		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// one of the best stuffs found on stackoverflow
		// map types with unique integers
		// here restricted to 65536 different types
		inline SignatureID _nextTypeId()
		{
			// start above pre-defined types
			static SignatureID id = 100;

			SignatureID result = id;
			++id;
			return result;
		}
		
		/** Returns a small value which identifies the type.
		Multiple calls with the same type return the same value. */
		template <typename T> SignatureID _getTypeId(SignatureID forcedID)
		{
			static SignatureID id = (forcedID == -1 ? _nextTypeId() : forcedID);
			return id;
		}
		
		// avoid having different types for pointers, consts &c
		template <typename T> SignatureID getTypeId(SignatureID forcedID = -1)
		{
			return _getTypeId<typename remove_all<T>::type>(forcedID);
		}

		struct DUMMY_TYPE {};
		struct INSTANCE_TYPE {};

		// specializations for known types
		template<> inline SignatureID getTypeId<DUMMY_TYPE>(SignatureID)				{ return  0; }
		template<> inline SignatureID getTypeId<INSTANCE_TYPE>(SignatureID)				{ return  1; }
		template<> inline SignatureID getTypeId<signed char>(SignatureID)				{ return 51; }
		template<> inline SignatureID getTypeId<int>(SignatureID)						{ return 52; }
		template<> inline SignatureID getTypeId<long int>(SignatureID)					{ return 53; }
		template<> inline SignatureID getTypeId<long long int>(SignatureID)				{ return 54; }
		template<> inline SignatureID getTypeId<unsigned char>(SignatureID)				{ return 55; }
		template<> inline SignatureID getTypeId<unsigned int>(SignatureID)				{ return 56; }
		template<> inline SignatureID getTypeId<unsigned long int>(SignatureID)			{ return 57; }
		template<> inline SignatureID getTypeId<unsigned long long int>(SignatureID)	{ return 58; }
		template<> inline SignatureID getTypeId<float>(SignatureID)						{ return 59; }
		template<> inline SignatureID getTypeId<double>(SignatureID)					{ return 60; }
		template<> inline SignatureID getTypeId<long double>(SignatureID)				{ return 61; }
		template<> inline SignatureID getTypeId<bool>(SignatureID)						{ return 62; }
		template<> inline SignatureID getTypeId<char *>(SignatureID)					{ return 63; }
		template<> inline SignatureID getTypeId<const char *>(SignatureID)				{ return 64; }
		template<> inline SignatureID getTypeId<std::string>(SignatureID)				{ return 65; }
		template<> inline SignatureID getTypeId<std::string &>(SignatureID)				{ return 66; }
		template<> inline SignatureID getTypeId<std::string const &>(SignatureID)		{ return 67; }

		///////////////////////////////////////////////////////////////////////////////////////////////////////////

		// return a string with object type name
		const char *ObjTypeName(SQObjectType typ);
		
		// dump stack types
		std::string dumpStackTypes(HSQUIRRELVM vm);
		
		// get squirrel signature for type
		template<typename P> SignatureID  GetTypeSignature()
		{
			return getTypeId<P>();
		}
		
		template<typename... PARMS> SignatureID const *GetSignatureString()
		{
			static SignatureID const s[] = { (SignatureID)sizeof...(PARMS), GetTypeSignature<PARMS>()...,};
			return s;
		}
		
		// loose compare of signature strings
		// it returns the number of substituted types (i, b, f to n)
		// -1 if argument count is different
		// -2 if types are not compatibles
		// 0 for perfect match
		int CompareTypeSignatures(SignatureType const &given, SignatureID const *target) ;
		
		// get type signature from a squirrel type
		SignatureID GetTypeSignatureFromId(SQObjectType tag) ;
	
		// get type signature string from params on squirrel stack
		SignatureType GetParamsSignatureFromStack(HSQUIRRELVM vm) ;
		
		// get an unique ID for a table -- used to register functions
		// separately on each table (in case we've got same func registered in different tables)
		std::string GetTableId(HSQOBJECT &obj, HSQUIRRELVM vm) ;
	
		// gets the instance type (instance object at requested position in stack)
		SignatureID getInstanceType(SQInteger idx, HSQUIRRELVM vm) ;
		
		// gets the instance variable (instance object at -top, before params)
		void *getInstance(HSQUIRRELVM vm) ;
		
		// gets the instance variable for a given instance object
		void *getInstance(HSQOBJECT &obj, HSQUIRRELVM vm) ;

		// release hook for instance member
		template<class C> SQInteger _instanceReleaseHook(SQUserPointer ptr, SQInteger size)
		{
			InstanceInfo *instanceInfo = (InstanceInfo *)ptr;
//			DEBUG_PRINT("Destroying instance %p, owns is %s\n", (void *)instanceInfo->_theInstance, instanceInfo->_owned ? "true" : "false");
			if(instanceInfo->_owned && instanceInfo->_theInstance)
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
				delete (C *)instanceInfo->_theInstance;
			#pragma GCC diagnostic pop
			instanceInfo->_theInstance = NULL;
			instanceInfo->_owned = false;
			return 0;
		}
		
		// sets the instance variable for object cls
		// (may be an instance or the class object)
		template<class C> void setInstance(HSQOBJECT &cls, C *instance, bool owned, HSQUIRRELVM vm)
		{
//			DEBUG_PRINT("Setting instance %p, owns is %s\n", (void *)instance, owned ? "true" : "false");
			sq_pushobject(vm, cls);
			
			// get user pointer for instance
			InstanceInfo *instanceInfo = NULL;
			bool res = SQ_SUCCEEDED(sq_getinstanceup(vm, -1, (void **)&instanceInfo, NULL));
			if(!res || !instanceInfo)
			{
				sq_poptop(vm);
				return;
			}
			instanceInfo->_theInstance = instance;
			instanceInfo->_owned = owned;
			instanceInfo->_type = getTypeId<C>();
			
			// setup cleanup handler
			// don't know if it's needed here, but doesn't harm
			sq_setreleasehook(vm, -1, _instanceReleaseHook<C>);

			sq_poptop(vm);
		}

		// resolve a table/class name from string in form of
		// [ROOTNAME/][/]atable.btable.ctable.aclass
		// ROOTNAME can be ROOT_TABLE, CONST_TABLE or REGISTRY_TABLE
		// if not present, it / is present the path starts from roottable
		// if / is NOT present, the path starts from current table
		bool resolveName(HSQUIRRELVM vm, HSQOBJECT &thisTable, const char *path, HSQOBJECT &destObj) ;
		bool resolveName(HSQUIRRELVM vm, SQFishTable *thisTable, const char *path, HSQOBJECT &destObj) ;
		
		// register a class on registry table
		void registerClass(HSQUIRRELVM vm, SQFishTable *parent, const char *className, std::type_info const &id);
		
		// recover registered class name from id
		const char *getRegisteredClass(HSQUIRRELVM vm, std::type_info const &id);


		// push a c++ class instance on stack
		template<class C> bool PushInstance(HSQUIRRELVM vm, const char *classPath, C &cppInstance, bool owns) {
			
			// we need the root table to solve the class
			HSQOBJECT root;
			sq_pushroottable(vm);
			sq_getstackobj(vm, -1, &root);
			sq_poptop(vm);

			// get the class object
			HSQOBJECT cls;
			if(!utils::resolveName(vm, root, classPath, cls))
			{
				RLOG("ERROR RESOLVING CLASS PATH '" << classPath << "'");
				return false;
			}
			
			// create the instance
			sq_pushobject(vm, cls);
			if(!SQ_SUCCEEDED(sq_createinstance(vm, -1)))
			{
				RLOG("ERROR CREATING THE INSTANCE");
				sq_poptop(vm);
				return false;
			}

			// get the instance back
			HSQOBJECT inst;
			sq_getstackobj(vm, -1, &inst);
			sq_addref(vm, &inst);
			
			// pop back both the class and the instance object
			sq_pop(vm, 2);
			
			// setup instance class pointer
			utils::setInstance(inst, &cppInstance, owns, vm);

			// re-push the instance
			sq_pushobject(vm, inst);
			sq_release(vm, &inst);
			
			return true;
		}

	}; // end namespace util
	
}; // end namespace sqfish


#endif
