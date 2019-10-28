/*
    see copyright notice in squirrel.h
*/
#include "sqpcheader.h"
#include "sqvm.h"
#include "sqtable.h"
#include "sqclass.h"
#include "sqfuncproto.h"
#include "sqclosure.h"


#ifndef SQUIRREL_SINGLE_SHAREDSTATE
SQClass::SQClass(SQSharedState *ss,SQClass *base)
#else
SQClass::SQClass(SQClass *base)
#endif
{
    _base = base;
    _typetag = 0;
    _hook = NULL;
    _udsize = 0;
    _locked = false;
    _constructoridx = -1;
    if(_base) {
        _constructoridx = _base->_constructoridx;
        _udsize = _base->_udsize;
        _defaultvalues.copy(base->_defaultvalues);
        _methods.copy(base->_methods);
        _COPY_VECTOR(_metamethods,base->_metamethods,MT_LAST);
        __ObjAddRef(_base);
    }
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    _members = base?base->_members->Clone() : SQTable::Create(ss,0);
#else
    _members = base?base->_members->Clone() : SQTable::Create(0);
#endif
    __ObjAddRef(_members);

    INIT_CHAIN();
    ADD_TO_CHAIN(&SHAREDSTATE->_gc_chain, this);
}

void SQClass::Finalize() {
    _attributes.Null();
    _NULL_SQOBJECT_VECTOR(_defaultvalues,_defaultvalues.size());
    _methods.resize(0);
    _NULL_SQOBJECT_VECTOR(_metamethods,MT_LAST);
    __ObjRelease(_members);
    if(_base) {
        __ObjRelease(_base);
    }
}

SQClass::~SQClass()
{
    REMOVE_FROM_CHAIN(&SHAREDSTATE->_gc_chain, this);
    Finalize();
}

#ifndef SQUIRREL_SINGLE_SHAREDSTATE
bool SQClass::NewSlot(SQSharedState *ss,const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic)
#else
bool SQClass::NewSlot(const SQObjectPtr &key,const SQObjectPtr &val,bool bstatic)
#endif
{
    SQObjectPtr temp;
    bool belongs_to_static_table = sq_type(val) == OT_CLOSURE || sq_type(val) == OT_NATIVECLOSURE || bstatic;
    if(_locked && !belongs_to_static_table)
        return false; //the class already has an instance so cannot be modified
    if(_members->Get(key,temp) && _isfield(temp)) //overrides the default value
    {
        _defaultvalues[_member_idx(temp)].val = val;
        return true;
    }
    if(belongs_to_static_table) {
        SQInteger mmidx;
        if((sq_type(val) == OT_CLOSURE || sq_type(val) == OT_NATIVECLOSURE) &&
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
            (mmidx = ss->GetMetaMethodIdxByName(key)) != -1) {
#else
            (mmidx = GlobalSharedstate->GetMetaMethodIdxByName(key)) != -1) {
#endif
            _metamethods[mmidx] = val;
        }
        else {
            SQObjectPtr theval = val;
            if(_base && sq_type(val) == OT_CLOSURE) {
                theval = _closure(val)->Clone();
                _closure(theval)->_base = _base;
                __ObjAddRef(_base); //ref for the closure
            }
            if(sq_type(temp) == OT_NULL) {
                bool isconstructor;
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
                SQVM::IsEqual(ss->_constructoridx, key, isconstructor);
#else
                SQVM::IsEqual(GlobalSharedstate->_constructoridx, key, isconstructor);
#endif
                if(isconstructor) {
                    _constructoridx = (SQInteger)_methods.size();
                }
                SQClassMember m;
                m.val = theval;
                _members->NewSlot(key,SQObjectPtr(_make_method_idx(_methods.size())));
                _methods.push_back(m);
            }
            else {
                _methods[_member_idx(temp)].val = theval;
            }
        }
        return true;
    }
    SQClassMember m;
    m.val = val;
    _members->NewSlot(key,SQObjectPtr(_make_field_idx(_defaultvalues.size())));
    _defaultvalues.push_back(m);
    return true;
}

