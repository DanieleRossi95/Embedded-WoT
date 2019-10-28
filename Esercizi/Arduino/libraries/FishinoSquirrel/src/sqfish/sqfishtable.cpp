#include "sqfish.h"

namespace sqfish
{
	// copy constructor -- should NOT be called on optimized compilers
	// but we provide it anyways to be sure to correctly handle classes
	// that owns their parent
	SQFishTable::SQFishTable(SQFishTable const &src)
	{
		_vm			= src._vm;
		_obj		= src._obj;
		_name		= src._name;
		sq_addref(_vm, &_obj);
		
		// if we owns our parent we shall deep copy it
		if(src._ownsParent)
		{
			_parent		= new SQFishTable(*src._parent);
			_ownsParent	= true;
		}
		else
		{
			_parent		= src._parent;
			_ownsParent	= false;
			
		}
	}

	// initialize object
	void SQFishTable::init(SQFishTable *parent, LITSTR name, HSQUIRRELVM v)
	{
		_vm = v;
		_name = name;
		
		// if no parent provided, use ourselves as parent
		// and the root table as object
		if(!parent)
		{
			_parent = new SQFishTable(ROOT_TABLE, v);
			_ownsParent = true;
			sq_pushroottable(_vm);
		}
		else
		{
			_parent = parent;
			_ownsParent = false;
			sq_pushobject(_vm, parent->_obj);
		}

		// try to get named table
		sq_pushstring(_vm, name->str(), -1, SQTrue);
		if(SQ_SUCCEEDED(sq_get(_vm, -2)))
		{
			// got something, let's check if it's a table
			SQObjectType typ = sq_gettype(_vm, -1);
			if(typ != OT_TABLE)
			{
				// discard the object
				sq_poptop(_vm);
				
				// re-push table name
				sq_pushstring(_vm, name->str(), -1, SQTrue);
				
				// create the table
				sq_newtable(_vm);

				sq_newslot(_vm, -3, SQFalse);

				sq_pushstring(_vm, name->str(), -1, SQTrue);
				sq_get(_vm, -2);
			}
		}
		else
		{
			// re-push table name
			sq_pushstring(_vm, name->str(), -1, SQTrue);
				
			// create new table
			sq_newtable(_vm);

			// create new slot in parent
			sq_newslot(_vm, -3, SQFalse);

			// re-fetch new table object
			sq_pushstring(_vm, name->str(), -1, SQTrue);
			sq_get(_vm, -2);
		}
		
		// get and store the table object
		sq_getstackobj(_vm, -1, &_obj);
		sq_addref(_vm, &_obj);
		
		// set registry table as delegate for this one
		sq_pushregistrytable(_vm);
		sq_setdelegate(_vm, -2);

		// pop this and parent table
		sq_pop(_vm, 2);
	}

	SQFishTable::SQFishTable(TABLES special, HSQUIRRELVM v)
	{
		_vm = v;
		_name = ""_LIT;
		
		if(special == ROOT_TABLE)
		{
			sq_pushroottable(_vm);
			
			// set registry table as root table delegate
			// maybe not the best idea, but....
			sq_pushregistrytable(_vm);
			sq_setdelegate(_vm, -2);
			
			// when creating the root table create also the _get
			// metamethod to its delegate (registry table)
			sq_getstackobj(_vm, -1, &_obj);
			addGetMethod(_vm, _obj);
		}
		else if(special == CONST_TABLE)
			sq_pushconsttable(_vm);
		else if(special == REGISTRY_TABLE)
			sq_pushregistrytable(_vm);
		else
		{
			sq_resetobject(&_obj);
			return;
		}
		sq_getstackobj(_vm, -1, &_obj);
		sq_addref(_vm, &_obj);
		
		// parent of a special table is always the same table
		// so we avoid walking back into nowhere
		_parent = this;
		_ownsParent = false;

		sq_poptop(_vm);
	}
	
	SQFishTable::SQFishTable(LITSTR name, HSQUIRRELVM v)
	{
		init(NULL, name, v);
	}

	SQFishTable::SQFishTable(SQFishTable *parent, LITSTR name, HSQUIRRELVM v)
	{
		init(parent, name, v);
	}

	// destructor
	SQFishTable::~SQFishTable()
	{
		sq_release(_vm, &_obj);
		if(_ownsParent)
		{
			RLOG("SQFishTable parent deleted");
			delete _parent;
		}
	}

	// get full name path from root
	std::string SQFishTable::getFullPath(void) const
	{
		std::string res = _name->str();
		SQFishTable *parent = _parent;
		while(parent && !parent->_name->empty())
		{
			res = std::string(parent->_name->str()) + "." + res;
			parent = parent->_parent;
		}
		return res;
	}

	// get table's child
	SQFishTable SQFishTable::Table(LITSTR name)
	{
		// no copy constructor is called here, but we provide it
		// anyways because it may cause problems on non-optimized compilers
		return SQFishTable(this, name, _vm);
	}
	
///////////////////////////////////////////////////////////////////////////////////////////

	SQFishTable Table(LITSTR name, HSQUIRRELVM v)
	{
		return SQFishTable(name, v);
	}
	
	SQFishTable RootTable(HSQUIRRELVM v)
	{
		return SQFishTable(ROOT_TABLE, v);
	}
	
	SQFishTable ConstTable(HSQUIRRELVM v)
	{
		return SQFishTable(CONST_TABLE, v);
	}
	
	SQFishTable RegistryTable(HSQUIRRELVM v)
	{
		return SQFishTable(REGISTRY_TABLE, v);
	}

	// helper -- needed because we're using templates in h files
	void pushTable(HSQUIRRELVM vm, SQFishTable *table)
	{
		sq_pushobject(vm, table->_obj);
	}
	
	
}; // end namespace sqfish
