//////////////////////////////////////////////////////////////////////////////////////
//						    FishinoJPEGDecoder.h									//
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
#ifndef FISHINOJPEGDECODER_H
#define FISHINOJPEGDECODER_H

#include <FishinoStream.h>

// some integer types

// These types must be 16-bit, 32-bit or larger integer
typedef int				JDR_INT;
typedef unsigned int	JDR_UINT;

// These types must be 8-bit integer
typedef signed char		JDR_CHAR;
typedef unsigned char	JDR_UCHAR;
typedef unsigned char	JDR_BYTE;

// These types must be 16-bit integer
typedef int16_t			JDR_SHORT;
typedef uint16_t		JDR_USHORT;
typedef uint16_t		JDR_WORD;
typedef uint16_t		JDR_WCHAR;

// These types must be 32-bit integer
typedef int32_t			JDR_LONG;
typedef uint32_t		JDR_ULONG;
typedef uint32_t		JDR_DWORD;

// output image color depths
enum JDR_COLORDEPTHS
{
	JDR_MONO = 0,	// 0: Mono image, 1 bit/pixel
	JDR_GRAYSCALE,	// 1: Gray scale image, 256 levels, 1 byte/pixel
	JDR_COLOR323,	// 2: 256 color image, 1 byte/pixel
	JDR_COLOR565,	// 3: 16 bit depth image, 2 bytes/pixel
	JDR_COLOR888	// 4: RGB image, 3 bytes/pixel
};

// image rotations
enum JDR_ROTATIONS
{
	JDR_ROT_0		= 0,
	JDR_ROT_90CW	= 1,
	JDR_ROT_90		= 1,
	JDR_ROT_180		= 2,
	JDR_ROT_270		= 3,
	JDR_ROT_90CCW	= 3,
	JDR_ROT_AUTOCW	= 4,
	JDR_ROT_AUTOCCW	= 5
};

// image centering
enum JDR_CENTERINGS
{
	JDR_CENTER_NONE		= 0,
	JDR_CENTER_HORIZ	= 1,
	JDR_CENTER_VERT		= 2,
	JDR_CENTER_BOTH		= 3
};

// decoder error codes
enum JDR_RESULT
{
	JDR_OK = 0,		// 0: Succeeded
	JDR_INTR,		// 1: Interrupted by output function
	JDR_INP,		// 2: Device error or wrong termination of input stream
	JDR_MEM1,		// 3: Insufficient memory pool for the image
	JDR_MEM2,		// 4: Insufficient stream input buffer
	JDR_PAR,		// 5: Parameter error
	JDR_FMT1,		// 6: Data format error (may be damaged data)
	JDR_FMT2,		// 7: Right format but not supported
	JDR_FMT3		// 8: Not supported JPEG standard
};
		
struct JDR_RECT
{
	JDR_SHORT left, top;
	JDR_SHORT width, height;
};
		
// type for consumer function -- gets a rectangular array of pixels
typedef bool (*JDR_CONSUMER)(JDR_RECT const &, void const *);
		
class FishinoJPEGDecoder
{
	public:
	
	private:
	
		JDR_UINT _dctr;								// Number of bytes available in the input buffer
		JDR_BYTE* _dptr;							// Current data read ptr
		JDR_BYTE* _inbuf;							// Bit stream input buffer
		JDR_BYTE _dmsk;								// Current bit in the current read byte
		JDR_BYTE _roughScale;						// Output scaling ratio
		JDR_BYTE _msx, _msy;						// MCU size in unit of block (width, height)
		JDR_BYTE _qtid[3];							// Quantization table ID of each component
		JDR_SHORT _dcv[3];							// Previous DC element of each component
		JDR_WORD _nrst;								// Restart inverval
		JDR_SHORT _width, _height;					// Size of the input image (pixel)
		JDR_BYTE* _huffbits[2][2];					// Huffman bit distribution tables [id][dcac]
		JDR_WORD* _huffcode[2][2];					// Huffman code word tables [id][dcac]
		JDR_BYTE* _huffdata[2][2];					// Huffman decoded data tables [id][dcac]
		JDR_LONG* _qttbl[4];						// Dequaitizer tables [id]
		void* _workbuf;								// Working buffer for IDCT and RGB output
		JDR_BYTE* _mcubuf;							// Working buffer for the MCU
		void *_pool;								// The pre-allocated memory pool - size is inside a #define
		void *_poolPtr;								// Pointer to available memory pool
		JDR_UINT _poolFreeSize;						// Size of momory pool (bytes available)
		
		JDR_BYTE* _rotBuf;							// mcu rotation buffer - can't do in-place :-(
		
		bool _autoShrink;							// set if you want that image is shrinked to fit given area
		JDR_ROTATIONS _rotation;					// set if you want that image is rotated to best fit area
		JDR_CENTERINGS _centering;					// set if you want the image centered on given area
		