SQInstance *SQClass::CreateInstance()
{
    if(!_locked) Lock();
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    return SQInstance::Create(_opt_ss(this),this);
#else
    return SQInstance::Create(this);
#endif
}

SQInteger SQClass::Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval)
{
    SQObjectPtr oval;
    SQInteger idx = _members->Next(false,refpos,outkey,oval);
    if(idx != -1) {
        if(_ismethod(oval)) {
            outval = _methods[_member_idx(oval)].val;
        }
        else {
            SQObjectPtr &o = _defaultvalues[_member_idx(oval)].val;
            outval = _realval(o);
        }
    }
    return idx;
}

bool SQClass::SetAttributes(const SQObjectPtr &key,const SQObjectPtr &val)
{
    SQObjectPtr idx;
    if(_members->Get(key,idx)) {
        if(_isfield(idx))
            _defaultvalues[_member_idx(idx)].attrs = val;
        else
            _methods[_member_idx(idx)].attrs = val;
        return true;
    }
    return false;
}

bool SQClass::GetAttributes(const SQObjectPtr &key,SQObjectPtr &outval)
{
    SQObjectPtr idx;
    if(_members->Get(key,idx)) {
        outval = (_isfield(idx)?_defaultvalues[_member_idx(idx)].attrs:_methods[_member_idx(idx)].attrs);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
void SQInstance::Init(SQSharedState *ss)
#else
void SQInstance::Init()
#endif
{
    _userpointer = NULL;
    _hook = NULL;
    __ObjAddRef(_class);
    _delegate = _class->_members;
    INIT_CHAIN();
    ADD_TO_CHAIN(&SHAREDSTATE->_gc_chain, this);
}

#ifndef SQUIRREL_SINGLE_SHAREDSTATE
SQInstance::SQInstance(SQSharedState *ss, SQClass *c, SQInteger memsize)
#else
SQInstance::SQInstance(SQClass *c, SQInteger memsize)
#endif
{
    _memsize = memsize;
    _class = c;
    SQUnsignedInteger nvalues = _class->_defaultvalues.size();
    for(SQUnsignedInteger n = 0; n < nvalues; n++) {
        new (&_values[n]) SQObjectPtr(_class->_defaultvalues[n].val);
    }
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    Init(ss);
#else
    Init();
#endif
}

#ifndef SQUIRREL_SINGLE_SHAREDSTATE
SQInstance::SQInstance(SQSharedState *ss, SQInstance *i, SQInteger memsize)
#else
SQInstance::SQInstance(SQInstance *i, SQInteger memsize)
#endif
{
    _memsize = memsize;
    _class = i->_class;
    SQUnsignedInteger nvalues = _class->_defaultvalues.size();
    for(SQUnsignedInteger n = 0; n < nvalues; n++) {
        new (&_values[n]) SQObjectPtr(i->_values[n]);
    }
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    Init(ss);
#else
    Init();
#endif
}

void SQInstance::Finalize()
{
    SQUnsignedInteger nvalues = _class->_defaultvalues.size();
    __ObjRelease(_class);
    _NULL_SQOBJECT_VECTOR(_values,nvalues);
}

SQInstance::~SQInstance()
{
    REMOVE_FROM_CHAIN(&SHAREDSTATE->_gc_chain, this);
    if(_class){ Finalize(); } //if _class is null it was already finalized by the GC
}

bool SQInstance::GetMetaMethod(SQVM* SQ_UNUSED_ARG(v),SQMetaMethod mm,SQObjectPtr &res)
{
    if(sq_type(_class->_metamethods[mm]) != OT_NULL) {
        res = _class->_metamethods[mm];
        return true;
    }
    return false;
}

bool SQInstance::InstanceOf(SQClass *trg)
{
    SQClass *parent = _class;
    while(parent != NULL) {
        if(parent == trg)
            return true;
        parent = parent->_base;
    }
    return false;
}
