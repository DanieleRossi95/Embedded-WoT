#include "FishinoRA8875SPI.h"

// Initialize library
// must be called in derived classes AFTER interface initialization
void FishinoRA8875Class::begin(void)
{
	uint8_t initIndex;

	if (_size == RA8875_320x240)
	{
		//still not supported! Wait next version
		_width = 320;
		_height = 240;
		initIndex = 0;
		_maxLayers = 2;
	}
	else if(_size == RA8875_480x272 || _size == Adafruit_480x272)
	{
		_width = 480;
		_height = 272;
		initIndex = 1;
		_maxLayers = 2;
	}
	else if(_size == RA8875_640x480)
	{
		//still not supported! Wait next version
		_width = 640;
		_height = 480;
		initIndex = 2;
		_maxLayers = 1;
	}
	else if(_size == RA8875_800x480 || _size == Adafruit_800x480)
	{
		_width = 800;
		_height = 480;
		initIndex = 3;
		_maxLayers = 1;
	}
	else
	{
		_width = 480;
		_height = 272;
		initIndex = 1;
		_maxLayers = 2;
	}

	_currentLayer = 0;
	_currentMode = GRAPHIC;
	_cursorX = 0;
	_cursorY = 0;
	_swapxy = false;
	_textWrap = true;
	_textSize = X16;
	_fontSpacing = 0;
	_extFontRom = false;
	_fontRomType = GT21L16T1W;
	_fontRomCoding = GB2312;
	_fontSource = INT;
	_fontFullAlig = false;
	_fontRotation = false;
	_fontInterline = 0;
	_fontFamily = STANDARD;
	_textCursorStyle = BLINK;
	_scrollXL = 0;
	_scrollXR = 0;
	_scrollYT = 0;
	_scrollYB = 0;
	_useMultiLayers = false;//starts with one layer only
	
#if !defined(USE_EXTERNALTOUCH)
	_touchPin = 255;
	_clearTInt = false;
	_touchEnabled = false;
	_tsAdcMinX = 0;
	_tsAdcMinY = 0;
	_tsAdcMaxX = 1024;
	_tsAdcMaxY = 1024;
#endif

#if defined(USE_RA8875_KEYMATRIX)
	_keyMatrixEnabled = false;
#endif

	// Display Configuration Register	  [0x20]
	// 7: (Layer Setting Control) 0:one Layer, 1:two Layers
	// 6,5,4: (na)
	// 3: (Horizontal Scan Direction) 0: SEG0 to SEG(n-1), 1: SEG(n-1) to SEG0
	// 2: (Vertical Scan direction) 0: COM0 to COM(n-1), 1: COM(n-1) to COM0
	// 1,0: (na)
	_DPCRReg = 0b00000000;
	
	//	Memory Write Control Register 0
	//		7: 0(graphic mode), 1(textx mode)
	//		6: 0(font-memory cursor not visible), 1(visible)
	//		5: 0(normal), 1(blinking)
	//		4: na
	//		3-2: 00(LR,TB), 01(RL,TB), 10(TB,LR), 11(BT,LR)
	//		1: 0(Auto Increase in write), 1(no)
	//		0: 0(Auto Increase in read), 1(no)
	_MWCR0Reg = 0b00000000;

	//	Font Control Register 0 [0x21]
	//		7: 0(CGROM font is selected), 1(CGRAM font is selected)
	//		6: na
	//		5: 0(Internal CGROM [reg 0x2F to 00]), 1(External CGROM [0x2E reg, bit6,7 to 0)
	//		4-2: na
	//		1-0: 00(ISO/IEC 8859-1), 01(ISO/IEC 8859-2), 10(ISO/IEC 8859-3), 11(ISO/IEC 8859-4)
	_FNCR0Reg = 0b00000000;

	//	Font Control Register 1 [0x22]
	//		7: 0(Full Alignment off), 1(Full Alignment on)
	//		6: 0(no-trasparent), 1(trasparent)
	//		5: na
	//		4: 0(normal), 1(90degrees)
	//		3-2: 00(x1), 01(x2), 10(x3), 11(x3) Horizontal font scale
	//		1-0: 00(x1), 01(x2), 10(x3), 11(x3) Vertical font scale
	_FNCR1Reg = 0b00000000;

	//	Font Write Type Setting Register [0x2E]
	//	7-6: 00(16x16,8x16,nx16), 01(24x24,12x24,nx24), 1x(32x32,16x32, nx32)
	//	5-0: 00...3F (font width off to 63 pixels)
	_FWTSETReg = 0b00000000;

	//	Serial Font ROM Setting [0x2F]
	//	GT Serial Font ROM Select
	//	7-5: 000(GT21L16TW/GT21H16T1W),001(GT30L16U2W),010(GT30L24T3Y/GT30H24T3Y),011(GT30L24M1Z),111(GT30L32S4W/GT30H32S4W)
	//	FONT ROM Coding Setting
	//	4-2: 000(GB2312),001(GB12345/GB18030),010(BIG5),011(UNICODE),100(ASCII),101(UNI-Japanese),110(JIS0208),111(Latin/Greek/Cyrillic/Arabic)
	//	1-0: 00...11
	//		 bits	ASCII		Lat/Gr/Cyr		Arabit
	//		 00		normal		normal			na
	//		 01		Arial		var Wdth		Pres Forms A
	//		 10		Roman		na				Pres Forms B
	//		 11		Bold		na				na
	_SFRSETReg = 0b00000000;

	//	Interrupt Control Register1		  [0xF0]
	//	7,6,5: (na)
	//	4: KEYSCAN Interrupt Enable Bit
	//	3: DMA Interrupt Enable Bit
	//	2: TOUCH Panel Interrupt Enable Bit
	//	1: BTE Process Complete Interrupt Enable Bit
	//	0:
	//	When MCU-relative BTE operation is selected(*1) and BTE
	//	Function is Enabled(REG[50h] Bit7 = 1), this bit is used to
	//		Enable the BTE Interrupt for MCU R/W:
	//		0 : Disable BTE interrupt for MCU R/W.
	//		1 : Enable BTE interrupt for MCU R/W.
	//	When the BTE Function is Disabled, this bit is used to
	//		Enable the Interrupt of Font Write Function:
	//		0 : Disable font write interrupt.
	//		1 : Enable font write interrupt.
	_INTC1Reg = 0b00000000;

#if !defined(USE_EXTERNALTOUCH)
	//	Touch Panel Control Register 0     [0x70]
	//	7: 0(disable, 1:(enable)
	//	6,5,4:TP Sample Time Adjusting (000...111)
	//	3:Touch Panel Wakeup Enable 0(disable),1(enable)
	//	2,1,0:ADC Clock Setting (000...111) set fixed to 010: (System CLK) / 4, 10Mhz Max! */
	//	_TPCR0Reg = RA8875_TPCR0_WAIT_4096CLK | RA8875_TPCR0_WAKEDISABLE | RA8875_TPCR0_ADCCLK_DIV4;
#endif

	// do hardware initialization
	initialize(initIndex);
}

