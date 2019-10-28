#ifndef __SQFISH__CLASS__H
#define __SQFISH__CLASS__H

// SQFishClass uses 2 special members on Squirrel classes to store
// various data pointers of underlying C++ class
// those fields are :
//		__theinstance__		which contains a pointer to the C++ class instance
//							it's an instance member, set when class is created
//		__dispatcher__		which contains the address of functions dispatcher
// all other data is stored in C++ structures outside Squirrel, we don't use registry
//
// The dispatcher, which is a static class member:
//		it's a function handle with variable number of parameters; the calls are:
//		dispatch(INSTANCE_CREATE)	which create the class and stores the handler in __theinstance__
//		dispatch(INSTANCE_DESTROY)	which deletes the instance
//		dispatch(FUNCTION_CALL)		which calls a function, see later
//		dispatch(MEMBER_SET)		which sets a variable member
//		dispatch(MEMBER_GET)		which gets a variable member
// all data is exchanged through the stack and is variable in size
// the dispatcher will take care of function pointers, overloaded functions, etc

namespace sqfish
{
	///////////////////////////////////////////////////////////////////////////////////////////

	typedef void (*SQFISH_CLASS_CREATE_FUNC)(int, ...);

	class SQFishTable;
	
	template<class C> class SQFishClass;
	template<class C> SQFishClass<C> Class(LITSTR name, HSQUIRRELVM v) ;
	template<class C> SQFishClass<C> Class(LITSTR base, LITSTR name, HSQUIRRELVM v) ;
	
	// forward declaration
	void pushTable(HSQUIRRELVM, SQFishTable *);
	
	// handler for constructors -- returns a newly constructed class
	template<typename C, typename... PARMS> C *constructorHandler(HSQUIRRELVM vm) {

		// create the object
		C * newC = StackBindNew<C, PARMS...>(vm);
		
		return newC;
	}

