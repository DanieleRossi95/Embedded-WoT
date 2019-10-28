#include "FishinoRA8875SPI.h"

#include <FishinoFlash.h>

// Upload user custom cahr or symbol to CGRAM, max 255
// Parameters:
// symbol[]: an 8bit x 16 char in an array. Must be exact 16 bytes
// address: 0...255 the address of the CGRAM where to store the char
void FishinoRA8875Class::uploadUserChar(const uint8_t symbol[],uint8_t address)
{
	bool modeChanged = false;
	if (_currentMode != GRAPHIC)
	{
		//was in text!
		changeMode(GRAPHIC);
		modeChanged = true;
	}
	writeReg(RA8875_CGSR,address);
	writeTo(CGRAM);
	writeCommand(RA8875_MRWC);
	for (uint8_t i=0;i<16;i++)
	{
		writeData(symbol[i]);
	}
	if (modeChanged)
		changeMode(TEXT);
}

// Retrieve and print to screen the user custom char or symbol
// User have to store a custom char before use this function
// Parameters:
// address: 0...255 the address of the CGRAM where char it's stored
// wide:0 for single 8x16 char, if you have wider chars that use
// more than a char slot they can be showed combined (see examples)
void FishinoRA8875Class::showUserChar(uint8_t symbolAddrs,uint8_t wide)
{
	uint8_t oldRegState = _FNCR0Reg;
	uint8_t i;
	bitSet(oldRegState,7);//set to CGRAM
	writeReg(RA8875_FNCR0,oldRegState);
	//layers?
	if (_useMultiLayers)
	{
		if (_currentLayer == 0)
		{
			writeTo(L1);
		}
		else
		{
			writeTo(L2);
		}
	}
	else
	{
		writeTo(L1);
	}
	writeCommand(RA8875_MRWC);
	writeData(symbolAddrs);
	if (wide > 0)
	{
		for (i=1;i<=wide;i++)
		{
			writeData(symbolAddrs+i);
		}
	}
	if (oldRegState != _FNCR0Reg)
		writeReg(RA8875_FNCR0,_FNCR0Reg);
}

// Set internal Font Encoding
// Parameters:
// f:ISO_IEC_8859_1, ISO_IEC_8859_2, ISO_IEC_8859_3, ISO_IEC_8859_4
// default:ISO_IEC_8859_1
void FishinoRA8875Class::setIntFontCoding(enum RA8875fontCoding f)
{
	uint8_t temp = _FNCR0Reg;
	temp &= ~((1<<1) | (1<<0));// Clear bits 1 and 0
	switch (f)
	{
		case ISO_IEC_8859_1:
			//do nothing
			break;
		case ISO_IEC_8859_2:
			temp |= (1 << 0);
			break;
		case ISO_IEC_8859_3:
			temp |= (1 << 1);
			break;
		case ISO_IEC_8859_4:
			temp |= ((1<<1) | (1<<0));// Set bits 1 and 0
			break;
		default:
			return;
	}
	_FNCR0Reg = temp;
	writeReg(RA8875_FNCR0,_FNCR0Reg);
}

// External Font Rom setup
// This will not phisically change the register but should be called before setFont(EXT)!
// You should use this values accordly Font ROM datasheet!
// Parameters:
// ert:ROM Type          (GT21L16T1W, GT21H16T1W, GT23L16U2W, GT30H24T3Y, GT23L24T3Y, GT23L24M1Z, GT23L32S4W, GT30H32S4W)
// erc:ROM Font Encoding (GB2312, GB12345, BIG5, UNICODE, ASCII, UNIJIS, JIS0208, LATIN)
// erf:ROM Font Family   (STANDARD, ARIAL, ROMAN, BOLD)
void FishinoRA8875Class::setExternalFontRom(enum RA8875extRomType ert, enum RA8875extRomCoding erc, enum RA8875extRomFamily erf)
{
	uint8_t temp = _SFRSETReg;//just to preserve the reg in case something wrong
	switch (ert)
	{
			//type of rom
		case GT21L16T1W:
		case GT21H16T1W:
			temp &= 0x1F;
			break;
		case GT23L16U2W:
			temp &= 0x1F;
			temp |= 0x20;
			break;
		case GT23L24T3Y:
		case GT30H24T3Y:
		case ER3303_1://encoding GB12345
			temp &= 0x1F;
			temp |= 0x40;
			//erc = GB12345;//forced
			break;
		case GT23L24M1Z:
			temp &= 0x1F;
			temp |= 0x60;
			break;
		case GT23L32S4W:
		case GT30H32S4W:
			temp &= 0x1F;
			temp |= 0x80;
			break;
			
		// non funziona.... 
		case ER3301_1:
			temp &= 0x1F;
			temp |= 0x00;
			
			break;
		default:
			_extFontRom = false;//wrong type, better avoid for future
			return;//cannot continue, exit
	}
	_fontRomType = ert;
	switch (erc)
	{
			//check rom font coding
		case GB2312:
			temp &= 0xE3;
			break;
		case GB12345:
			temp &= 0xE3;
			temp |= 0x04;
			break;
		case BIG5:
			temp &= 0xE3;
			temp |= 0x08;
			break;
		case UNICODE:
			temp &= 0xE3;
			temp |= 0x0C;
			break;
		case ASCII:
			temp &= 0xE3;
			temp |= 0x10;
			break;
		case UNIJIS:
			temp &= 0xE3;
			temp |= 0x14;
			break;
		case JIS0208:
			temp &= 0xE3;
			temp |= 0x18;
			break;
		case LATIN:
			temp &= 0xE3;
			temp |= 0x1C;
			break;
		default:
			_extFontRom = false;//wrong coding, better avoid for future
			return;//cannot continue, exit
	}
	_fontRomCoding = erc;
	_SFRSETReg = temp;
	setExtFontFamily(erf,false);
	_extFontRom = true;
	//writeReg(RA8875_SFRSET,_SFRSETReg);//0x2F
	//delay(4);
}

// select the font family for the external Font Rom Chip
// Parameters:
// erf: STANDARD, ARIAL, ROMAN, BOLD
// setReg:
// true(send phisically the register, useful when you change
// family after set setExternalFontRom)
// false:(change only the register container, useful during config)
void FishinoRA8875Class::setExtFontFamily(enum RA8875extRomFamily erf, bool setReg)
{
	_fontFamily = erf;
	switch (erf)
	{
			//check rom font family
		case STANDARD:
			_SFRSETReg &= 0xFC;
			break;
		case ARIAL:
			_SFRSETReg &= 0xFC;
			_SFRSETReg |= 0x01;
			break;
		case ROMAN:
			_SFRSETReg &= 0xFC;
			_SFRSETReg |= 0x02;
			break;
		case BOLD:
			_SFRSETReg |= ((1<<1) | (1<<0)); // set bits 1 and 0
			break;
		default:
			_fontFamily = STANDARD;
			_SFRSETReg &= 0xFC;
			return;
	}
	if (setReg)
		writeReg(RA8875_SFRSET,_SFRSETReg);
}

// choose from internal/external (if exist) Font Rom
// Parameters:
// s: Font source (INT,EXT)
void FishinoRA8875Class::setFont(enum RA8875fontSource s)
{
	//enum RA8875fontCoding c
	if (s == INT)
	{
		//check the font coding
		if (_extFontRom)
		{
			setFontSize(X16,false);
			writeReg(RA8875_SFRSET,0b00000000);//_SFRSETReg
		}
		_FNCR0Reg &= ~((1<<7) | (1<<5));// Clear bits 7 and 5
		writeReg(RA8875_FNCR0,_FNCR0Reg);
		_fontSource = s;
		delay(1);
	}
	else
	{
		if (_extFontRom)
		{
			_fontSource = s;
			//now switch
			_FNCR0Reg |= (1 << 5);
			writeReg(RA8875_FNCR0,_FNCR0Reg);//0x21
			delay(1);
			writeReg(RA8875_SFCLR,0x02);//Serial Flash/ROM CLK frequency/2
			setFontSize(X24,false);////X24 size
			writeReg(RA8875_SFRSET,_SFRSETReg);//at this point should be already set
			delay(4);
			writeReg(RA8875_SROC,0x28);// 0x28 rom 0,24bit adrs,wave 3,1 byte dummy,font mode, single mode
			delay(4);
		}
		else
		{
			setFont(INT);
		}
	}
}

// Enable/Disable the Font Full Alignemet feature (default off)
// Parameters:
// align: true,false
void FishinoRA8875Class::setFontFullAlign(bool align)
{
	align == true ? _FNCR1Reg |= (1 << 7) : _FNCR1Reg &= ~(1 << 7);
	writeReg(RA8875_FNCR1,_FNCR1Reg);
}

// Enable/Disable 90" Font Rotation (default off)
// Parameters:
// rot: true,false
void FishinoRA8875Class::setFontRotate(bool rot)
{
	rot == true ? _FNCR1Reg |= (1 << 4) : _FNCR1Reg &= ~(1 << 4);
	writeReg(RA8875_FNCR1,_FNCR1Reg);
}

// Set distance between text lines (default off)
// Parameters:
// pix: 0...63 pixels
void FishinoRA8875Class::setFontInterline(uint8_t pix)
{
	if (pix > 0x3F)
		pix = 0x3F;
	_fontInterline = pix;
	//_FWTSETReg &= 0xC0;
	//_FWTSETReg |= spc & 0x3F;
	writeReg(RA8875_FLDR,_fontInterline);
}

// Set GFX font
void FishinoRA8875Class::setFont(const GFXfont *f)
{
	FishinoGFX::setFont(f);
}

// Get text bounding box
void FishinoRA8875Class::getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	// if we're using gfx fonts just call GFX class member
	if(_gfxFont)
	{
		FishinoGFX::getTextBounds(string, x, y, x1, y1, w, h);
		return;
	}

	// @@ just handle internal font by now
	// for ROM fonts we'll look later
	size_t len = strlen(string);
	*x1 = x;
	*y1 = y + 16 * _fontScale;
	*h = 16 * _fontScale;
	*w = 8 * len * _fontScale;
}

// Get text bounding box
void FishinoRA8875Class::getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	// if we're using gfx fonts just call GFX class member
	if(_gfxFont)
	{
		FishinoGFX::getTextBounds(s, x, y, x1, y1, w, h);
		return;
	}

	// @@@ temp!!! Shall support font sizes and others!!!
	size_t len = strlen(s);
	*x1 = x;
	*y1 = y + 16 * _fontScale;
	*h = 16 * _fontScale;
	*w = 8 * len * _fontScale;
}

// Show/Hide text cursor
// Parameters:
// cur:(true/false) true:visible, false:not visible
// c: cursor type (NORMAL, BLINK)
void FishinoRA8875Class::showCursor(bool cur,enum RA8875tcursor c)
{
	if (c == BLINK)
	{
		_textCursorStyle = c;
		_MWCR0Reg |= (1 << 5);
	}
	else
	{
		_textCursorStyle = NORMAL;
		_MWCR0Reg &= ~(1 << 5);
	}
	bitWrite(_MWCR0Reg,6,cur);//set cursor visibility flag
	writeReg(RA8875_MWCR0,_MWCR0Reg);
}

// Set cursor property blink and his rate
// Parameters:
// rate:blink speed (fast 0...255 slow)
void FishinoRA8875Class::setCursorBlinkRate(uint8_t rate)
{
	writeReg(RA8875_BTCR,rate);//set blink rate
}

// set the text color and his background
// Parameters:
// fColor:16bit foreground color (text) RGB565
// bColor:16bit background color RGB565
void FishinoRA8875Class::setTextColor(uint16_t fColor, uint16_t bColor)
{
	setForegroundColor(fColor);
	setBackgroundColor(bColor);
	_FNCR1Reg &= ~(1 << 6);
	writeReg(RA8875_FNCR1,_FNCR1Reg);
	FishinoGFX::setTextColor(fColor, bColor);
}

// set the text color w transparent background
// Parameters:
// fColor:16bit foreground color (text) RGB565
void FishinoRA8875Class::setTextColor(uint16_t fColor)
{
	setForegroundColor(fColor);
	_FNCR1Reg |= (1 << 6);
	writeReg(RA8875_FNCR1,_FNCR1Reg);
	FishinoGFX::setTextColor(fColor, fColor);
}

