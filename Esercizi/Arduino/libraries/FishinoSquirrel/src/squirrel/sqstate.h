/*  see copyright notice in squirrel.h */
#ifndef _SQSTATE_H_
#define _SQSTATE_H_

#include "squtils.h"
#include "sqobject.h"
struct SQString;
struct SQTable;
//max number of character for a printed number
#define NUMBER_MAX_CHAR 50

struct SQStringTable
{
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    SQStringTable(SQSharedState*ss) ;
#else
    SQStringTable() ;
#endif
    ~SQStringTable() ;
    SQString *Add(const SQChar *,SQInteger len, SQBool isConst) ;
    void Remove(SQString *) ;
private:
    void Resize(SQInteger size) ;
    void AllocNodes(SQInteger size) ;
    SQString **_strings;
    SQUnsignedInteger _numofslots;
    SQUnsignedInteger _slotused;
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    SQSharedState *_sharedstate;
#endif
};

struct RefTable {
    struct RefNode {
        SQObjectPtr obj;
        SQUnsignedInteger refs;
        struct RefNode *next;
    };
    RefTable() ;
    ~RefTable() ;
    void AddRef(SQObject &obj) ;
    SQBool Release(SQObject &obj) ;
    SQUnsignedInteger GetRefCount(SQObject &obj) ;
#ifndef NO_GARBAGE_COLLECTOR
    void Mark(SQCollectable **chain) ;
#endif
    void Finalize() ;
private:
    RefNode *Get(SQObject &obj,SQHash &mainpos,RefNode **prev,bool add) ;
    RefNode *Add(SQHash mainpos,SQObject &obj) ;
    void Resize(SQUnsignedInteger size) ;
    void AllocNodes(SQUnsignedInteger size) ;
    SQUnsignedInteger _numofslots;
    SQUnsignedInteger _slotused;
    RefNode *_nodes;
    RefNode *_freelist;
    RefNode **_buckets;
};

// weak references table
// used only if SQUIRREL_WEAKREF_TABLE is enabled
// it maps an object with its weak reference object
// as weak references are seldom, we use a simple linear search
#ifdef SQUIRREL_WEAKREF_TABLE
class SQWeakRefTable {
	
	private:

		struct SQWeakRefLink {
			SQRefCounted *dest;
			SQWeakRef *ref;
		};
		
		sqvector<SQWeakRefLink> _links;
		
	protected:
		
	public:
		
		// get / add a weak reference to a refcounted
		SQWeakRef *GetRef(SQObjectType type, SQRefCounted *refCounted) ;
		
		// remove weak ref for object
		void RemoveRef(SQRefCounted *refCounted) ;
};
#endif

#define ADD_STRING(ss,str,len,isConst) ss->_stringtable->Add(str,len, isConst)
#define REMOVE_STRING(ss,bstr) ss->_stringtable->Remove(bstr)

struct SQObjectPtr;

struct SQSharedState
{
    SQSharedState() ;
    ~SQSharedState() ;
    void Init() ;
public:
    SQChar* GetScratchPad(SQInteger size) ;
    SQInteger GetMetaMethodIdxByName(const SQObjectPtr &name) ;
#ifndef NO_GARBAGE_COLLECTOR
    SQInteger CollectGarbage(SQVM *vm) ;
    void RunMark(SQVM *vm,SQCollectable **tchain) ;
    SQInteger ResurrectUnreachable(SQVM *vm) ;
    static void MarkObject(SQObjectPtr &o,SQCollectable **chain) ;
#endif
    SQObjectPtrVec *_metamethods;
    SQObjectPtr _metamethodsmap;
    SQObjectPtrVec *_systemstrings;
    SQObjectPtrVec *_types;
    SQStringTable *_stringtable;
    RefTable _refs_table;
    SQObjectPtr _registry;
    SQObjectPtr _consts;
    SQObjectPtr _constructoridx;
#ifndef NO_GARBAGE_COLLECTOR
    SQCollectable *_gc_chain;
#endif

#ifdef SQUIRREL_WEAKREF_TABLE
    SQWeakRefTable _weakRrefs;
#endif

    SQObjectPtr _root_vm;
    SQObjectPtr _table_default_delegate;
    static const SQRegFunction _table_default_delegate_funcz[];
    SQObjectPtr _array_default_delegate;
    static const SQRegFunction _array_default_delegate_funcz[];
    SQObjectPtr _string_default_delegate;
    static const SQRegFunction _string_default_delegate_funcz[];
    SQObjectPtr _number_default_delegate;
    static const SQRegFunction _number_default_delegate_funcz[];
    SQObjectPtr _generator_default_delegate;
    static const SQRegFunction _generator_default_delegate_funcz[];
    SQObjectPtr _closure_default_delegate;
    static const SQRegFunction _closure_default_delegate_funcz[];
    SQObjectPtr _thread_default_delegate;
    static const SQRegFunction _thread_default_delegate_funcz[];
    SQObjectPtr _class_default_delegate;
    static const SQRegFunction _class_default_delegate_funcz[];
    SQObjectPtr _instance_default_delegate;
    static const SQRegFunction _instance_default_delegate_funcz[];
    SQObjectPtr _weakref_default_delegate;
    static const SQRegFunction _weakref_default_delegate_funcz[];

    SQCOMPILERERROR _compilererrorhandler;
    SQPRINTFUNCTION _printfunc;
    SQPRINTFUNCTION _errorfunc;
    bool _debuginfo;
    bool _notifyallexceptions;
    SQUserPointer _foreignptr;
    SQRELEASEHOOK _releasehook;
    
    // stuffs to allow VM timed breaks
    SQInteger _breakLoops;
    SQInteger _breakCounter;
    SQBREAKHOOK _breakHook;
    SQBool _shutdown;
private:
    SQChar *_scratchpad;
    SQInteger _scratchpadsize;
};

	#define _sp(s) (SHAREDSTATE->GetScratchPad(s))
	#define _spval (SHAREDSTATE->GetScratchPad(-1))
	
	#define _table_ddel     _table(SHAREDSTATE->_table_default_delegate)
	#define _array_ddel     _table(SHAREDSTATE->_array_default_delegate)
	#define _string_ddel    _table(SHAREDSTATE->_string_default_delegate)
	#define _number_ddel    _table(SHAREDSTATE->_number_default_delegate)
	#define _generator_ddel _table(SHAREDSTATE->_generator_default_delegate)
	#define _closure_ddel   _table(SHAREDSTATE->_closure_default_delegate)
	#define _thread_ddel    _table(SHAREDSTATE->_thread_default_delegate)
	#define _class_ddel     _table(SHAREDSTATE->_class_default_delegate)
	#define _instance_ddel  _table(SHAREDSTATE->_instance_default_delegate)
	#define _weakref_ddel   _table(SHAREDSTATE->_weakref_default_delegate)

bool CompileTypemask(SQIntVec &res,const SQChar *typemask) ;

#ifdef SQUIRREL_SINGLE_SHAREDSTATE
extern SQSharedState *GlobalSharedstate;
#endif

#endif //_SQSTATE_H_