// Hardware initialization of RA8875 and turn on
void FishinoRA8875Class::initialize(uint8_t initIndex)
{
	// soft reset
	writeCommand(RA8875_PWRR);
	writeData(RA8875_PWRR_SOFTRESET);
	writeData(RA8875_PWRR_NORMAL);
	delay(200);

	const uint8_t initStrings[4][15] =
	{
		{0x0A,0x02,0x03,0x27,0x00,0x05,0x04,0x03,0xEF,0x00,0x05,0x00,0x0E,0x00,0x02},//0 -> 320x240 (to be fixed)
		{0x10,0x02,0x82,0x3B,0x00,0x01,0x00,0x05,0x0F,0x01,0x02,0x00,0x07,0x00,0x09},//1 -> 480x272 (0x0A)
		{0x0B,0x02,0x01,0x4F,0x05,0x0F,0x01,0x00,0xDF,0x10,0x0A,0x00,0x0E,0x00,0x01},//2 -> 640x480 (to be fixed)
		{0x10,0x02,0x81,0x63,0x00,0x03,0x03,0x0B,0xDF,0x01,0x1F,0x00,0x16,0x00,0x01}// 3 -> 800x480 (0x0B)(to be fixed?)
	};

	// PLL Control Register 1
	writeReg(RA8875_PLLC1,initStrings[initIndex][0]);
	delay(1);
	
	// PLL Control Register 2
	writeReg(RA8875_PLLC2,initStrings[initIndex][1]);
	delay(1);

	// Pixel Clock Setting Register
	writeReg(RA8875_PCSR,initStrings[initIndex][2]);
	delay(1);
	
	// we are working ALWAYS at 65K color space!!!!
	writeReg(RA8875_SYSR,0x0C);
	
	// LCD Horizontal Display Width Register
	writeReg(RA8875_HDWR,initStrings[initIndex][3]);
	
	// Horizontal Non-Display Period Fine Tuning Option Register
	writeReg(RA8875_HNDFTR,initStrings[initIndex][4]);
	
	// LCD Horizontal Non-Display Period Register
	writeReg(RA8875_HNDR,initStrings[initIndex][5]);
	
	// HSYNC Start Position Register
	writeReg(RA8875_HSTR,initStrings[initIndex][6]);
	
	// HSYNC Pulse Width Register
	writeReg(RA8875_HPWR,initStrings[initIndex][7]);
	
	// LCD Vertical Display Height Register0
	writeReg(RA8875_VDHR0,initStrings[initIndex][8]);
	
	// LCD Vertical Display Height Register1
	writeReg(RA8875_VDHR1,initStrings[initIndex][9]);
	
	// LCD Vertical Non-Display Period Register 0
	writeReg(RA8875_VNDR0,initStrings[initIndex][10]);
	
	// LCD Vertical Non-Display Period Register 1
	writeReg(RA8875_VNDR1,initStrings[initIndex][11]);
	
	// VSYNC Start Position Register 0
	writeReg(RA8875_VSTR0,initStrings[initIndex][12]);
	
	// VSYNC Start Position Register 1
	writeReg(RA8875_VSTR1,initStrings[initIndex][13]);
	
	// VSYNC Pulse Width Register
	writeReg(RA8875_VPWR,initStrings[initIndex][14]);
	
	// set the active window
	setActiveWindow(0, 0, _width - 1, _height - 1);

	// clear FULL memory
	clearMemory(true);

	// end of hardware initialization
	delay(10);

	// now starts the first time setting up

	// turn On Display
	displayOn(true);
	
	// only for adafruit stuff
	if (_size == Adafruit_480x272 || _size == Adafruit_800x480)
		GPIOX(true);
	
	// setup PWM ch 1 for backlight
	PWMsetup(1,true, RA8875_PWM_CLK_DIV1024);
	
	// turn on PWM1
	PWMout(1,255);
	
	// set default blink rate
	setCursorBlinkRate(DEFAULTCURSORBLINKRATE);
	
	// set default text cursor type and turn off
	if (_textCursorStyle == BLINK)
		showCursor(false,BLINK);
	
	// set default internal font encoding
	setIntFontCoding(DEFAULTINTENCODING);
	
	// set internal font use
	setFont(INT);
	
	// since the blackground it's black...
	setTextColor(GFX_WHITE);
	
	// now tft it's ready to go and in [Graphic mode]
}

