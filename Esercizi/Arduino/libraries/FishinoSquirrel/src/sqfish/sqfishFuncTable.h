#ifndef __SQFISH__FUNC__TABLE__H
#define __SQFISH__FUNC__TABLE__H

#include <assert.h>
//#include <memory.h>

namespace sqfish
{
	// function index are 32 bit unsigned integers
	// lower 16 bits are the class index. Global functions have class index 0
	// next 16 bits are function idx
	typedef uint32_t SQFISH_FUNC_INDEX;
	const SQFISH_FUNC_INDEX SQFISH_INVALID_INDEX = (SQFISH_FUNC_INDEX)-1;
	
	struct FunctionOverload;

	// some weird typedefs...
	typedef void (FunctionOverload::*SQFISH_MEMBER_FUNCTION)(void);
	typedef int FunctionOverload::*SQFISH_MEMBER_VARIABLE;

	typedef void (*SQFISH_GLOBAL_FUNCTION)(void);
	typedef int *SQFISH_GLOBAL_VARIABLE;

	constexpr size_t SQFISH_FUNC_PTR_SIZE =
		sizeof(SQFISH_MEMBER_FUNCTION) > sizeof(SQFISH_GLOBAL_FUNCTION) ? sizeof(SQFISH_MEMBER_FUNCTION) : sizeof(SQFISH_GLOBAL_FUNCTION);
	typedef uint8_t SQFISH_FUNC_PTR_CONTAINER[SQFISH_FUNC_PTR_SIZE];
	
	typedef SQInteger (*SQFISH_FUNC_GLOBAL_HANDLER)(SQFISH_GLOBAL_FUNCTION, HSQUIRRELVM);
	typedef SQInteger (*SQFISH_FUNC_MEMBER_HANDLER)(SQFISH_MEMBER_FUNCTION, HSQUIRRELVM);
	typedef void *(*SQFISH_FUNC_CONSTRUCTOR_HANDLER)(HSQUIRRELVM);
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// add the _get metamethod to selected table or class
	// the _get metamethod will fetch create the closure on the fly just before calling
	// metamethod MUST be set into table's delegate
	bool addGetMethod(HSQUIRRELVM vm, HSQOBJECT &container);

	// add the _set metamethod to selected class
	// the _set metamethod will set a member variable
	bool addSetMethod(HSQUIRRELVM vm, HSQOBJECT &container);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this struct contains info for an overload of a binded function
	// (both member and non-member), which is composed by the global
	// handler, the function pointer and the parameters signature
	class OverloadInfo
	{
		protected:
			SignatureID const *_signature;
			bool _isMember;
			
			// the handler
			union
			{
				SQFISH_FUNC_GLOBAL_HANDLER _globalHandler;
				SQFISH_FUNC_MEMBER_HANDLER _memberHandler;
				SQFISH_FUNC_CONSTRUCTOR_HANDLER _constructorHandler;
			};
			
			// the true function pointer
			// we use a buffer able to store all kind of function ptrs
			// (global fn ptrs have size <= of member ones)
			union
			{
				SQFISH_FUNC_PTR_CONTAINER _ptrContainer;
				SQFISH_MEMBER_FUNCTION _memberFunc;
				SQFISH_GLOBAL_FUNCTION _globalFunc;
			};
			
		public:
		
			OverloadInfo(SignatureID const *signature) 
			{
				_signature = signature;
				_isMember = false;
				_memberHandler = 0;
				memset(_ptrContainer, 0, SQFISH_FUNC_PTR_SIZE);
			}
			
			OverloadInfo(void) 
			{
				// this one should NEVER be called
				// but fishino STL porting has some caveats...
				_signature = NULL;
				_isMember = false;
				_memberHandler = 0;
				memset(_ptrContainer, 0, SQFISH_FUNC_PTR_SIZE);
			}
			
			inline bool isMember(void) const { return _isMember; }
			inline SignatureID const *getSignature(void) const { return _signature; }
			
			// sets the constructor handler
			void setConstructorHandler(SQFISH_FUNC_CONSTRUCTOR_HANDLER handler)
			{
				_constructorHandler = handler;
			}
			
			// set the global handlers
			void setGlobalHandlers(SQFISH_FUNC_GLOBAL_HANDLER handler, SQFISH_GLOBAL_FUNCTION func)
			{
				_globalHandler = handler;
				_globalFunc = func;
				_isMember = false;
			}
			
			// set the member handlers
			void setMemberHandlers(SQFISH_FUNC_MEMBER_HANDLER handler, SQFISH_MEMBER_FUNCTION func)
			{
				_memberHandler = handler;
				_memberFunc = func;
				_isMember = true;
			}
			
			// calls the corresponding function
			SQInteger call(HSQUIRRELVM vm) const
			{
				if(_isMember)
					return _memberHandler(_memberFunc, vm);
				else
					return _globalHandler(_globalFunc, vm);
			}
			
			// calls the corresponding constructor
			void *callConstructor(HSQUIRRELVM vm) const
			{
				return _constructorHandler(vm);
			}
	};
	
	class OverloadsInfo
	{
		protected:
			
