//////////////////////////////////////////////////////////////////////////////////////
//																					//
//							FishinoRA8875.h											//
//		Hardware driver for TFT displays with RA8875 display controller				//
//							Generic abstract interface								//
//		Can't be used directly; see specialized classes for hardware interfaces		//
//					Created by Massimo Del Fedele, 2018								//
//																					//
//  Copyright (c) 2018 Massimo Del Fedele.  All rights reserved				.		//
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
//	Version 7.5.0 - 2018/02/02 - INITIAL VERSION									//
//																					//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef _FISHINO_RA8875_H_
#define _FISHINO_RA8875_H_

#include <FishinoGFX.h>

#include <FishinoFlash.h>

#ifdef __AVR__
#include <math.h>
#endif

/* ---------------------------- USER SETTINGS ---------------------*/

// display size -- must be set here
#define FISHINO_RA8875SPI_SIZE	RA8875_800x480

/* EXTERNAL TOUCH CONTROLLER ++++++++++++++++++++++++++++++++++++++++++
Some TFT come with capacitive touch screen or you may decide to use a better
controller for that, decomment the following line to save resources */
#define USE_EXTERNALTOUCH

/* INTERNAL KEY MATRIX ++++++++++++++++++++++++++++++++++++++++++
RA8875 has a 5x6 Key Matrix controller onboard, if you are not plan to use it
better leave commented the following define since it will share some registers
with several functions, otherwise de-comment it! */
//#define USE_RA8875_KEYMATRIX

/* DEFAULT CURSOR BLINK RATE ++++++++++++++++++++++++++++++++++++++++++++
Nothing special here, you can set the default blink rate */
#define DEFAULTCURSORBLINKRATE		10

/* DEFAULT INTERNAL FONT ENCODING ++++++++++++++++++++++++++++++++++++++++++++
RA8875 has 4 different font set, same shape but suitable for most languages
please look at RA8875 datasheet and choose the correct one for your language!
The default one it's the most common one and should work in most situations */
#define DEFAULTINTENCODING			ISO_IEC_8859_1//ISO_IEC_8859_2,ISO_IEC_8859_3,ISO_IEC_8859_4

#include "FishinoRA8875_Registers.h"

#if !defined(USE_EXTERNALTOUCH)
#include "FishinoRA8875Calibration.h"
#endif

enum RA8875sizes
{
	RA8875_320x240,
	RA8875_480x272,
	RA8875_800x480,
	Adafruit_480x272,
	Adafruit_800x480,
	RA8875_640x480
};

enum RA8875modes
{
	GRAPHIC,
	TEXT
};

enum RA8875tcursor
{
	NORMAL,
	BLINK
};

enum RA8875tsize{
	X16,
	X24,
	X32
};

enum RA8875fontSource
{
	INT,
	EXT
};

enum RA8875fontCoding
{
	ISO_IEC_8859_1,
	ISO_IEC_8859_2,
	ISO_IEC_8859_3,
	ISO_IEC_8859_4
};

enum RA8875extRomType
{
	GT21L16T1W,
	GT21H16T1W,
	GT23L16U2W,
	GT30H24T3Y,
	GT23L24T3Y,
	GT23L24M1Z,
	GT23L32S4W,
	GT30H32S4W,
	ER3303_1,
	ER3301_1
};

enum RA8875extRomCoding
{
	GB2312,
	GB12345,
	BIG5,
	UNICODE,
	ASCII,
	UNIJIS,
	JIS0208,
	LATIN
};

enum RA8875extRomFamily
{
	STANDARD,
	ARIAL,
	ROMAN,
	BOLD
};

enum RA8875boolean
{
	LAYER1,
	LAYER2,
	TRANSPARENT,
	LIGHTEN,
	OR,
	AND,
	FLOATING
};//for LTPR0

enum RA8875writes
{
	L1,
	L2,
	CGRAM,
	PATTERN,
	CURSOR
	
};//TESTING

// Touch screen cal structs
typedef struct Point
{
	int32_t x;
	int32_t y;
	
} tsPoint_t;

typedef struct
{
	int32_t An,Bn,Cn,Dn,En,Fn,Divider ;
	
} tsMatrix_t;

class FishinoRA8875Class : public FishinoGFX
{
	protected:
		
		//////////////////////////////////////////////////////////
		//					INTERFACE....INTERFACE!				//
		//////////////////////////////////////////////////////////

		// low level access  commands
		virtual void writeReg(uint8_t reg, uint8_t val) = 0;
		virtual uint8_t readReg(uint8_t reg) = 0;
		
		// mid level access commands
		virtual void writeCommand(uint8_t d)		= 0;
		virtual void writeData(uint8_t data)		= 0;
		virtual void writeData16(uint16_t data)		= 0;
		virtual uint8_t readData(void)				= 0;
		virtual uint8_t readStatus(void)			= 0;

		// write a block of data, RAM version
		// bufLen is the number of pixels (2 byte) data
		virtual void writeBuffer(uint16_t const *buf, size_t bufLen)	= 0;
		
		// write a block of data, FLASH version
		// buf MUST be a 16 bit image buffer in flash area
		// bufLen is the number of pixels (2 byte) data
		virtual void writeBuffer(const __FlashStringHelper *buf, size_t bufLen)	= 0;

		// read-transparent-write a block of data, RAM version
		virtual void writeBufferT(uint16_t const *buf, size_t bufLen, uint16_t transpColor)	= 0;
		
		// read-transparent-write a block of data, Flash version
		// buffer MUST be a 16 bit image chunk
		virtual void writeBufferT(const __FlashStringHelper *buf, size_t bufLen, uint16_t transpColor) = 0;

		// from adafruit
		virtual boolean waitPoll(uint8_t r, uint8_t f) = 0;
		
		// 0x80, 0x40(BTE busy), 0x01(DMA busy)
		virtual void waitBusy(uint8_t res=0x80) = 0;

		
		//////////////////////////////////////////////////////////
		//					HARDWARE ROUTINES					//
		//////////////////////////////////////////////////////////

		enum RA8875modes 		_currentMode;
		enum RA8875sizes 		_size;
		
		// swap x and y flag -- used on 90 and 270° rotations
		bool _swapxy;
		
		// Register containers -----------------------------------------
		uint8_t _MWCR0Reg; //keep track of the register 		  [0x40]
		uint8_t _DPCRReg;  ////Display Configuration		  	  [0x20]
		uint8_t _FNCR0Reg; //Font Control Register 0 		  	  [0x21]
		uint8_t _FNCR1Reg; //Font Control Register1 			  [0x22]
		uint8_t _FWTSETReg; //Font Write Type Setting Register 	  [0x2E]
		uint8_t _SFRSETReg; //Serial Font ROM Setting 		  	  [0x2F]
		uint8_t _TPCR0Reg; //Touch Panel Control Register 0	  	  [0x70]
		uint8_t _INTC1Reg; //Interrupt Control Register1		  [0xF0]

		void PWMsetup(uint8_t pw,bool on, uint8_t clock);
		
		// device initializations
		void initialize(uint8_t initIndex);
		
		// Change memory write direction
		// Parameters:
		// dir the direction, 0..3
		// 0 : L->R, then T->D
		// 1 : R->L, then T->D
		// 2 : T->D, then L->R
		// 3 : D->T, then L->R
		void setWriteDirection(uint8_t dir);

		// Change the beam scan direction on display
		// Parameters:
		// invertH:true(inverted),false(normal) horizontal
		// invertV:true(inverted),false(normal) vertical
		// used on screen rotation
		void setScanDirection(bool invertH,bool invertV);
		
		//////////////////////////////////////////////////////////
		// 				IMAGE RAM READ / WRITE SUPPORT			//
		//////////////////////////////////////////////////////////
		
		// copy an image buffer to image ram
		// supports 4 sizes image crop and transparency
		void writeImageBuf(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t skipLeft, uint16_t skipRight, uint16_t skipTop, uint16_t skipBottom, bool inv, bool transp, uint16_t transpColor, bool flashBuf, uint16_t const *buf);
		

		// copy an image buffer to image ram -- BTE version
		// supports 4 sides image crop and transparency
		// can be used only with display rotations 0 and 2 - fast
		void writeBTEImageBuf(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t skipLeft, uint16_t skipRight, uint16_t skipTop, uint16_t skipBottom, bool inv, bool transp, uint16_t transpColor, bool flashBuf, uint16_t const *buf);

		//////////////////////////////////////////////////////////
		//					BITBLT ROUTINES						//
		//////////////////////////////////////////////////////////

		// set BTE block source
		void BteSource(uint16_t sx,uint16_t sy);
		
		// set BTE block destination
		void BteDest(uint16_t dx,uint16_t dy);

		// set BTE block size
		void BteSize(uint16_t w, uint16_t h);
		
		// sets operation and rop code
		void BteOpRop(uint8_t op, uint8_t rop);
		
		// sets BTE background, foreground and transparent color
		void BteSetBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
		void BteSetBackgroundColor(uint16_t col)
			{ return BteSetBackgroundColor( (uint8_t)((col >> 11) & 0x1f), (uint8_t)((col >> 5) & 0x3f), (uint8_t)(col & 0x1f)); }
		void BteSetForegroundColor(uint8_t r, uint8_t g, uint8_t b);
		void BteSetForegroundColor(uint16_t col)
			{ return BteSetForegroundColor( (uint8_t)((col >> 11) & 0x1f), (uint8_t)((col >> 5) & 0x3f), (uint8_t)(col & 0x1f)); }
		void BteSetTransparentColor(uint8_t r, uint8_t g, uint8_t b)
			{ return BteSetForegroundColor(r, g, b); }
		void BteSetTransparentColor(uint16_t col)
			{ return BteSetForegroundColor(col); }
		
		// start BTE transfer
		void BteStart(void);
		
		// wait for BTE engine to complete
		void BteWait(void);
		
		virtual void bitBltHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf, bool flashBuf, bool inv, bool transp, uint16_t transpColor);

		virtual void bitBlt0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltHelper(x, y, w, h, buf, true, false, false, 0); }
		virtual void bitBltT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltHelper(x, y, w, h, buf, true, false, true, 0); }
		virtual void bitBltInv0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltHelper(x, y, w, h, buf, true, true, false, 0); }
		virtual void bitBltInvT0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf)
			{ bitBltHelper(x, y, w, h, buf, true, true, true, 0); }

		//////////////////////////////////////////////////////////
		//					LAYERS ROUTINES						//
		//////////////////////////////////////////////////////////

		uint8_t		_maxLayers;
		bool		_useMultiLayers;
		uint8_t		_currentLayer;
		
		//////////////////////////////////////////////////////////
		//					DRAWING ROUTINES					//
		//////////////////////////////////////////////////////////
		
		// graphic set location
		virtual void setCursor0(int16_t x, int16_t y);
		virtual void getCursor0(int16_t &x, int16_t &y);
		virtual void setCursorX0(int16_t x);
		virtual void setCursorY0(int16_t y) ;
		
		virtual void setReadCursor0(int16_t x, int16_t y);
		virtual void getReadCursor0(int16_t &x, int16_t &y);
		virtual void setReadCursorX0(int16_t x);
		virtual void setReadCursorY0(int16_t y) ;
		
		// swap 2 coordinates or widths
		static inline void swap(int16_t &x, int16_t &y) { int16_t tmp = x; x = y; y = tmp; }
		static inline void swap(uint16_t &x, uint16_t &y) { uint16_t tmp = x; x = y; y = tmp; }

		void checkLimitsHelper(int16_t &x,int16_t &y);
		void circleHelper(int16_t x0, int16_t y0, uint16_t r, uint16_t color, bool filled);
		void rectHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color, bool filled);
		void triangleHelper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);
		void ellipseHelper(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color, bool filled);
		void curveHelper(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled);
		void lineAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
		void curveAddressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
		void roundRectHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color, bool filled);

		void drawPixel0(int16_t x, int16_t y, uint16_t color);

		void drawFastVLine0(int16_t x, int16_t y, int16_t h, uint16_t color);
		void drawFastHLine0(int16_t x, int16_t y, int16_t w, uint16_t color);
		void drawLine0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

		void drawRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);
		void fillRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color);

		void drawCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color);
		void fillCircle0(int16_t x0, int16_t y0, uint16_t r, uint16_t color);

		void drawTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
		void fillTriangle0(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

		void drawEllipse0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color);
		void fillEllipse0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint16_t color);

		void drawCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color);
		void fillCurve0(int16_t xCenter, int16_t yCenter, uint16_t longAxis, uint16_t shortAxis, uint8_t curvePart, uint16_t color);

		void drawRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
		void fillRoundRect0(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);

		//////////////////////////////////////////////////////////
		//				FONT AND TEXT ROUTINES					//
		//////////////////////////////////////////////////////////
		
		uint8_t					_fontSpacing;
		bool					_fontFullAlig;
		bool					_fontRotation;
		bool					_extFontRom;
		uint8_t					_fontInterline;
		enum RA8875extRomFamily _fontFamily;
		enum RA8875extRomType 	_fontRomType;
		enum RA8875extRomCoding _fontRomCoding;
		enum RA8875tsize		_textSize;
		enum RA8875fontSource 	_fontSource;
		enum RA8875tcursor		_textCursorStyle;

		void textWrite(const char* buffer, uint16_t len=0);

		//////////////////////////////////////////////////////////
		//					BLOCK ROUTINES						//
		//////////////////////////////////////////////////////////
		
		int16_t		_scrollXL,_scrollXR,_scrollYT,_scrollYB;

		void DMA_blockModeSize(int16_t BWR,int16_t BHR,int16_t SPWR);
		void DMA_startAddress(unsigned long adrs);

		//////////////////////////////////////////////////////////
		//			INTERNAL TOUCHSCREEN AND KEYBOARD			//
		//////////////////////////////////////////////////////////
		
#if !defined(USE_EXTERNALTOUCH)
		uint8_t		_touchPin;
		bool		_clearTInt;
		uint16_t	_tsAdcMinX,_tsAdcMinY,_tsAdcMaxX,_tsAdcMaxY;
		bool		_touchEnabled;
#endif

#if defined(USE_RA8875_KEYMATRIX)
		bool		_keyMatrixEnabled;
#endif
		//----------------------------------------

#if !defined(USE_EXTERNALTOUCH)
		void readTouchADC(uint16_t *x, uint16_t *y);
		void clearTouchInt(void);
		bool touched(void);
#endif

		// constructor
		FishinoRA8875Class(RA8875sizes size);

	public:
		//////////////////////////////////////////////////////////
		//					INITIALIZATION						//
		//////////////////////////////////////////////////////////

		// initialization
		void begin(void);
		
		//////////////////////////////////////////////////////////
		//					HARDWARE ROUTINES					//
		//////////////////////////////////////////////////////////

		// software reset
		void softReset(void);

		// void    	softReset(void);
		void displayOn(bool on);
		void sleep(bool sleep);
		void brightness(uint8_t val);
		
		// GRAPHIC,TEXT
		void changeMode(enum RA8875modes m);
		void clearMemory(bool full);

		//////////////////////////////////////////////////////////
		//					LAYERS ROUTINES						//
		//////////////////////////////////////////////////////////

		bool useLayers(bool on);
		void writeTo(enum RA8875writes d);
		void layerEffect(enum RA8875boolean efx);
		void layerTransparency(uint8_t layer1,uint8_t layer2);
		
		//////////////////////////////////////////////////////////
		//					BLOCK ROUTINES						//
		//////////////////////////////////////////////////////////

		// block move
		void blockMove(uint16_t sx, uint16_t sy, uint8_t sl, uint16_t dx, uint16_t dy, uint8_t dl, uint16_t w, uint16_t h);
		
		// block vertical scroll area, current layer
		void blockVScroll(uint16_t sx, uint16_t sy, uint16_t w, uint16_t h, int16_t dy, uint8_t lay, uint16_t fillColor);

		void fillScreen(uint16_t color);

		void setActiveWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
		void setActiveWindow(void);
		void getActiveWindow(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2);
		
		void setRotation(uint8_t rotation);

		//////////////////////////////////////////////////////////
		//						COLOR ROUTINES					//
		//////////////////////////////////////////////////////////

		// set BPP
		void set8BPP(void);
		void set16BPP(void);
		
		void setForegroundColor(uint16_t color);
		void setForegroundColor(uint8_t R,uint8_t G,uint8_t B);
		void setBackgroundColor(uint16_t color);
		void setBackgroundColor(uint8_t R,uint8_t G,uint8_t B);
		void setTrasparentColor(uint16_t color);
		void setTrasparentColor(uint8_t R,uint8_t G,uint8_t B);
		
		//----------cursor stuff................
		// to calculate max column. (screenWidth/fontWidth)-1
		// to calculate max row.    (screenHeight/fontHeight)-1
		void showCursor(bool cur,enum RA8875tcursor c = BLINK);//show text cursor, select cursor typ (NORMAL,BLINK)
		void setCursorBlinkRate(uint8_t rate);//0...255 0:faster
		
		void setTextColor(uint16_t fColor, uint16_t bColor);
		
		// transparent background
		void setTextColor(uint16_t fColor);
		
		void uploadUserChar(const uint8_t symbol[],uint8_t address);
		
		// 0...255
		void showUserChar(uint8_t symbolAddrs,uint8_t wide=0);
		
		// 0..3
		void setFontScale(uint8_t scale);
		
		// X16,X24,X32
		void setFontSize(enum RA8875tsize ts, bool halfSize = false);
		
		// 0:disabled ... 63:pix max
		void setFontSpacing(uint8_t spc);
		
		// true = 90 degrees
		void setFontRotate(bool rot);
		
		// 0...63 pix
		void setFontInterline(uint8_t pix);
		
		// mmmm... doesn't do nothing! Have to investigate
		void setFontFullAlign(bool align);
		
		void setExternalFontRom(enum RA8875extRomType ert, enum RA8875extRomCoding erc,enum RA8875extRomFamily erf=STANDARD);

		// INT,EXT (if you have a chip installed)
		void setFont(enum RA8875fontSource s);
		void setIntFontCoding(enum RA8875fontCoding f);
		void setExtFontFamily(enum RA8875extRomFamily erf,bool setReg=true);
		
		void setFont(const GFXfont *f = NULL);
		void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
		void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

		// 0...7 Select a custom graphic cursor (you should upload first)
		void setGraphicCursor(uint8_t cur);
		void showGraphicCursor(bool cur);//show graphic cursor
		
		void setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
		void scroll(uint16_t x,uint16_t y);
		
//		void drawFlashImage(int16_t x,int16_t y,int16_t w,int16_t h,uint8_t picnum);
		void drawFlashImage0(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t address);
		
		void GPIOX(bool on);
		void PWMout(uint8_t pw,uint8_t p);//1:backlight, 2:free
		
#if !defined(USE_EXTERNALTOUCH)
		void touchBegin(uint8_t intPin);//prepare Touch Screen driver
		void touchEnable(bool enabled);//enable/disable Touch Polling (disable INT)
		bool touchDetect(bool autoclear=false);//true=touch detected
		//Note:		must followed by touchReadxxx or use autoclear=true for standalone
		void touchReadRaw(uint16_t *x, uint16_t *y);//returns 10bit ADC data (0...1024)
		void touchReadPixel(uint16_t *x, uint16_t *y);//return pixels (0...width, 0...height)
		bool touchCalibrated(void);//true if screen calibration it's present
#endif

		virtual size_t write(uint8_t b)
		{
			textWrite((const char *)&b, 1);
			return 1;
		}

		virtual size_t write(const uint8_t *buffer, size_t size)
		{
			textWrite((const char *)buffer, size);
			return size;
		}
		using Print::write;
};

#endif