		JDR_SHORT _x0, _y0;							// destination origin (can be negative)
		JDR_SHORT _xDelta, _yDelta;					// bresenham running differences
		JDR_SHORT _destSz, _sourceSz;				// bresenham interpolation coefficients (depending of dest/source sizes)
		JDR_SHORT _xDest, _yDest;					// running destination coordinates
		
		// destination size
		JDR_SHORT _destWidth, _destHeight;
		
		// the stream
		FishinoStream *_stream;
		
		// line consumer function
		JDR_CONSUMER _consumerFunction;
		
		// origin on device
		JDR_SHORT _deviceXOrg, _deviceYOrg;
		
		// destination device size (without rotation)
		JDR_WORD _deviceWidth, _deviceHeight;
		
		// rotation for image onto device
		JDR_ROTATIONS _deviceRotation;
		
		// destination color depth
		JDR_COLORDEPTHS _deviceColorDepth;
	
		// Allocate a memory block from memory pool
		void* allocPool(JDR_UINT nd);
		
		// skip data from input stream
		bool skipInput(JDR_WORD len);

		// Create de-quantization and prescaling tables with a DQT segment
		// return 0 if OK, != 0 if Failed
		JDR_UINT createQtTable(
			const JDR_BYTE* data,	// Pointer to the quantizer tables
			JDR_UINT ndata			// Size of input data
		);

		// Create huffman code tables with a DHT segment
		// returns 0 if OK, != 0 if Failed
		JDR_UINT createHuffmanTable(
			const JDR_BYTE* data,	// Pointer to the packed huffman tables
			JDR_UINT ndata			// Size of input data
		);

		// Extract N bits from input stream
		// >=0: extracted data, <0: error code
		JDR_INT bitext(
			// Number of bits to extract (1 to 11)
			JDR_UINT nbit
		);

		// Extract a huffman decoded data from input stream
		// >=0: decoded data, <0: error code
		JDR_INT huffext(
			const JDR_BYTE* hbits,	// Pointer to the bit distribution table
			const JDR_WORD* hcode,	// Pointer to the code word table
			const JDR_BYTE* hdata	// Pointer to the data table
		);

		// Apply Inverse-DCT in Arai Algorithm (see also aa_idct.png)
		void blockIdct(
			JDR_LONG* src,	// Input block data (de-quantized and pre-scaled for Arai Algorithm)
			JDR_BYTE* dst	// Pointer to the destination to store the block as byte array
		);

		// Load all blocks in the MCU into working buffer
		JDR_RESULT mcuLoad(void);
		
		// rotate mcu
		void mcuRotate90(JDR_RECT &rec);
		void mcuRotate180(JDR_RECT &rec);
		void mcuRotate270(JDR_RECT &rec);
		
		// convert color format to destination one into working buffer
		JDR_RESULT colorConvert(JDR_BYTE *buf, JDR_UINT nPixels);

		// Output an MCU: Convert YCrCb to RGB and output it in RGB form
		// non-scaled version
		JDR_RESULT mcuOutput();
		
		// Output an MCU: Convert YCrCb to RGB and output it in RGB form
		// scaled version
		JDR_RESULT mcuOutputScaled();

		// Process restart interval
		JDR_RESULT restart(
			JDR_WORD rstn	// Expected restart sequence number
		);

		// Analyze the JPEG image and Initialize decompressor object
		JDR_RESULT jdPrepare(void);

		// Start to decompress the JPEG picture
		// non-scaled version
		JDR_RESULT jdDecomp(void);

		// Start to decompress the JPEG picture
		// scaled version
		JDR_RESULT jdDecompScaled(void);

	protected:
	
	public:
	
		// constructor
		FishinoJPEGDecoder();
		
		// destructor
		~FishinoJPEGDecoder();
		
		// clears internal data
		FishinoJPEGDecoder &clear(void);
		
		// set data provider stream
		JDR_RESULT setProvider(FishinoStream &stream);
		
		// set consumer function and destination depths
		FishinoJPEGDecoder &setConsumer(JDR_CONSUMER const &c, JDR_USHORT width, JDR_USHORT height, JDR_COLORDEPTHS depth);
		
		// set rotation mode
		FishinoJPEGDecoder &setRotation(JDR_ROTATIONS rot);
		
		// set position
		FishinoJPEGDecoder &setPosition(JDR_INT x, JDR_INT y);
		
		// set auto scale
		FishinoJPEGDecoder &setAutoScale(bool a);
		
		// set auto centering
		FishinoJPEGDecoder &setAutoCenter(JDR_CENTERINGS c); 
		
		// decode jpeg data and send decoded to consumer
		JDR_RESULT decode(void);
};

#endif