			smallvec<OverloadInfo> _overloads;
			
		public:
			
			// find an overload by signature
			OverloadInfo const *get(SignatureType const &signature, bool strict) const ;
			OverloadInfo const *get(SignatureID const *signature) const ;
	
			// create an overload by signature -- return NULL if already there
			OverloadInfo *create(SignatureID const *signature) ;
			
			// get the number of available overloads
			size_t getCount(void) const { return _overloads.getCount(); }

			OverloadsInfo() { _overloads.clear(); }
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this structure contains informations about a binden function
	// so, the squirrel name and an array of overloaded versions
	class FunctionInfo
	{
		private:
			// the function name
			LITSTR _name;
			
			// its overloaded versions
			OverloadsInfo _overloads;

		public:
			// constructor
			FunctionInfo(LITSTR name) 
			{
				_name = name;
			}
			
			// dummy -- needed for std::vector
			FunctionInfo(void)
			{
				assert(false);
			}
			
			inline LITSTR getName(void) const { return _name; }
	
			// find an overload by signature
			OverloadInfo const *getOverloadInfo(SignatureType const &signature, bool strict) const ;
	
			// create an overload by signature -- return NULL if already there
			OverloadInfo *createOverloadInfo(SignatureID const *signature) ;
	};
	
	class FunctionsInfo
	{
		protected:
	
			smallvec<FunctionInfo> _functions;
			
		public:
			
			// create function and overload info for a function
			bool create(LITSTR name, SignatureID const *signature, FunctionInfo *&functionInfo, OverloadInfo *&overloadInfo);
			
			// get a function by name
			FunctionInfo const *get(const char *name) const;
			
			// get number of available functions
			size_t getCount(void) const { return _functions.getCount(); }
			
			FunctionsInfo() { _functions.clear(); }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this structure contains setter and getter for variables
	union VarAddr
	{
		SQFISH_MEMBER_VARIABLE member;
		SQFISH_GLOBAL_VARIABLE global;
	};
	
	struct VarInfo
	{
		LITSTR varName;
		VarAddr varAddr;
		bool (*setFunc)(VarAddr const &addr, HSQUIRRELVM vm);
		bool (*getFunc)(VarAddr const &addr, HSQUIRRELVM vm);
		
		VarInfo(LITSTR name) { varName = name; }
	};
	
	class VarsInfo
	{
		private:
			smallvec<VarInfo> _infos;
		
		public:

			// add a varinfo to list
			VarInfo *add(LITSTR name);
			
			// get a variable from list
			VarInfo const *get(const char *name) const;
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this class is connected to each squirrel table that contains binded functions
	// it maps the functions with their handles and type signatures
	class TableInfo
	{
		protected:
			
			// the functions
			FunctionsInfo _functions;
			
		public:
			// create function and overload info for a function
			bool createFunctionInfo(LITSTR name, SignatureID const *signature, FunctionInfo *&functionInfo, OverloadInfo *&overloadInfo)  ;
			
			// get a function by name
			FunctionInfo const *getFunction(const char *name) const { return _functions.get(name); }
			
			// retrieve (or create) a TableInfo data inside a table
			static TableInfo *get(HSQOBJECT &table, HSQUIRRELVM vm) ;
			static TableInfo *get(SQInteger sp, HSQUIRRELVM vm) ;
			
			// we need a virtual destructor here
			virtual ~TableInfo()   {}
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// this class is connected to each squirrel class that contains binded functions
	// and binded members; it maps the functions with their handles and type signatures
	// and member getters/setters
	class ClassInfo : public TableInfo
	{
		protected:
			
			// the constructors 
			OverloadsInfo _constructors;
			
			// variables
			VarsInfo _varFunctions;
			
			// no-create flag
			bool _noCreate;
			
		public:
		
			// create a constructor overload from signature
			OverloadInfo *createConstructorInfo(SignatureID const *signature) ;
			
			// get a constructor overload from signature
			OverloadInfo const *getConstructorInfo(SignatureType const &signature) const ;
			
			// get number of constructors
			size_t numConstructors(void) const { return _constructors.getCount(); }
			
			// retrieve a ClassInfo data inside a class
			static ClassInfo *get(HSQOBJECT &cls, HSQUIRRELVM vm);
			static ClassInfo *get(SQInteger sp, HSQUIRRELVM vm) ;
			
			// retrieve a ClassInfo data inside a class
			static ClassInfo *create(HSQOBJECT &cls, HSQUIRRELVM vm) ;
			static ClassInfo *create(SQInteger sp, HSQUIRRELVM vm);
			
			// constructor
			ClassInfo()   { _noCreate = false; }
			
			// we need a virtual destructor here
			virtual ~ClassInfo()  {}
			
			// gets the no create flag
			bool cantCreate(void) const { return _noCreate; }
			
			// sets the no create flag
			void setNoCreate(bool b = true) { _noCreate = b; }
			
			// try to fetch a variable descriptor
			VarInfo const *getVar(const char *name) const { return _varFunctions.get(name); }
			VarInfo *addVar(LITSTR name) { return _varFunctions.add(name); }
	};
}; // end sqfish namespace

#endif
