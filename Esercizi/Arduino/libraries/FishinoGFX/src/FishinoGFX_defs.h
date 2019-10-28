// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
	#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
	#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#ifndef min
	#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
	#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif
