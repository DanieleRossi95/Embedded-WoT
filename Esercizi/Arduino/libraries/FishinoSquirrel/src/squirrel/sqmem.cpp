/*
    see copyright notice in squirrel.h
*/
#include <stdio.h>

#include "sqpcheader.h"
#ifndef SQ_EXCLUDE_DEFAULT_MEMFUNCTIONS

#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
size_t __malloc__total = 0;
#endif

#ifdef SQUIRREL_MEMORY_TEST
extern void __refresh__malloc(void);
#endif

#ifdef SQUIRREL_MEMORY_SHOW_ALLOC
extern void printAlloc(size_t prev, size_t next);
#endif

void *sq_vm_malloc(SQUnsignedInteger size)
{
#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	__malloc__total += size;
#endif
#ifdef SQUIRREL_MEMORY_TEST
	__refresh__malloc();
#endif
#ifdef SQUIRREL_MEMORY_SHOW_ALLOC
	printAlloc(0, size);
#endif
	return malloc(size);
}

void *sq_vm_realloc(void *p, SQUnsignedInteger SQ_UNUSED_ARG(oldsize), SQUnsignedInteger size)
{
#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	__malloc__total = __malloc__total + size - oldsize;
#endif
#ifdef SQUIRREL_MEMORY_TEST
	__refresh__malloc();
#endif
#ifdef SQUIRREL_MEMORY_SHOW_ALLOC
	printAlloc(oldsize, size);
#endif
	return realloc(p, size);
}

void sq_vm_free(void *p, SQUnsignedInteger SQ_UNUSED_ARG(size))
{
#if defined(SQUIRREL_MEMORY_TEST) || defined(SQUIRREL_MEMORY_SHOW_ALLOC)
	__malloc__total -= size;
#endif
#ifdef SQUIRREL_MEMORY_TEST
	__refresh__malloc();
#endif
#ifdef SQUIRREL_MEMORY_SHOW_ALLOC
	printAlloc(size, 0);
#endif
	free(p);
}
#endif
