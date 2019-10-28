#ifndef __SQFISH__TABLE__H
#define __SQFISH__TABLE__H

#include <string.h>

namespace sqfish
{
	
	enum TABLES {
		ROOT_TABLE,
		CONST_TABLE,
		REGISTRY_TABLE
	};
	
	// base class for tables and similar
	class SQFishTable
	{
		// allow return-by-copy
		friend SQFishTable Table(LITSTR name, HSQUIRRELVM v);
		friend SQFishTable RootTable(HSQUIRRELVM v);
		friend SQFishTable ConstTable(HSQUIRRELVM v);
		friend SQFishTable RegistryTable(HSQUIRRELVM v);
		
		template<class C> friend class SQFishClass;
		friend bool utils::resolveName(HSQUIRRELVM, SQFishTable *, const char *, HSQOBJECT &);
		
		// helper -- needed because we're using templates in h files
		friend void pushTable(HSQUIRRELVM vm, SQFishTable *table);

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
			
			// initialize object
			void init(SQFishTable *parent, LITSTR name, HSQUIRRELVM v) ;
			
			// constructor for child tables
			SQFishTable(SQFishTable *parent, LITSTR name, HSQUIRRELVM v) ;
			
			// avoid copy a table object externally
			// but used to copy tables that owns parents
			SQFishTable(SQFishTable const &) ;
			
		protected:
			
		public:
			
			// constructors
			SQFishTable(TABLES special, HSQUIRRELVM v) ;
			SQFishTable(LITSTR name, HSQUIRRELVM v) ;
			
			// destructor
			~SQFishTable()  ;

			// get full name path from root
			std::string getFullPath(void) const;

			// create child table
			SQFishTable Table(LITSTR name) ;
			
			template<typename RES, typename... PARMS> SQFishTable  &Func(LITSTR sqName, RES(*fn)(PARMS...)) {

				addGlobal(_obj, sqName, fn, _vm);
				return *this;
			}

			
			// add a variable to the table
			template<typename T> SQFishTable  &Value(LITSTR name, T val) {

				// push the table
				sq_pushobject(_vm, _obj);
				
				// push the name
				sq_pushstring(_vm, name->str(), -1, SQTrue);
				
				// push the value
				PushValue<T>(_vm, val);
				
				// create the slot
				sq_newslot(_vm, -3, SQFalse);
				
				// pop the table
				sq_poptop(_vm);
				
				return *this;
			}
			
			// add a class to the table
			template<class C> SQFishClass<C>  Class(LITSTR name)
			{
				return SQFishClass<C>(this, false, name, _vm);
			}
			
			template<class C> SQFishClass<C>  Class(LITSTR base, LITSTR name)
			{
				return SQFishClass<C>(this, false, base, name, _vm);
			}
			
			// create an instance of the class from an existing C++ instance
			// useful, for example, to bind standard streams, or already initialized devices
			template<class C> SQFishTable  &Instance(LITSTR instanceName, LITSTR classPath, C &cppInstance) {
				
				// get the class object
				HSQOBJECT cls;
				if(!utils::resolveName(_vm, _obj, classPath->str(), cls))
				{
					RLOG("ERROR RESOLVING CLASS PATH '" << classPath->str() << "'");
					return *this;
				}
				
				// create the instance
				sq_pushobject(_vm, cls);
				if(!SQ_SUCCEEDED(sq_createinstance(_vm, -1)))
				{
					RLOG("ERROR CREATING THE INSTANCE");
					sq_poptop(_vm);
					return *this;
				}
				
				// get the instance back
				HSQOBJECT inst;
				sq_getstackobj(_vm, -1, &inst);
				sq_addref(_vm, &inst);
				
				// pop back both the class and the instance object
				sq_pop(_vm, 2);
				
				// now create the slot in current table
				sq_pushobject(_vm, _obj);
				sq_pushstring(_vm, instanceName->str(), -1, SQTrue);
				sq_pushobject(_vm, inst);
				if(!SQ_SUCCEEDED(sq_newslot(_vm, -3, SQFalse)))
				{
					sq_pop(_vm, 3);
					RLOG("ERROR CREATING THE INSTANCE SLOT");
					return *this;
				}
				
				// remove the additional ref to instance object
				sq_release(_vm, &inst);
				
				// setup instance class pointer
				// the C++ instance is NOT owned by us here
				utils::setInstance(inst, &cppInstance, false, _vm);
				return *this;
			}
				
			// return to parent table
			SQFishTable &Parent(void) { return *_parent; }
			SQFishTable &operator--(int) { return *_parent; }
	};
	
	// build a named table inside root table
	SQFishTable Table(LITSTR name, HSQUIRRELVM v) ;
	
	// get system tables
	SQFishTable RootTable(HSQUIRRELVM v) ;
	SQFishTable ConstTable(HSQUIRRELVM v) ;
	SQFishTable RegistryTable(HSQUIRRELVM v) ;

	// construct a class in the root table
	template<class C> SQFishClass<C>  Class(LITSTR name, HSQUIRRELVM v)
	{
		// creating a class on the fly in root table means that we shall
		// create a parent, root table inside class itself
		// again, this means that we NEED a copy constructor for SQFishClass
		SQFishTable *table = new SQFishTable(ROOT_TABLE, v);
		return SQFishClass<C>(table, true, name, v);
	}

	// construct a class in the root table
	template<class C> SQFishClass<C>  Class(LITSTR base, LITSTR name, HSQUIRRELVM v)
	{
		// creating a class on the fly in root table means that we shall
		// create a parent, root table inside class itself
		// again, this means that we NEED a copy constructor for SQFishClass
		SQFishTable *table = new SQFishTable(ROOT_TABLE, v);
		return SQFishClass<C>(table, true, base, name, v);
	}

}; // end namespace sqfish


#endif
