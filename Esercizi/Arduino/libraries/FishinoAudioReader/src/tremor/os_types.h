/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2002    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.

 ********************************************************************/
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

#include <sys/types.h>
#include <inttypes.h>

#define LITTLE_ENDIAN 0
#define BIG_ENDIAN 1
#define BYTE_ORDER LITTLE_ENDIAN

#ifdef _LOW_ACCURACY_
#  define X(n) (((((n)>>22)+1)>>1) - ((((n)>>22)+1)>>9))
#  define LOOKUP_T const unsigned char
#else
#  define X(n) (n)
#  define LOOKUP_T const ogg_int32_t
#endif

// a couple of malloc hooks
extern void *__debug__malloc(size_t size);
extern void *__debug__calloc(size_t num, size_t size);
extern void *__debug__realloc(void *ptr, size_t size);
extern void __debug__free(void *ptr);

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
//#define OGG_MALLOC_DEBUG
#ifdef OGG_MALLOC_DEBUG
	#define _ogg_malloc  __debug__malloc
	#define _ogg_calloc  __debug__calloc
	#define _ogg_realloc __debug__realloc
	#define _ogg_free    __debug__free
#else
	#define _ogg_malloc  malloc
	#define _ogg_calloc  calloc
	#define _ogg_realloc realloc
	#define _ogg_free    free
#endif

typedef int64_t ogg_int64_t;
typedef int32_t ogg_int32_t;
typedef uint32_t ogg_uint32_t;
typedef int16_t ogg_int16_t;
typedef uint16_t ogg_uint16_t;

#endif  /* _OS_TYPES_H */
