//////////////////////////////////////////////////////////////////////////////////////
//						    FishinoJPEGDecoder.cpp									//
//																					//
//					  JPEG Decoder for Fishino boards								//
//																					//
//		Copyright (c) 2017 Massimo Del Fedele.  All rights reserved.				//
//																					//
//	Redistribution and use in source and binary forms, with or without				//
//	modification, are permitted provided that the following conditions are met:		//
//																					//
//	- Redistributions of source code must retain the above copyright notice,		//
//	  this list of conditions and the following disclaimer.							//
//	- Redistributions in binary form must reproduce the above copyright notice,		//
//	  this list of conditions and the following disclaimer in the documentation		//
//	  and/or other materials provided with the distribution.						//
//																					//	
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"		//
//	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE		//
//	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE		//
//	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE		//
//	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR				//
//	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF			//
//	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS		//
//	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN			//
//	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)			//
//	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE		//
//	POSSIBILITY OF SUCH DAMAGE.														//
//																					//
//  Version 5.0.0 -- May 2017		Initial version									//
//  Version 5.2.0 -- May 2017		Used new class FishinoStream					//
//  Version 6.0.2 -- May 2017		Some small fixes for FishinoHttpInStream		//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#include "FishinoJPEGDecoder.h"

#define DEBUG_LEVEL_INFO
#include <FishinoDebug.h>

// Size of stream input buffer
#define	JD_SZBUF		512

// size of rotation table
#define JD_SZROTBUF		(256 * 3)

// Use table for saturation (might be a bit faster but increases 1K bytes of code size)
#define JD_TBLCLIP		0

// work buffer size
#define JDR_POOL_SIZE (2588 + JD_SZROTBUF + JD_SZBUF)

static void inline swap(JDR_WORD &a, JDR_WORD &b) { JDR_WORD tmp = a; a = b; b = tmp; }
static void inline swap(JDR_SHORT &a, JDR_SHORT &b) { JDR_SHORT tmp = a; a = b; b = tmp; }

