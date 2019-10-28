#include <stdio.h>

#include "sqpcheader.h"

#if defined(SQRAT_MEMORY_TEST) || defined(SQRAT_MEMORY_SHOW_ALLOC)
size_t __sqrat__malloc__total = 0;
#endif

#ifdef SQRAT_MEMORY_TEST
extern void __sqrat__refresh__malloc(void);
#endif

#ifdef SQRAT_MEMORY_SHOW_ALLOC
extern void printSqratAlloc(size_t prev, size_t next);
#endif

void *sqrat_malloc(SQUnsignedInteger size)
{
#if defined(SQRAT_MEMORY_TEST) || defined(SQRAT_MEMORY_SHOW_ALLOC)
	__sqrat__malloc__total += size;
#endif
#ifdef SQRAT_MEMORY_TEST
	__sqrat__refresh__malloc();
#endif
#ifdef SQRAT_MEMORY_SHOW_ALLOC
	printSqratAlloc(0, size);
#endif
	return malloc(size);
}

void *sqrat_realloc(void *p, SQUnsignedInteger SQ_UNUSED_ARG(oldsize), SQUnsignedInteger size)
{
#if defined(SQRAT_MEMORY_TEST) || defined(SQRAT_MEMORY_SHOW_ALLOC)
	__sqrat__malloc__total = __sqrat__malloc__total + size - oldsize;
#endif
#ifdef SQRAT_MEMORY_TEST
	__sqrat__refresh__malloc();
#endif
#ifdef SQRAT_MEMORY_SHOW_ALLOC
	printSqratAlloc(oldsize, size);
#endif
	return realloc(p, size);
}

void sqrat_free(void *p, SQUnsignedInteger SQ_UNUSED_ARG(size))
{
#if defined(SQRAT_MEMORY_TEST) || defined(SQRAT_MEMORY_SHOW_ALLOC)
	__sqrat__malloc__total -= size;
#endif
#ifdef SQRAT_MEMORY_TEST
	__sqrat__refresh__malloc();
#endif
#ifdef SQRAT_MEMORY_SHOW_ALLOC
	printSqratAlloc(size, 0);
#endif
	free(p);
}