// Set the Text size by it's multiple. normal should=0, max is 3 (x4)
// Parameters:
// scale:0..3  -> 0:normal, 1:x2, 2:x3, 3:x4
void FishinoRA8875Class::setFontScale(uint8_t scale)
{
	if (scale > 4)
		scale = 4;
	else if(scale < 1)
		scale = 1;
	scale--;
	_FNCR1Reg &= ~(0xF); // clear bits from 0 to 3
	_FNCR1Reg |= scale << 2;
	_FNCR1Reg |= scale;
	writeReg(RA8875_FNCR1,_FNCR1Reg);
	_fontScale = scale + 1;
}

// Choose between 16x16(8x16) - 24x24(12x24) - 32x32(16x32)
// for External Font ROM
// Parameters:
// ts:X16,X24,X32
// halfSize:true/false (16x16 -> 8x16 and so on...)
void FishinoRA8875Class::setFontSize(enum RA8875tsize ts, bool halfSize)
{
	switch (ts)
	{
		case X16:
			_FWTSETReg &= 0x3F;
			break;
		case X24:
			_FWTSETReg &= 0x3F;
			_FWTSETReg |= 0x40;
			break;
		case X32:
			_FWTSETReg &= 0x3F;
			_FWTSETReg |= 0x80;
			break;
		default:
			return;
	}
	_textSize = ts;
	writeReg(RA8875_FWTSET,_FWTSETReg);
}

// Choose space in pixels between chars
// Parameters:
// spc:0...63pix (default 0=off)
void FishinoRA8875Class::setFontSpacing(uint8_t spc) //ok
{
	if (spc > 0x3F)
		spc = 0x3F;
	_fontSpacing = spc;
	_FWTSETReg &= 0xC0;
	_FWTSETReg |= spc & 0x3F;
	writeReg(RA8875_FWTSET,_FWTSETReg);

}

// This is the function that write text. Still in development
// NOTE: It identify correctly (when I got it) println and nl & rt
void FishinoRA8875Class::textWrite(const char* buffer, uint16_t len)
{
	// if we're using a gfx font, just call base class print function
	if(_gfxFont)
	{
		FishinoGFX::textWrite(buffer, len);
		return;
	}
	
	bool goBack = false;
	uint8_t start = 0;
	uint16_t i,ny;
	uint8_t t1,t2;
	if (_currentMode == GRAPHIC)
	{
		changeMode(TEXT);
		goBack = true;
	}
	if (len == 0)
		len = strlen(buffer);
	if (len > 0 && ((buffer[0] == '\r') && (buffer[1] == '\n')))
	{
		//got a println?
		//get current y
		t1 = readReg(RA8875_F_CURYL);
		t2 = readReg(RA8875_F_CURYH);
		//calc new line y
		ny = (t2 << 8) | (t1 & 0xFF);
		//update y
		ny = ny + (16 + (16*_fontScale))+_fontInterline;//TODO??
		setCursor(0,ny);
		start = 2;
#if defined(ENERGIA)//oops! Energia 013 seems have a bug here! Should send a \r but only \n given!
	}
	else
		if (len > 0 && ((buffer[0] == '\n')))
		{
			//get current y
			t1 = readReg(RA8875_F_CURYL);
			t2 = readReg(RA8875_F_CURYH);
			//calc new line y
			ny = (t2 << 8) | (t1 & 0xFF);
			//update y
			ny = ny + (16 + (16*_textScale))+_fontInterline;//TODO??
			setCursor(0,ny);
			start = 1;
		}
#else
	}
#endif
	writeCommand(RA8875_MRWC);
	for (i=start;i<len;i++)
	{
		if (buffer[i] == '\n' || buffer[i] == '\r')
		{
			//_cursor_y += textsize * 8;
			//_cursor_x  = 0;

		}
		else
		{
			writeData(buffer[i]);
			waitBusy(0x80);
		}
#if defined(__AVR__)
		if (_fontScale > 1)
			delay(1);
#elif defined(__arm__)
		if (_fontScale > 0)
			delay(1);//Teensy3
#endif
	}
	if (goBack)
		changeMode(GRAPHIC);
}