	template<class C> class SQFishClass
	{
		friend class SQFishTable;
		friend SQFishClass Class<C>(LITSTR name, HSQUIRRELVM v);
		friend SQFishClass Class<C>(LITSTR base, LITSTR name, HSQUIRRELVM v);
		
		private:
			
			// the associated vm
			HSQUIRRELVM _vm;
			
			// object handle
			HSQOBJECT _obj;
			
			// our name
			LITSTR _name;
			
			// parent table, if any
			SQFishTable *_parent;
			
			// flag stating that we own our parent
			// used ONLY to manage a parent on global tables/classes
			bool _ownsParent;
			
			// class can be constructed only by table objects
			// (besides the exception of global Class() function, which sets
			// a parent root table on the fly
			SQFishClass(SQFishTable *parent, bool ownsParent, LITSTR name, HSQUIRRELVM v) 
				{ init(parent, ownsParent, NULL, name, v); }
				
			SQFishClass(SQFishTable *parent, bool ownsParent, LITSTR base, LITSTR name, HSQUIRRELVM v) 
				{ init(parent, ownsParent, base, name, v); }
				
			// copy constructor - private (class shall NOT be copied) but
			// used by SQFishTable class on non-optimized compilers
			SQFishClass(SQFishClass const &src)  {
				
				_vm = src._vm;
				_obj = src._obj;
				_name = src._name;
				if(src._ownsParent)
				{
					_parent = new SQFishTable(*src._parent);
					_ownsParent = true;
				}
				else
				{
					_parent = src._parent;
					_ownsParent = false;
				}
			}
				
			// default constructor -- construct a class with no parameters
			// if class is default constructible, otherwise returns NULL
			static C *defaultConstructor(HSQUIRRELVM vm) 
			{
				C *instance = utils::defaultConstruct<C>();
				return instance;
			}
			
			// the squirrel constructor function
			static SQInteger _constructor(HSQUIRRELVM vm) 
			{
				int top = sq_gettop(vm);

				// get signature string from stack parameters
				SignatureType signature = utils::GetParamsSignatureFromStack(vm);
				
				// get the class object
				HSQOBJECT cls;
				if(!SQ_SUCCEEDED(sq_getstackobj(vm, -top, &cls)))
					return sq_throwerror(vm, _SC("error getting class object"));
				
				// get class info data
				ClassInfo const *classInfo = ClassInfo::get(cls, vm);
				if(!classInfo)
					return sq_throwerror(vm, _SC("error getting class info object"));

				if(classInfo->cantCreate())
					return sq_throwerror(vm, _SC("instances creations are disabled for this class"));
				
				C * instance = NULL;
				if(classInfo->numConstructors() > 0)
				{
					// try to get the right constructor
					OverloadInfo const *overloadInfo = classInfo->getConstructorInfo(signature);
					if(!overloadInfo)
						return sq_throwerror(vm, _SC("couldn't create instance - no suitable constructor found"));
					
					// call the constructor
					instance = (C *)overloadInfo->callConstructor(vm);
					utils::setInstance(cls, instance, true, vm);

					if(!instance)
						return sq_throwerror(vm, _SC("couldn't create instance - no suitable constructor found"));
					return 0;
				}
				else if(!signature.size())
				{
					// no constructor given, try to use default
					// but only if no parameters were given
					instance = defaultConstructor(vm);
					utils::setInstance(cls, instance, true, vm);

					if(!instance)
						return sq_throwerror(vm, _SC("couldn't create instance - no default constructor"));
					return 0;
				}
				else
				{
					utils::setInstance(cls, (C *)NULL, false, vm);
					return sq_throwerror(vm, _SC("couldn't create instance - wrong number of parameters"));
				}
			}
			
			// copy constructor (_cloned metamethod) handler
			static SQInteger _copyConstructor(HSQUIRRELVM vm) 
			{
				// get the source instance
				HSQOBJECT source;
				if(!SQ_SUCCEEDED(sq_getstackobj(vm, -1, &source)))
					return sq_throwerror(vm, _SC("error getting source instance"));
				
				// get class info data
				ClassInfo const *classInfo = ClassInfo::get(source, vm);
				if(!classInfo)
					return sq_throwerror(vm, _SC("error getting class info object"));

				if(classInfo->cantCreate())
					return sq_throwerror(vm, _SC("instances creations are disabled for this class"));

				// get the copied instance
				HSQOBJECT copied;
				if(!SQ_SUCCEEDED(sq_getstackobj(vm, -2, &copied)))
					return sq_throwerror(vm, _SC("error getting copied instance"));
				
				// get the source instance C++ class
				C *sourceInstance = (C *)utils::getInstance(source, vm);
				if(!sourceInstance)
					return sq_throwerror(vm, _SC("error getting source C++ class"));
				C *destInstance = utils::copyConstruct(*sourceInstance);
				if(!destInstance) 
					return sq_throwerror(vm, _SC("object is not cloneable"));
				
				utils::setInstance(copied, destInstance, true, vm);
				return 0;
			}
			
		protected:
			
			// create base class structure and store its handler
			void init(SQFishTable *parent, bool ownsParent, LITSTR base, LITSTR name, HSQUIRRELVM v) 
			{
				_vm = v;
				_name = name;
				
				// store parent and gather its object
				_parent = parent;
				_ownsParent = ownsParent;
				pushTable(_vm, _parent);
		
				// try to get named object
				sq_pushstring(_vm, name->str(), -1, SQTrue);
		
				bool mustCreate = true;
				if(SQ_SUCCEEDED(sq_get(_vm, -2)))
				{
					// got something, let's check if it's a class
					SQObjectType typ = sq_gettype(_vm, -1);
					if(typ == OT_CLASS)
					{
						do
						{
							// it's a class, just check if is an already connected one
							// otherwise we shall re-create it
							// binded classes must contain:
							//		an __sqfish__ member which contain the class descriptor
							//		(must be set as a static member because class user data
							//		doesn't exists...)
							//		an userdata area with enough size for a CLASSUD struct
							//		the methametods _get and _set, but we don't check for them
							//		the methamedod _clone, which handles instance cloning
							//		a constructor(), which does the instance setup
							// for simplicity we just check the size and the __sqfish__ member
							if(sq_getsize(_vm, -1) != sizeof(InstanceInfo))
								break;
		
							sq_pushstring(_vm, _SC("__sqfish__"), -1, SQTrue);
							if(SQ_SUCCEEDED(sq_get(_vm, -2)))
								break;
							sq_poptop(_vm);
		
							// we could do more checks, but they're not necessary
							// the system should be tricked to fail here
							mustCreate = false;
		
						} while(0);
					
					}
					if(mustCreate)
						// discard the (class) object
						sq_poptop(_vm);
				}
		
				if(mustCreate)
				{
					// re-push the class name
					sq_pushstring(_vm, name->str(), -1, SQTrue);
					
					// if we provided a base, use it
					if(base && *base->str())
					{
						HSQOBJECT baseClass;
						utils::resolveName(_vm, _parent, base->str(), baseClass);
						sq_pushobject(_vm, baseClass);
						if(!SQ_SUCCEEDED(sq_newclass(_vm, SQTrue)))
						{
//							DEBUG_ERROR("Failed to create class\n");
						}
					}
					else
					{
						// create new class
						if(!SQ_SUCCEEDED(sq_newclass(_vm, SQFalse)))
						{
//							DEBUG_ERROR("Failed to create class\n");
						}
					}
		
					// create new slot in parent
					sq_newslot(_vm, -3, SQFalse);
		
					// re-fetch new class object
					sq_pushstring(_vm, name->str(), -1, SQTrue);
					sq_get(_vm, -2);
					
					// we need the object now
					sq_getstackobj(_vm, -1, &_obj);
					sq_addref(_vm, &_obj);

					/*ClassInfo *classInfo =*/ ClassInfo::create(_obj, _vm);
					
					// create the constructor
					sq_pushstring(_vm, _SC("constructor"), -1, SQTrue);
					sq_newclosure(_vm, _constructor, 0);
					sq_newslot(_vm, -3, SQFalse);
					
					// override the _clone
					sq_pushstring(_vm, _SC("_cloned"), -1, SQTrue);
					sq_newclosure(_vm, _copyConstructor, 0);
					sq_newslot(_vm, -3, SQFalse);
					
					// override _set and _get metamethods
					addSetMethod(_vm, _obj);
					addGetMethod(_vm, _obj);
					
					// register the class name along with its c++ type
					// to be able to return it from functions
					utils::registerClass(_vm, _parent, name->str(), typeid(C));
					utils::getTypeId<C>();
				}
				else
				{
					// get and store the class object
					sq_getstackobj(_vm, -1, &_obj);
					sq_addref(_vm, &_obj);
				}
		
				// pop new class and its parent handlers
				sq_pop(_vm, 2);
			}
			
		public:
			
			// destructor
			~SQFishClass() 
			{
				sq_release(_vm, &_obj);
				if(_ownsParent)
				{
					RLOG("SQFishClass parent deleted");
					delete _parent;
				}
			}
			
			// adds a type alias to class
			// it's used when we derive a C++ type to add/modify some member behaviours
			// BEWARE, aliased classes should NOT contain data but just member functions
			template<class T> SQFishClass &Alias(void) {
				// register the class name along with its c++ aliased type
				// to be able to return it from functions
				utils::registerClass(_vm, _parent, _name->str(), typeid(T));
				utils::getTypeId<T>(utils::getTypeId<C>());
				return *this;
			}
			
			// sets the 'Nocreate' behaviour -- this disallows new instances creation
			// useful for classes that are created in C++ (for example the standard streams)
			// see Instance() later
			SQFishClass &NoCreate(bool b = true)  {

				// get the info data
				ClassInfo *classInfo = ClassInfo::get(_obj, _vm);
				
				// set/reset the no create flag
				classInfo->setNoCreate(b);
				
				return *this;
			}
			
			// bind a constructor
			template<typename... PARMS>  SQFishClass &Constructor() {
				
				// get the info data
				ClassInfo *classInfo = ClassInfo::get(_obj, _vm);
				
				// get the signature string from params types
				SignatureID const *signature = utils::GetSignatureString<PARMS...>();
				
				// create a constructor
				OverloadInfo *overloadInfo = classInfo->createConstructorInfo(signature);
				if(!overloadInfo)
					return *this;

				// setup the true constructor
				overloadInfo->setConstructorHandler((SQFISH_FUNC_CONSTRUCTOR_HANDLER)constructorHandler<C, PARMS...>);
				
				// and finally the squirrel binding
		
				// push the table/class container
				sq_pushobject(_vm, _obj);
				
				// push the function name
				sq_pushstring(_vm, _SC("constructor"), -1, SQTrue);
				
				// create the closure
				sq_newclosure(_vm, (SQFUNCTION)_constructor, 0);
				
				// do NOT activate parameter check, it will be done inside handler!
				
				// create the slot
				sq_newslot(_vm, -3, SQFalse);

				// pop the table/class container
				sq_poptop(_vm);
				
				return *this;
			}

			// bind a member function
			template<typename RES, typename... PARMS>  SQFishClass &Func(LITSTR sqName, RES(C::*fn)(PARMS...)) {

				addMember(_obj, sqName, fn, _vm);
				return *this;
			}

			template<typename RES, typename... PARMS>  SQFishClass &Func(LITSTR sqName, RES(C::*fn)(PARMS...) const) {

				addMember(_obj, sqName, (RES(C::*)(PARMS...))fn, _vm);
				return *this;
			}

			template<typename RES, typename... PARMS, typename P>  SQFishClass &Func(LITSTR sqName, RES(P::*fn)(PARMS...)) {

				addMember(_obj, sqName, fn, _vm);
				return *this;
			}

			template<typename RES, typename... PARMS, typename P>  SQFishClass &Func(LITSTR sqName, RES(P::*fn)(PARMS...) const) {

				addMember(_obj, sqName, (RES(P::*)(PARMS...))fn, _vm);
				return *this;
			}

			// bind a global function
			template<typename RES, typename... PARMS>  SQFishClass &Func(LITSTR sqName, RES(*fn)(PARMS...)) {

				addGlobal(_obj, sqName, fn, _vm);
				return *this;
			}

			// bind a member variable
			template<typename T>  SQFishClass &Var(LITSTR sqName, T C::*val) {
				
				addVariable(_obj, sqName, val, _vm);
				return *this;
			}

			template<typename P, typename T>  SQFishClass &Var(LITSTR sqName, T P::*val) {
				
				addVariable(_obj, sqName, val, _vm);
				return *this;
			}

			// bind a global/static variable
			template<typename T>  SQFishClass &Var(LITSTR sqName, T *val) {
				
				addVariable(_obj, sqName, val, _vm);
				return *this;
			}

			// return to parent table
			SQFishTable &Parent(void)  { return *_parent; }
			SQFishTable &operator--(int)  { return *_parent; }
	};
}; // end namespace sqfish

#endif