// Software Reset
void FishinoRA8875Class::softReset(void)
{
	writeCommand(RA8875_PWRR);
	writeData(RA8875_PWRR_SOFTRESET);
	writeData(RA8875_PWRR_NORMAL);
	delay(200);
}

// Clear memory
// Parameters:
// full: true(clear all memory), false(clear active window only)
void FishinoRA8875Class::clearMemory(bool full)
{
	uint8_t temp = 0b00000000;
	if (!full)
		temp |= (1 << 6);
	temp |= (1 << 7);//enable start bit
	writeReg(RA8875_MCLR,temp);
	//_cursorX = _cursorY = 0;
	waitBusy(0x80);
}

// Change the mode between graphic and text
// Parameters:
// m: can be GRAPHIC or TEXT
void FishinoRA8875Class::changeMode(enum RA8875modes m)
{
	writeCommand(RA8875_MWCR0);
	if (m == GRAPHIC)
	{
		if (_currentMode == TEXT)
		{
			//avoid useless consecutive calls
			_MWCR0Reg &= ~(1 << 7);
			_currentMode = GRAPHIC;
			writeData(_MWCR0Reg);
		}
	}
	else
	{
		if (_currentMode == GRAPHIC)
		{
			//avoid useless consecutive calls
			_MWCR0Reg |= (1 << 7);
			_currentMode = TEXT;
			writeData(_MWCR0Reg);
		}
	}
}

// on/off GPIO (basic for Adafruit module)
void FishinoRA8875Class::GPIOX(bool on)
{
	writeReg(RA8875_GPIOX, on);
}

// Setup PWM engine
// Parameters:
// pw:pwm selection (1,2)
// on: turn on/off
// clock: the clock setting
void FishinoRA8875Class::PWMsetup(uint8_t pw,bool on, uint8_t clock)
{
	uint8_t reg;
	uint8_t set;
	if (pw > 1)
	{
		reg = RA8875_P2CR;
		if (on)
		{
			set = RA8875_PxCR_ENABLE;
		}
		else
		{
			set = RA8875_PxCR_DISABLE;
		}
	}
	else
	{
		reg = RA8875_P1CR;
		if (on)
		{
			set = RA8875_PxCR_ENABLE;
		}
		else
		{
			set = RA8875_PxCR_DISABLE;
		}
	}
	writeReg(reg,(set | (clock & 0xF)));
}

// PWM out
// Parameters:
// pw:pwm selection (1,2)
// p:0...255 rate
void FishinoRA8875Class::PWMout(uint8_t pw,uint8_t p)
{
	uint8_t reg;
	if (pw > 1)
	{
		reg = RA8875_P2DCR;
	}
	else
	{
		reg = RA8875_P1DCR;
	}
	writeReg(reg, p);
}

// Set the brightness of the backlight (if connected to pwm)
// (basic controls pwm 1)
// Parameters:
// val:0...255
void FishinoRA8875Class::brightness(uint8_t val)
{
	PWMout(1,val);
}

// turn display on/off
void FishinoRA8875Class::displayOn(bool on)
{
	on == true ? writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON) : writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}


// Sleep mode on/off (caution! in SPI this need some more code!)
void FishinoRA8875Class::sleep(bool sleep)
{
	sleep == true ? writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP) : writeReg(RA8875_PWRR, RA8875_PWRR_DISPOFF);
}
