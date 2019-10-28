/*  see copyright notice in squirrel.h */
#ifndef _SQSTRING_H_
#define _SQSTRING_H_

inline SQHash  _hashstr (const SQChar *s, size_t l)
{
        SQHash h = (SQHash)l;  /* seed */
        size_t step = (l>>5)|1;  /* if string is too long, don't hash all its chars */
        for (; l>=step; l-=step)
            h = h ^ ((h<<5)+(h>>2)+(unsigned short)*(s++));
        return h;
}

struct SQString : public SQRefCounted
{
     SQString(){}
     ~SQString(){}
public:
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    static SQString *Create(SQSharedState *ss, const SQChar *, SQInteger len, SQBool isConst) ;
#else
    static SQString *Create(const SQChar *, SQInteger len, SQBool isConst) ;
#endif
    SQInteger Next(const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval) ;
    void Release() ;
#ifndef SQUIRREL_SINGLE_SHAREDSTATE
    SQSharedState *_sharedstate;
#endif
    SQString *_next; //chain for the string table
    SQInteger _len;
#ifndef SQSTRING_DONT_STORE_HASH
    SQHash _hash;
#else
    SQHash  hash() { return _hashstr(_val, _len); }
#endif
    SQChar *_val;
} SQ_PACKED_ATTRIBUTE;



#endif //_SQSTRING_H_