// Zigzag-order to raster-order conversion table
#define ZIG(n)	Zig[n]
static const JDR_BYTE Zig[64] =
{
	0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

// Input scale factor of Arai algorithm
// (scaled up 16 bits for fixed point operations)
// See also aa_idct.png
#define IPSF(n)	Ipsf[n]
#define _E(x) (JDR_WORD)(x*8192)
static const JDR_WORD Ipsf[64] =
{
	_E(1.00000), _E(1.38704), _E(1.30656), _E(1.17588), _E(1.00000), _E(0.78570), _E(0.54120), _E(0.27590),
	_E(1.38704), _E(1.92388), _E(1.81226), _E(1.63099), _E(1.38704), _E(1.08979), _E(0.75066), _E(0.38268),
	_E(1.30656), _E(1.81226), _E(1.70711), _E(1.53636), _E(1.30656), _E(1.02656), _E(0.70711), _E(0.36048),
	_E(1.17588), _E(1.63099), _E(1.53636), _E(1.38268), _E(1.17588), _E(0.92388), _E(0.63638), _E(0.32442),
	_E(1.00000), _E(1.38704), _E(1.30656), _E(1.17588), _E(1.00000), _E(0.78570), _E(0.54120), _E(0.27590),
	_E(0.78570), _E(1.08979), _E(1.02656), _E(0.92388), _E(0.78570), _E(0.61732), _E(0.42522), _E(0.21677),
	_E(0.54120), _E(0.75066), _E(0.70711), _E(0.63638), _E(0.54120), _E(0.42522), _E(0.29290), _E(0.14932),
	_E(0.27590), _E(0.38268), _E(0.36048), _E(0.32442), _E(0.27590), _E(0.21678), _E(0.14932), _E(0.07612)
};
#undef _E

// Conversion table for fast clipping process
#if JD_TBLCLIP

#define BYTECLIP(v) Clip8[(JDR_UINT)(v) & 0x3FF]

static const JDR_BYTE Clip8[1024] =
{
	// 0..255
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
	// 256..511
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	// -512..-257
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	// -256..-1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#else

inline JDR_BYTE BYTECLIP(JDR_INT val)
{
	if(val < 0)
		val = 0;
	else if(val > 255)
		val = 255;
	return (JDR_BYTE)val;
}

#endif


// Allocate a memory block from memory pool
void* FishinoJPEGDecoder::allocPool(JDR_UINT nd)
{
	char *rp = 0;

	// Align block size to the word boundary
	nd = (nd + 3) & ~3;
	if (_poolFreeSize >= nd)
	{
		_poolFreeSize -= nd;

		// Get start of available memory pool
		rp = (char*)_poolPtr;

		// Allocate requierd bytes
		_poolPtr = (void*)(rp + nd);
	}
	// Return allocated memory block (NULL:no memory to allocate)
	return (void*)rp;
}

// skip data from input stream
bool FishinoJPEGDecoder::skipInput(JDR_WORD len)
{
	JDR_ULONG dummy;
	while(len > sizeof(JDR_ULONG))
	{
		if(_stream->read((uint8_t *)&dummy, sizeof(JDR_ULONG)) != sizeof(JDR_ULONG))
			return false;
		len -= 4;
	}
	if(!len)
		return true;
	return _stream->read((uint8_t *)&dummy, len) == len;
}

// Create de-quantization and prescaling tables with a DQT segment
// return 0 if OK, != 0 if Failed
JDR_UINT FishinoJPEGDecoder::createQtTable(
	const JDR_BYTE* data,	// Pointer to the quantizer tables
	JDR_UINT ndata			// Size of input data
)
{
	JDR_UINT i;
	JDR_BYTE d, z;
	JDR_LONG *pb;

  	// Process all tables in the segment
	while(ndata)
	{
		if (ndata < 65)
			// Err: table size is unaligned
			return JDR_FMT1;
		ndata -= 65;

		// Get table property
		d = *data++;
		if (d & 0xF0)
			// Err: not 8-bit resolution
			return JDR_FMT1;

		// Get table ID
		i = d & 3;

		// Allocate a memory block for the table
		pb = (JDR_LONG *)allocPool(64 * sizeof(JDR_LONG));
		if (!pb)
			// Err: not enough memory
			return JDR_MEM1;

		// Register the table
		_qttbl[i] = pb;

		// Load the table
		for (i = 0; i < 64; i++)
		{
			// Zigzag-order to raster-order conversion
			z = ZIG(i);

			// Apply scale factor of Arai algorithm to the de-quantizers
			pb[z] = (JDR_LONG)((JDR_DWORD) * data++ * IPSF(z));
		}
	}

	return JDR_OK;
}


// Create huffman code tables with a DHT segment
// returns 0 if OK, != 0 if Failed
JDR_UINT FishinoJPEGDecoder::createHuffmanTable(
	const JDR_BYTE* data,	// Pointer to the packed huffman tables
	JDR_UINT ndata			// Size of input data
)
{
	JDR_UINT i, j, b, np, cls, num;
	JDR_BYTE d, *pb, *pd;
	JDR_WORD hc, *ph;

  	// Process all tables in the segment
	while (ndata)
	{
		if (ndata < 17)
			// Err: wrong data size
			return JDR_FMT1;
		ndata -= 17;

		// Get table number and class
		d = *data++;
		cls = (d >> 4);

		// class = dc(0)/ac(1), table number = 0/1
		num = d & 0x0F;
		if(d & 0xEE)
			// Err: invalid class/number
			return JDR_FMT1;

		// Allocate a memory block for the bit distribution table
		pb = (JDR_BYTE *)allocPool(16);
		if(!pb)
			// Err: not enough memory
			return JDR_MEM1;

		_huffbits[num][cls] = pb;
		// Load number of patterns for 1 to 16-bit code
		for(np = i = 0; i < 16; i++)
		{
			pb[i] = b = *data++;
			// Get sum of code words for each code
			np += b;
		}

		// Allocate a memory block for the code word table
		ph = (JDR_WORD *)allocPool(np * sizeof(JDR_WORD));
		if(!ph)
			// Err: not enough memory
			return JDR_MEM1;

		_huffcode[num][cls] = ph;
		hc = 0;
		// Re-build huffman code word table
		for(j = i = 0; i < 16; i++)
		{
			b = pb[i];
			while (b--) ph[j++] = hc++;
			hc <<= 1;
		}

		if (ndata < np)
			// Err: wrong data size
			return JDR_FMT1;

		ndata -= np;
		// Allocate a memory block for the decoded data
		pd = (JDR_BYTE *)allocPool(np);
		if (!pd)
			// Err: not enough memory
			return JDR_MEM1;

		_huffdata[num][cls] = pd;
		// Load decoded data corresponds to each code ward
		for (i = 0; i < np; i++)
		{
			d = *data++;
			if (!cls && d > 11)
				return JDR_FMT1;
			*pd++ = d;
		}
	}

	return JDR_OK;
}

// Extract N bits from input stream
// >=0: extracted data, <0: error code
JDR_INT FishinoJPEGDecoder::bitext(
	// Number of bits to extract (1 to 11)
	JDR_UINT nbit
)
{
	JDR_BYTE msk, s, *dp;
	JDR_UINT dc, v, f;

	msk = _dmsk;
	dc = _dctr;
	// Bit mask, number of data available, read ptr
	dp = _dptr;
	s = *dp;
	v = f = 0;
	do
	{
		// Next byte?
		if (!msk)
		{
			// No input data is available, re-fill input buffer
			if (!dc)
			{
				// Top of input buffer
				dp = _inbuf;
				dc = _stream->read(dp, JD_SZBUF);
				if (!dc)
					// Err: read error or wrong stream termination
					return 0 - (JDR_INT)JDR_INP;
			}
			else
			{
				// Next data ptr
				dp++;
			}
			// Decrement number of available bytes
			dc--;
			// In flag sequence?
			if(f)
			{
				// Exit flag sequence
				f = 0;
				if(*dp != 0)
					// Err: unexpected flag is detected (may be collapted data)
					return 0 - (JDR_INT)JDR_FMT1;
				// The flag is a data 0xFF
				*dp = s = 0xFF;
			}
			else
			{
				// Get next data byte
				s = *dp;
				// Is start of flag sequence?
				if(s == 0xFF)
				{
					f = 1;
					// Enter flag sequence
					continue;
				}
			}
			// Read from MSB
			msk = 0x80;
		}
		// Get a bit
		v <<= 1;
		if(s & msk)
			v++;
		msk >>= 1;
		nbit--;
	}
	while(nbit);

	_dmsk = msk;
	_dctr = dc;
	_dptr = dp;

	return (JDR_INT)v;
}

// Extract a huffman decoded data from input stream
// >=0: decoded data, <0: error code
JDR_INT FishinoJPEGDecoder::huffext(
	const JDR_BYTE* hbits,	// Pointer to the bit distribution table
	const JDR_WORD* hcode,	// Pointer to the code word table
	const JDR_BYTE* hdata	// Pointer to the data table
)
{
	JDR_BYTE msk, s, *dp;
	JDR_UINT dc, v, f, bl, nd;


	msk = _dmsk;
	dc = _dctr;
	// Bit mask, number of data available, read ptr
	dp = _dptr;
	s = *dp;
	v = f = 0;
	// Max code length
	bl = 16;
	do
	{
		// Next byte?
		if (!msk)
		{
			// No input data is available, re-fill input buffer
			if (!dc)
			{
				// Top of input buffer
				dp = _inbuf;
				dc = _stream->read(dp, JD_SZBUF);
				// Err: read error or wrong stream termination
				if (!dc) return
					0 - (JDR_INT)JDR_INP;
			}
			else
			{
				// Next data ptr
				dp++;
			}
			// Decrement number of available bytes
			dc--;
			// In flag sequence?
			if(f)
			{
				// Exit flag sequence
				f = 0;
				if (*dp != 0)
					// Err: unexpected flag is detected (may be collapted data)
					return 0 - (JDR_INT)JDR_FMT1;
				
				// The flag is a data 0xFF
				*dp = s = 0xFF;	
			}
			else
			{
				// Get next data byte
				s = *dp;
				// Is start of flag sequence?
				if (s == 0xFF)
				{
					f = 1;
					// Enter flag sequence, get trailing byte
					continue;
				}
			}
			// Read from MSB
			msk = 0x80;
		}
		// Get a bit
		v <<= 1;
		if(s & msk)
			v++;
		msk >>= 1;

		// Search the code word in this bit length
		for (nd = *hbits++; nd; nd--)
		{
			// Matched?
			if (v == *hcode++)
			{
				_dmsk = msk;
				_dctr = dc;
				_dptr = dp;
				// Return the decoded data
				return *hdata;
			}
			hdata++;
		}
		bl--;
	}
	while (bl);

	// Err: code not found (may be collapted data)
	return 0 - (JDR_INT)JDR_FMT1;
}

// Apply Inverse-DCT in Arai Algorithm (see also aa_idct.png)
void FishinoJPEGDecoder::blockIdct(
	JDR_LONG* src,	// Input block data (de-quantized and pre-scaled for Arai Algorithm)
	JDR_BYTE* dst	// Pointer to the destination to store the block as byte array
)
{
	const JDR_LONG M13 = (JDR_LONG)(1.41421 * 4096), M2 = (JDR_LONG)(1.08239 * 4096), M4 = (JDR_LONG)(2.61313 * 4096), M5 = (JDR_LONG)(1.84776 * 4096);
	JDR_LONG v0, v1, v2, v3, v4, v5, v6, v7;
	JDR_LONG t10, t11, t12, t13;
	JDR_UINT i;

	// Process columns
	for (i = 0; i < 8; i++)
	{
		// Get even elements
		v0 = src[8 * 0];
		v1 = src[8 * 2];
		v2 = src[8 * 4];
		v3 = src[8 * 6];

		// Process the even elements
		t10 = v0 + v2;
		t12 = v0 - v2;
		t11 = (v1 - v3) * M13 >> 12;
		v3 += v1;
		t11 -= v3;
		v0 = t10 + v3;
		v3 = t10 - v3;
		v1 = t11 + t12;
		v2 = t12 - t11;

		// Get odd elements
		v4 = src[8 * 7];
		v5 = src[8 * 1];
		v6 = src[8 * 5];
		v7 = src[8 * 3];

		// Process the odd elements
		t10 = v5 - v4;
		t11 = v5 + v4;
		t12 = v6 - v7;
		v7 += v6;
		v5 = (t11 - v7) * M13 >> 12;
		v7 += t11;
		t13 = (t10 + t12) * M5 >> 12;
		v4 = t13 - (t10 * M2 >> 12);
		v6 = t13 - (t12 * M4 >> 12) - v7;
		v5 -= v6;
		v4 -= v5;

		// Write-back transformed values
		src[8 * 0] = v0 + v7;
		src[8 * 7] = v0 - v7;
		src[8 * 1] = v1 + v6;
		src[8 * 6] = v1 - v6;
		src[8 * 2] = v2 + v5;
		src[8 * 5] = v2 - v5;
		src[8 * 3] = v3 + v4;
		src[8 * 4] = v3 - v4;

		// Next column
		src++;
	}

	// Process rows
	src -= 8;
	for (i = 0; i < 8; i++)
	{
		// Get even elements (remove DC offset (-128) here)
		v0 = src[0] + (128L << 8);
		v1 = src[2];
		v2 = src[4];
		v3 = src[6];

		// Process the even elements
		t10 = v0 + v2;
		t12 = v0 - v2;
		t11 = (v1 - v3) * M13 >> 12;
		v3 += v1;
		t11 -= v3;
		v0 = t10 + v3;
		v3 = t10 - v3;
		v1 = t11 + t12;
		v2 = t12 - t11;

		// Get odd elements
		v4 = src[7];
		v5 = src[1];
		v6 = src[5];
		v7 = src[3];

		// Process the odd elements
		t10 = v5 - v4;
		t11 = v5 + v4;
		t12 = v6 - v7;
		v7 += v6;
		v5 = (t11 - v7) * M13 >> 12;
		v7 += t11;
		t13 = (t10 + t12) * M5 >> 12;
		v4 = t13 - (t10 * M2 >> 12);
		v6 = t13 - (t12 * M4 >> 12) - v7;
		v5 -= v6;
		v4 -= v5;

		// Descale the transformed values 8 bits and output
		dst[0] = BYTECLIP((v0 + v7) >> 8);
		dst[7] = BYTECLIP((v0 - v7) >> 8);
		dst[1] = BYTECLIP((v1 + v6) >> 8);
		dst[6] = BYTECLIP((v1 - v6) >> 8);
		dst[2] = BYTECLIP((v2 + v5) >> 8);
		dst[5] = BYTECLIP((v2 - v5) >> 8);
		dst[3] = BYTECLIP((v3 + v4) >> 8);
		dst[4] = BYTECLIP((v3 - v4) >> 8);
		dst += 8;

		// Next row
		src += 8;
	}
}

// Load all blocks in the MCU into working buffer
JDR_RESULT FishinoJPEGDecoder::mcuLoad(void)
{
	// Block working buffer for de-quantize and IDCT
	JDR_LONG *tmp = (JDR_LONG*)_workbuf;
	JDR_UINT blk, nby, nbc, i, z, id, cmp;
	JDR_INT b, d, e;
	JDR_BYTE *bp;
	const JDR_BYTE *hb, *hd;
	const JDR_WORD *hc;
	const JDR_LONG *dqf;

	// Number of Y blocks (1, 2 or 4)
	nby = _msx * _msy;
	// Number of C blocks (2)
	nbc = 2;
	// Pointer to the first block
	bp = _mcubuf;

	for (blk = 0; blk < nby + nbc; blk++)
	{
		// Component number 0:Y, 1:Cb, 2:Cr
		cmp = (blk < nby) ? 0 : blk - nby + 1;
		// Huffman table ID of the component
		id = cmp ? 1 : 0;

		// Extract a DC element from input stream
		
		// Huffman table for the DC element
		hb = _huffbits[id][0];
		hc = _huffcode[id][0];
		hd = _huffdata[id][0];
		
		// Extract a huffman coded data (bit length)
		b = huffext(hb, hc, hd);
		if (b < 0)
			// Err: invalid code or input
			return (JDR_RESULT)(0 - b);
		
		// DC value of previous block
		d = _dcv[cmp];
		
		// If there is any difference from previous block
		if(b)
		{
			// Extract data bits
			e = bitext(b);
			if (e < 0)
				// Err: input
				return (JDR_RESULT)(0 - e);
			
			// MSB position
			b = 1 << (b - 1);
			
			// Restore sign if needed
			if(!(e & b))
				e -= (b << 1) - 1;
			
			// Get current value
			d += e;
			
			// Save current DC value for next block
			_dcv[cmp] = (JDR_SHORT)d;
		}
		// De-quantizer table ID for this component
		dqf = _qttbl[_qtid[cmp]];
		
		// De-quantize, apply scale factor of Arai algorithm and descale 8 bits
		tmp[0] = d * dqf[0] >> 8;

		// Extract following 63 AC elements from input stream
		
		// Clear rest of elements
		for (i = 1; i < 64; i++)
			tmp[i] = 0;
		
		// Huffman table for the AC elements
		hb = _huffbits[id][1];
		hc = _huffcode[id][1];
		hd = _huffdata[id][1];
		
		// Top of the AC elements
		i = 1;
		do
		{
			// Extract a huffman coded value (zero runs and bit length)
			b = huffext(hb, hc, hd);
			
			// EOB?
			if (b == 0)
				break;
			
			if (b < 0)
				// Err: invalid code or input error
				return (JDR_RESULT)(0 - b);
			
			// Number of leading zero elements
			z = (JDR_UINT)b >> 4;
			if (z)
			{
				// Skip zero elements
				i += z;
				if (i >= 64)
					// Too long zero run
					return JDR_FMT1;
			}
			
			// Bit length
			if (b &= 0x0F)
			{
				// Extract data bits
				d = bitext(b);
				if (d < 0)
					// Err: input device
					return (JDR_RESULT)(0 - d);
				
				// MSB position
				b = 1 << (b - 1);
				
				// Restore negative value if needed
				if (!(d & b))
					d -= (b << 1) - 1;
				
				// Zigzag-order to raster-order converted index
				z = ZIG(i);
				
				// De-quantize, apply scale factor of Arai algorithm and descale 8 bits
				tmp[z] = d * dqf[z] >> 8;
			}
		}
		// Next AC element
		while (++i < 64);

		if (_roughScale == 3)
			// If scale ratio is 1/8, IDCT can be ommited and only DC element is used
			*bp = (*tmp / 256) + 128;
		else
			// Apply IDCT and store the block to the MCU buffer
			blockIdct(tmp, bp);

		// Next block
		bp += 64;
	}

	// All blocks have been loaded successfully
	return JDR_OK;
}

// convert color format to destination one into working buffer
JDR_RESULT FishinoJPEGDecoder::colorConvert(JDR_BYTE *buf, JDR_UINT nPixels)
{
	switch(_deviceColorDepth)
	{
		case JDR_MONO :
			// still not supported
			return JDR_PAR;

		case JDR_GRAYSCALE :
			// still not supported
			return JDR_PAR;

		case JDR_COLOR323 :
			// still not supported
			return JDR_PAR;

		case JDR_COLOR565 :
			{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
				JDR_WORD w, *d = (JDR_WORD*)buf;
#pragma GCC diagnostic pop
		
				while(nPixels--)
				{
					w = (*buf++ & 0xF8) << 8;		// RRRRR-----------
					w |= (*buf++ & 0xFC) << 3;		// -----GGGGGG-----
					w |= *buf++ >> 3;				// -----------BBBBB
					*d++ = w;
				}
			}
			break;

		case JDR_COLOR888 :
			// do nothing, already in this format
			break;
			
		default:
			// bad color depth
			return JDR_PAR;
	}
	return JDR_OK;
}

// rotate mcu
void FishinoJPEGDecoder::mcuRotate90(JDR_RECT &rec)
{
	JDR_BYTE *dest = _rotBuf;
	for(JDR_CHAR y = 0; y < rec.width; y++)
		for(JDR_CHAR x = 0; x < rec.height; x++)
		{
			JDR_BYTE *src = (JDR_BYTE *)_workbuf + (rec.width * (x + 1)  - y - 1) * 3;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
		}

	swap(rec.width, rec.height);
	swap(rec.left, rec.top);
	rec.top = _deviceHeight - rec.top - rec.height;
}

void FishinoJPEGDecoder::mcuRotate180(JDR_RECT &rec)
{
	JDR_BYTE *dest = _rotBuf;
	for(JDR_CHAR y = 0; y < rec.height; y++)
		for(JDR_CHAR x = 0; x < rec.width; x++)
		{
			JDR_BYTE *src = (JDR_BYTE *)_workbuf + (rec.width * rec.height - rec.width * y - x - 1) * 3;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
		}

	rec.left = _deviceWidth - rec.left - rec.width;
	rec.top = _deviceHeight - rec.top - rec.height;
}

void FishinoJPEGDecoder::mcuRotate270(JDR_RECT &rec)
{
	JDR_BYTE *dest = _rotBuf;
	for(JDR_CHAR y = 0; y < rec.width; y++)
		for(JDR_CHAR x = 0; x < rec.height; x++)
		{
			JDR_BYTE *src = (JDR_BYTE *)_workbuf + (rec.width * (rec.height - x - 1) + y) * 3;
			*dest++ = *src++;
			*dest++ = *src++;
			*dest++ = *src++;
		}

	swap(rec.width, rec.height);
	swap(rec.left, rec.top);
	rec.left = _deviceWidth - rec.left - rec.width;
}

// Output an MCU: Convert YCrCb to RGB and output it in RGB form
JDR_RESULT FishinoJPEGDecoder::mcuOutput()
{
	const JDR_INT CVACC = (sizeof(JDR_INT) > 2) ? 1024 : 128;
	JDR_CHAR ix, iy;
	JDR_CHAR mx, my;
	JDR_INT yy, cb, cr;
	JDR_RECT rect;

	// MCU size (pixel)
	mx = _msx * 8;
	my = _msy * 8;
	
	// try to skip non displayed blocks early
	if(
		_xDest >= _destWidth	||
		_yDest >= _destHeight	||
		_xDest + mx <= 0	||
		_yDest + my <= 0
	)
	{
		_xDest += mx;
		_yDest += my;
		return JDR_OK;
	}

	// base pointers for tables
	JDR_BYTE *pc = _mcubuf + mx * my;
	JDR_BYTE *pcb, *pcr;
	JDR_BYTE *rgb24 = (JDR_BYTE*)_workbuf;
	
	// create 4 block pointers to speed up calculations
	JDR_BYTE *pyArr[4];
	for(JDR_CHAR i = 0; i < 4; i++)
		pyArr[i] = _mcubuf;
	if(_msx == 1)
	{
		if(_msy == 2)
		{
			pyArr[2] += 64;
			pyArr[3] = pyArr[2]; 
		}
	}
	else // _msx == 2
	{
		pyArr[1] += 64;
		if(_msy == 1)
			pyArr[3] = pyArr[1];
		else
		{
			pyArr[2] += 128;
			pyArr[3] += 192;
		}
	}

	// Build an RGB MCU from discrete components
	for (iy = 0; iy < my; iy++)
	{
		for (ix = 0; ix < mx; ix++)
		{
			JDR_BYTE arrIdx = ((iy & 8) ? 2 : 0) + ((ix & 8) ? 1 : 0);
			JDR_BYTE *&py = pyArr[arrIdx];
			if(_msy == 1)
			{
				if(_msx == 1)
					pcb = pc + my * iy + ix;
				else
					pcb = pc + my * iy / 2 + ix / 2;
			}
			else
			{
				if(_msx == 1)
					pcb = pc + mx * ix / 2 + iy;
				else
					pcb = pc + (my * iy + ix) / 4;
			}
			pcr = pcb + 64;

			// get luminance component
			yy = *py++;
			
			// Get Cb/Cr component and restore right level
			cb = *pcb - 128;
			cr = *pcr - 128;
			
			// Convert YCbCr to RGB
			*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.402 * CVACC) * cr) / CVACC);									// R
			*rgb24++ = BYTECLIP(yy - ((JDR_INT)(0.344 * CVACC) * cb + (JDR_INT)(0.714 * CVACC) * cr) / CVACC);	// G
			*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.772 * CVACC) * cb) / CVACC);									// B
		}
	}
	
	rect.left = _xDest;
	rect.top = _yDest;
	rect.width = mx;
	rect.height = my;
	
	// crop the MCU if partially outside destination image
	JDR_BYTE *sourceP = (JDR_BYTE *)_workbuf;
	JDR_BYTE *destP = sourceP;
	JDR_BYTE *packedP = sourceP;
	if(_yDest < 0)
	{
		rect.top = 0;
		rect.height += _yDest;
		sourceP += 3 * mx * (-_yDest);
		packedP = sourceP;
		destP = packedP;
	}
	else if(_yDest + my > _destHeight)
		rect.height = _destHeight - _yDest;
	
	int8_t leftCrop = 0;
	if(_xDest < 0)
	{
		rect.left = 0;
		leftCrop = -_xDest;
		rect.width -= leftCrop;
	}
	else if(_xDest + mx > _destWidth)
		rect.width = _destWidth - _xDest;
	if(leftCrop || rect.width != mx)
	{
		for(JDR_CHAR i = 0; i < rect.height; i++)
		{
			sourceP += 3 * leftCrop;
			for(JDR_CHAR j = 0; j < rect.width; j++)
			{
				*destP++ = *sourceP++;
				*destP++ = *sourceP++;
				*destP++ = *sourceP++;
			}
		}
	}
	
	// setup next MCU position
	_xDest += mx;
	_yDest += my;

	// rotate MCU if needed
	switch(_rotation)
	{
		case JDR_ROT_0:
		default:
			// Convert RGB888 requested output format
			if(colorConvert((JDR_BYTE *)_workbuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, (JDR_BYTE const *)_workbuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_90:
			// rotate mcu
			mcuRotate90(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_180:
			mcuRotate180(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_270:
			mcuRotate270(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;
	}
}

// Output an MCU: Convert YCrCb to RGB and output it in RGB form
// scaled version
JDR_RESULT FishinoJPEGDecoder::mcuOutputScaled()
{
	const JDR_INT CVACC = (sizeof(JDR_INT) > 2) ? 1024 : 128;
	JDR_CHAR ix, iy, mx, my;
	JDR_INT yy, cb, cr;
	JDR_RECT rect;

	// MCU size (pixel)
	mx = _msx * 8;
	my = _msy * 8;
	
	// start of rectangular area in the frame buffer
	rect.left = _xDest;
	rect.top = _yDest;
	
	// base pointers for tables
	JDR_BYTE *pc = _mcubuf + mx * my;
	JDR_BYTE *pcb, *pcr;
	JDR_BYTE *rgb24 = (JDR_BYTE*)_workbuf;
	
	// create 4 block pointers to speed up calculations
	JDR_BYTE *pyArr[4];
	for(JDR_CHAR i = 0; i < 4; i++)
		pyArr[i] = _mcubuf;
	if(_msx == 1)
	{
		if(_msy == 2)
		{
			pyArr[2] += 64;
			pyArr[3] = pyArr[2]; 
		}
	}
	else // _msx == 2
	{
		pyArr[1] += 64;
		if(_msy == 1)
			pyArr[3] = pyArr[1];
		else
		{
			pyArr[2] += 128;
			pyArr[3] += 192;
		}
	}

	// Not for 1/8 scaling
	if (_roughScale < 3)
	{
		// Build an RGB MCU from discrete components
		for (iy = 0; iy < my; iy++)
		{
			for (ix = 0; ix < mx; ix++)
			{
				JDR_BYTE arrIdx = ((iy & 8) ? 2 : 0) + ((ix & 8) ? 1 : 0);
				JDR_BYTE *&py = pyArr[arrIdx];
				if(_msy == 1)
				{
					if(_msx == 1)
						pcb = pc + my * iy + ix;
					else
						pcb = pc + my * iy / 2 + ix / 2;
				}
				else
				{
					if(_msx == 1)
						pcb = pc + mx * ix / 2 + iy;
					else
						pcb = pc + (my * iy + ix) / 4;
				}
				pcr = pcb + 64;

				// get luminance component
				yy = *py++;
				
				// Get Cb/Cr component and restore right level
				cb = *pcb - 128;
				cr = *pcr - 128;
				
				// Convert YCbCr to RGB
				*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.402 * CVACC) * cr) / CVACC);									// R
				*rgb24++ = BYTECLIP(yy - ((JDR_INT)(0.344 * CVACC) * cb + (JDR_INT)(0.714 * CVACC) * cr) / CVACC);	// G
				*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.772 * CVACC) * cb) / CVACC);									// B
			}
		}

		// Descale the MCU rectangular if needed
		if (_roughScale)
		{
			JDR_UINT x, y, r, g, b, s, w, a;
			JDR_BYTE *op;

			// Get averaged RGB value of each square correcponds to a pixel
			
			// Bumber of shifts for averaging
			s = _roughScale * 2;
			
			// Width of square
			w = 1 << _roughScale;
			
			// Bytes to skip for next line in the square
			a = (mx - w) * 3;
			
			op = (JDR_BYTE*)_workbuf;
			for (iy = 0; iy < my; iy += w)
			{
				for (ix = 0; ix < mx; ix += w)
				{
					rgb24 = (JDR_BYTE*)_workbuf + (iy * mx + ix) * 3;
					r = g = b = 0;
					
					// Accumulate RGB value in the square
					for (y = 0; y < w; y++)
					{
						for (x = 0; x < w; x++)
						{
							r += *rgb24++;
							g += *rgb24++;
							b += *rgb24++;
						}
						rgb24 += a;
					}
					// Put the averaged RGB value as a pixel
					*op++ = (JDR_BYTE)(r >> s);
					*op++ = (JDR_BYTE)(g >> s);
					*op++ = (JDR_BYTE)(b >> s);
				}
			}
		}
	}
	// For only 1/8 scaling (left-top pixel in each block are the DC value of the block)
	else
	{
		// Build a 1/8 descaled RGB MCU from discrete comopnents
		
		// Get Cb/Cr component and restore right level
		cb = pc[0] - 128;
		cr = pc[64] - 128;
		for (iy = 0; iy < my; iy += 8)
		{
			for (ix = 0; ix < mx; ix += 8)
			{
				JDR_BYTE arrIdx = ((iy & 8) ? 2 : 0) + ((ix & 8) ? 1 : 0);
				JDR_BYTE *&py = pyArr[arrIdx];

				// Get Y component
				yy = *py;

				// Convert YCbCr to RGB
				*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.402 * CVACC) * cr / CVACC));									// R
				*rgb24++ = BYTECLIP(yy - ((JDR_INT)(0.344 * CVACC) * cb + (JDR_INT)(0.714 * CVACC) * cr) / CVACC);	// G
				*rgb24++ = BYTECLIP(yy + ((JDR_INT)(1.772 * CVACC) * cb / CVACC));									// B
			}
		}
	}
	
	mx >>= _roughScale;
	my >>= _roughScale;

	// store left corner dest position and delta
	JDR_SHORT leftXDest = _xDest;
	JDR_SHORT leftXDelta = _xDelta;
	
	// dest and source pointers
	JDR_BYTE *dest = (JDR_BYTE *)_workbuf, *source = (JDR_BYTE *)_workbuf;
	
	rect.height = 0;
	for(JDR_CHAR iy = 0; iy < my; iy++)
	{
		// check for image limit (bresenham is not perfect...)
		if(_yDest >= _destHeight)
			break;
		
		// if we shall not output this line
		// just update delta and continue
		if(_yDelta < 0)
		{
			_yDelta += 2 * _destSz;
			source += 3 * mx;
			continue;
		}
		
		// increment vertical size
		rect.height++;

		// restore current xDest position
		_xDest = leftXDest;
		_xDelta = leftXDelta;
		
		// re-set width to 0 -- dumb but we repeat this for each line
		rect.width = 0;
		for(JDR_CHAR ix = 0; ix < mx; ix++)
		{
			// check for image limit (bresenham is not perfect...)
			if(_xDest >= _destWidth)
				break;

			// if we shall not output this pixel
			// just update delta and continue
			if(_xDelta < 0)
			{
				_xDelta += 2 * _destSz;
				source += 3;
				continue;
			}
			
			// copy the pixel and increment horizontal size
			rect.width++;
			*dest++ = *source++;
			*dest++ = *source++;
			*dest++ = *source++;

			// do second bresenham x part here
			_xDest++;
			_xDelta += 2 * (_destSz - _sourceSz);

		}
		// do second bresenham y part here
		_yDest++;
		_yDelta += 2 * (_destSz - _sourceSz);
	}
	
	// rotate MCU if needed
	switch(_rotation)
	{
		case JDR_ROT_0:
		default:
			// Convert RGB888 requested output format
			if(colorConvert((JDR_BYTE *)_workbuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, (JDR_BYTE const *)_workbuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_90:
			// rotate mcu
			mcuRotate90(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_180:
			mcuRotate180(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;

		case JDR_ROT_270:
			mcuRotate270(rect);

			// Convert RGB888 requested output format
			if(colorConvert(_rotBuf, rect.width * rect.height) != JDR_OK)
				return JDR_PAR;
	
			// Output the rectangular block
			return _consumerFunction(rect, _rotBuf) ? JDR_OK : JDR_INTR;
	}
}

// Process restart interval
JDR_RESULT FishinoJPEGDecoder::restart(
	JDR_WORD rstn	// Expected restart sequence number
)
{
	JDR_UINT i, dc;
	JDR_WORD d;
	JDR_BYTE *dp;

	// Discard padding bits and get two bytes from the input stream
	dp = _dptr;
	dc = _dctr;
	d = 0;
	for (i = 0; i < 2; i++)
	{
		// No input data is available, re-fill input buffer
		if (!dc)
		{
			dp = _inbuf;
			dc = _stream->read(dp, JD_SZBUF);
			if (!dc) return JDR_INP;
		}
		else
		{
			dp++;
		}
		dc--;
		d = (d << 8) | *dp;	// Get a byte
	}
	_dptr = dp;
	_dctr = dc;
	_dmsk = 0;

	// Check the marker
	if ((d & 0xFFD8) != 0xFFD0 || (d & 7) != (rstn & 7))
		// Err: expected RSTn marker is not detected (may be collapted data)
		return JDR_FMT1;

	// Reset DC offset
	_dcv[2] = _dcv[1] = _dcv[0] = 0;

	return JDR_OK;
}


// Analyze the JPEG image and Initialize decompressor object
#define	LDB_WORD(ptr)		(JDR_WORD)(((JDR_WORD)*((JDR_BYTE*)(ptr))<<8)|(JDR_WORD)*(JDR_BYTE*)((ptr)+1))
JDR_RESULT FishinoJPEGDecoder::jdPrepare(void)
{
	JDR_BYTE *seg, b;
	JDR_WORD marker;
	JDR_DWORD ofs;
	JDR_UINT n, i, j, len;
	JDR_RESULT rc;

	// No restart interval (default)
	_nrst = 0;

	// Nulls pointers
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			_huffbits[i][j] = 0;
			_huffcode[i][j] = 0;
			_huffdata[i][j] = 0;
		}
	}
	for (i = 0; i < 4; i++)
		_qttbl[i] = 0;

	// Allocate stream input buffer
	_inbuf = seg = (JDR_BYTE *)allocPool(JD_SZBUF);
	if (!seg)
	{
		DEBUG_ERROR("allocPool failed1\n");
		return JDR_MEM1;
	}

	// Check SOI marker
	if (_stream->read(seg, 2) != 2)
		return JDR_INP;
	
	if (LDB_WORD(seg) != 0xFFD8)
		// Err: SOI is not detected
		return JDR_FMT1;
	
	ofs = 2;
	for (;;)
	{
		// Get a JPEG marker
		if (_stream->read(seg, 4) != 4)
			return JDR_INP;
		
		// Marker
		marker = LDB_WORD(seg);
		
		// Length field
		len = LDB_WORD(seg + 2);
		if (len <= 2 || (marker >> 8) != 0xFF)
			return JDR_FMT1;
		
		// Content size excluding length field
		len -= 2;
		
		// Number of bytes loaded
		ofs += 4 + len;

		switch (marker & 0xFF)
		{
			// SOF0 (baseline JPEG)
			case 0xC0:
			
				// Load segment data
				if (len > JD_SZBUF) return JDR_MEM2;
				if (_stream->read(seg, len) != len)
					return JDR_INP;

				// Image width in unit of pixel
				_width = LDB_WORD(seg + 3);
				
				// Image height in unit of pixel
				_height = LDB_WORD(seg + 1);
				
				// Err: Supports only Y/Cb/Cr format
				if (seg[5] != 3) return JDR_FMT3;

				// Check three image components
				for (i = 0; i < 3; i++)
				{
					// Get sampling factor
					b = seg[7 + 3 * i];
					
					// Y component
					if(!i)
					{
						// Check sampling factor
						if (b != 0x11 && b != 0x22 && b != 0x21 && b != 0x12)
							// Err: Supports only 4:4:4, 4:2:0 or 4:2:2(H and V)
							return JDR_FMT3;
						
						// Size of MCU [blocks]
						_msx = b >> 4;
						_msy = b & 15;
					}
					// Cb/Cr component
					else
					{
						if (b != 0x11)
							// Err: Sampling factor of Cr/Cb must be 1
							return JDR_FMT3;
					}
					
					// Get dequantizer table ID for this component
					b = seg[8 + 3 * i];
					if (b > 3)
						// Err: Invalid ID
						return JDR_FMT3;
					_qtid[i] = b;
				}
				break;

			// DRI
			case 0xDD:
			
				// Load segment data
				if (len > JD_SZBUF)
					return JDR_MEM2;
				if (_stream->read(seg, len) != len)
					return JDR_INP;

				// Get restart interval (MCUs)
				_nrst = LDB_WORD(seg);
				break;

			// DHT
			case 0xC4:
			
				// Load segment data
				if (len > JD_SZBUF)
					return JDR_MEM2;
				if (_stream->read(seg, len) != len)
					return JDR_INP;

				// Create huffman tables
				rc = (JDR_RESULT)createHuffmanTable(seg, len);
				if (rc)
					return rc;
				break;

			// DQT
			case 0xDB:
				// Load segment data
				if (len > JD_SZBUF)
					return JDR_MEM2;
				if (_stream->read(seg, len) != len)
					return JDR_INP;

				// Create de-quantizer tables
				rc = (JDR_RESULT)createQtTable(seg, len);
				if (rc)
					return rc;
				break;

			// SOS
			case 0xDA:
			
				// Load segment data
				if (len > JD_SZBUF)
					return JDR_MEM2;
				if (_stream->read(seg, len) != len)
					return JDR_INP;

				if (!_width || !_height)
					// Err: Invalid image size
					return JDR_FMT1;

				if (seg[0] != 3)
					// Err: Supports only three color components format
					return JDR_FMT3;

				// Check if all tables corresponding to each components have been loaded
				for (i = 0; i < 3; i++)
				{
					// Get huffman table ID
					b = seg[2 + 2 * i];
					if (b != 0x00 && b != 0x11)
						// Err: Different table number for DC/AC element
						return JDR_FMT3;
					b = i ? 1 : 0;

					// Check huffman table for this component
					if (!_huffbits[b][0] || !_huffbits[b][1])
						// Err: Huffman table not loaded
						return JDR_FMT1;
					
					if (!_qttbl[_qtid[i]])
						// Err: Dequantizer table not loaded
						return JDR_FMT1;
				}

				// Allocate working buffer for MCU and RGB
				
				// Number of Y blocks in the MCU
				n = _msy * _msx;
				if (!n)
					// Err: SOF0 has not been loaded
					return JDR_FMT1;
				
				// Allocate buffer for IDCT and RGB output
				len = n * 64 * 2 + 64;
				
				// but at least 256 byte is required for IDCT
				if (len < 256)
					len = 256;
				// and it may occupy a part of following MCU working buffer for RGB output
				_workbuf = allocPool(len);
				if (!_workbuf)
				{
					DEBUG_ERROR("allocPool failed2\n");
					// Err: not enough memory
					return JDR_MEM1;
				}
				
				// Allocate MCU working buffer
				_mcubuf = (JDR_BYTE *)allocPool((n + 2) * 64);
				if (!_mcubuf)
				{
					DEBUG_ERROR("allocPool failed3\n");
					// Err: not enough memory
					return JDR_MEM1;
				}
				
				// allocate rotation buffer
				_rotBuf = (JDR_BYTE *)allocPool(JD_SZROTBUF);
				if (!_rotBuf)
				{
					DEBUG_ERROR("allocPool failed4\n");
					// Err: not enough memory
					return JDR_MEM1;
				}
				
				// Pre-load the JPEG data to extract it from the bit stream
				_dptr = seg;
				_dctr = 0;
				
				// Prepare to read bit stream
				_dmsk = 0;
				
				// Align read offset to JD_SZBUF
				if (ofs %= JD_SZBUF)
				{
					_dctr = _stream->read(seg + ofs, JD_SZBUF - (JDR_UINT)ofs);
					_dptr = seg + ofs - 1;
				}

				// Initialization succeeded. Ready to decompress the JPEG image.
				return JDR_OK;

			case 0xC1:	// SOF1
			case 0xC2:	// SOF2
			case 0xC3:	// SOF3
			case 0xC5:	// SOF5
			case 0xC6:	// SOF6
			case 0xC7:	// SOF7
			case 0xC9:	// SOF9
			case 0xCA:	// SOF10
			case 0xCB:	// SOF11
			case 0xCD:	// SOF13
			case 0xCE:	// SOF14
			case 0xCF:	// SOF15
			case 0xD9:	// EOI
				// Unsuppoeted JPEG standard (may be progressive JPEG)
				return JDR_FMT3;

			// Unknown segment (comment, exif or etc..)
			default:
				// Skip segment data
				// Null pointer specifies to skip bytes of stream
				if (!skipInput(len))
					return JDR_INP;
		}
	}
}

// Start to decompress the JPEG picture
// non-scaled version
JDR_RESULT FishinoJPEGDecoder::jdDecomp(void)
{
	JDR_SHORT x, y, mx, my;
	JDR_WORD rst, rsc;
	JDR_RESULT rc;

	// Size of the MCU (pixel)
	mx = _msx * 8;
	my = _msy * 8;

	// Initialize DC values
	_dcv[2] = _dcv[1] = _dcv[0] = 0;
	rst = rsc = 0;

	rc = JDR_OK;

	// Vertical loop of MCUs
	_yDest = _y0;
	JDR_SHORT yNext = _yDest;
	for (y = 0; y < _height; y += my)
	{
		_xDest = _x0;

		// Horizontal loop of MCUs
		for (x = 0; x < _width; x += mx)
		{
			yNext = _yDest;

			// Process restart interval if enabled
			if (_nrst && rst++ == _nrst)
			{
				rc = restart(rsc++);
				if (rc != JDR_OK)
					return rc;
				rst = 1;
			}

			// Load an MCU (decompress huffman coded stream and apply IDCT)
			rc = mcuLoad();
			if (rc != JDR_OK)
				return rc;
			
			// Output the MCU (color space conversion, scaling and output)
			rc = mcuOutput();
			if (rc != JDR_OK)
				return rc;

			swap(_yDest, yNext);
		}
		swap(_yDest, yNext);
	}
	return rc;
}

// Start to decompress the JPEG picture
// scaled version
JDR_RESULT FishinoJPEGDecoder::jdDecompScaled(void)
{
	JDR_SHORT x, y, mx, my;
	JDR_WORD rst, rsc;
	JDR_RESULT rc;

	// Size of the MCU (pixel)
	mx = _msx * 8;
	my = _msy * 8;

	// Initialize DC values
	_dcv[2] = _dcv[1] = _dcv[0] = 0;
	rst = rsc = 0;

	rc = JDR_OK;

	// Vertical loop of MCUs
	_yDest = _y0;
	_yDelta = 2 * _destSz - _sourceSz;
	JDR_SHORT yNext = _yDest;
	JDR_SHORT yNextDelta = _yDelta;
	for (y = 0; y < _height; y += my)
	{
		_xDest = _x0;
		_xDelta = 2 * _destSz - _sourceSz;

		// Horizontal loop of MCUs
		for (x = 0; x < _width; x += mx)
		{
			yNext = _yDest;
			yNextDelta = _yDelta;

			// Process restart interval if enabled
			if (_nrst && rst++ == _nrst)
			{
				rc = restart(rsc++);
				if (rc != JDR_OK)
					return rc;
				rst = 1;
			}

			// Load an MCU (decompress huffman coded stream and apply IDCT)
			rc = mcuLoad();
			if (rc != JDR_OK)
				return rc;
			
			// Output the MCU (color space conversion, scaling and output)
			rc = mcuOutputScaled();
			if (rc != JDR_OK)
				return rc;

			swap(_yDest, yNext);
			swap(_yDelta, yNextDelta);
		}
		swap(_yDest, yNext);
		swap(_yDelta, yNextDelta);
	}
	return rc;
}

// constructor
FishinoJPEGDecoder::FishinoJPEGDecoder()
{
	_pool = NULL;
	_poolPtr = NULL;
	_poolFreeSize = 0;
	
	_stream = NULL;
	_consumerFunction = NULL;
	
	_deviceWidth = _deviceHeight = 0;
	_deviceColorDepth = JDR_COLOR888;
	
	// defaults with auto shrink active
	_autoShrink = false;

	// do NOT rotate the image
	_deviceRotation = JDR_ROT_0;
	
	// automatically center the image
	_centering = JDR_CENTER_NONE;
	_deviceXOrg = _deviceYOrg = 0;
}

// destructor
FishinoJPEGDecoder::~FishinoJPEGDecoder()
{
	clear();
}

// clears internal data
FishinoJPEGDecoder &FishinoJPEGDecoder::clear(void)
{
	if(_pool)
		free(_pool);
	_pool = NULL;
	_poolPtr = NULL;
	_poolFreeSize = 0;

	return *this;
}

// set data provider stream
JDR_RESULT FishinoJPEGDecoder::setProvider(FishinoStream &stream)
{
	// set the provider stream
	_stream = &stream;
	
	// if stream supports it, rewind it
	_stream->seek(0, SEEK_SET);

	// clear all the data
	clear();
	
	_poolFreeSize = JDR_POOL_SIZE;
	_pool = malloc(_poolFreeSize);
	if(!_pool)
	{
		DEBUG_ERROR("malloc failed allocating %d bytes\n", _poolFreeSize);
		return JDR_MEM1;
	}
	_poolPtr = _pool;
	
	// prepare for decoding
	return jdPrepare();
}

// set consumer function and destination depths
FishinoJPEGDecoder &FishinoJPEGDecoder::setConsumer(JDR_CONSUMER const &c, uint16_t width, uint16_t height, JDR_COLORDEPTHS depth)
{
	_consumerFunction	= c;
	_deviceWidth		= width;
	_deviceHeight		= height;
	_deviceColorDepth	= depth;

	return *this;
}

// set rotation mode
FishinoJPEGDecoder &FishinoJPEGDecoder::setRotation(JDR_ROTATIONS rot)
{
	_deviceRotation = rot;
	return *this;
}

// set position
FishinoJPEGDecoder &FishinoJPEGDecoder::setPosition(JDR_INT x, JDR_INT y)
{
	_deviceXOrg = x;
	_deviceYOrg = y;
	return *this;
}

// set auto scale
FishinoJPEGDecoder &FishinoJPEGDecoder::setAutoScale(bool a)
{
	_autoShrink = a;
	return *this;
}

// set auto centering
FishinoJPEGDecoder &FishinoJPEGDecoder::setAutoCenter(JDR_CENTERINGS c)
{
	_centering = c;
	return *this;
}
		
// decode jpeg data and send decoded to consumer
JDR_RESULT FishinoJPEGDecoder::decode(void)
{
	// check if we've an input stream attached
	if(!_stream)
		return JDR_INP;
	
	// check if we've got a consumer function
	// and if destination sizes and depth are correct
	if(!_consumerFunction)
		return JDR_INP;

	if(_deviceWidth == 0 || _deviceHeight == 0)
		return JDR_INP;

	switch(_deviceColorDepth)
	{
		case JDR_MONO:
			// by now not supported
			return JDR_INP;

		case JDR_GRAYSCALE:
			// by now not supported
			return JDR_INP;

		case JDR_COLOR323:
			// by now not supported
			return JDR_INP;

		case JDR_COLOR565:
			// supported
			break;

		case JDR_COLOR888:
			// supported
			break;

		default:
			// error
			return JDR_INP;
	}
	
	// set origin
	_x0 = _deviceXOrg;
	_y0 = _deviceYOrg;
	
	if(_deviceRotation == JDR_ROT_AUTOCW || _deviceRotation == JDR_ROT_AUTOCCW)
	{
		if(
			(_width > _height && _deviceWidth < _deviceHeight) ||
			(_width < _height && _deviceWidth > _deviceHeight)
		)
		{
			if(_deviceRotation == JDR_ROT_AUTOCW)
				_rotation = JDR_ROT_90;
			else
				_rotation = JDR_ROT_270;
		}
		else
			_rotation = JDR_ROT_0;
	}
	else
		_rotation = _deviceRotation;
	
	// adjust for rotation
	switch(_rotation)
	{
		case JDR_ROT_0:
		default:
			_destWidth = _deviceWidth;
			_destHeight = _deviceHeight;
			break;
		
		case JDR_ROT_90:
			_destWidth = _deviceHeight;
			_destHeight = _deviceWidth;
			break;
		
		case JDR_ROT_180:
			_destWidth = _deviceWidth;
			_destHeight = _deviceHeight;
			break;
		
		case JDR_ROT_270:
			_destWidth = _deviceHeight;
			_destHeight = _deviceWidth;
			break;
		
	}
	
	// if autoresize is needed, calculate its parameters
	// do not upscale, just downscale
	if(_autoShrink && (_destWidth < _width || _destHeight < _height))
	{
		// bresenham coefficients
		if((JDR_LONG)_destWidth * (JDR_LONG)_height < (JDR_LONG)_width * (JDR_LONG)_destHeight)
		{
			_destSz = _destWidth;
			_sourceSz = _width;
			if(_centering & JDR_CENTER_VERT)
				_y0 = ((JDR_SHORT)_destHeight - ((JDR_LONG)_height * _destSz) / _sourceSz) / 2;
		}
		else
		{
			_destSz = _destHeight;
			_sourceSz = _height;
			if(_centering & JDR_CENTER_HORIZ)
				_x0 = ((JDR_SHORT)_destWidth - ((JDR_LONG)_width * _destSz) / _sourceSz) / 2;
		}
		
		// rought scale factor, from 0 to 3, meaning scale from 1:1 to 1:8
		// this scaling is done directly from jpeg engine
		// remaining scaling must be done by bresenham algo
		_roughScale = 0;
		while(_sourceSz >= 2 * _destSz && _roughScale < 3)
		{
			_sourceSz /= 2;
			_roughScale++;
		}
		
		// run the scaled decompressor
		return jdDecompScaled();
	}
	// otherwise set them to  defaults, depending on autocenter
	else
	{
		if(_centering & JDR_CENTER_HORIZ)
			_x0 = ((JDR_SHORT)_destWidth - (JDR_SHORT)_width) / 2;
		
		if(_centering & JDR_CENTER_VERT)
			_y0 = ((JDR_SHORT)_destHeight - (JDR_SHORT)_height) / 2;
		
		// run the non-scaled decompressor
		return jdDecomp();
	}
}
