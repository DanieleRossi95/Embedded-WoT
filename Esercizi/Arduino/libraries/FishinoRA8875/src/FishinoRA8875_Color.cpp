#include "FishinoRA8875SPI.h"

// set 8 BPP mode (still untested!)
void FishinoRA8875Class::set8BPP(void)
{
	uint8_t r = readReg(0x10);
	r &= ~0x0c;
	writeReg(0x10, r);
}

// set 16 bpp mode
void FishinoRA8875Class::set16BPP(void)
{
	uint8_t r = readReg(0x10);
	r |= 0x0c;
	writeReg(0x10, r);
}

// Sets set the foreground color using 16bit RGB565 color
// Parameters:
// color:16bit color RGB565
void FishinoRA8875Class::setForegroundColor(uint16_t color)
{
	writeReg(RA8875_FGCR0,((color & 0xF800) >> 11));
	writeReg(RA8875_FGCR1,((color & 0x07E0) >> 5));
	writeReg(RA8875_FGCR2,(color & 0x001F));
}

// Sets set the foreground color using 8bit R,G,B
// Parameters:
// R:8bit RED
// G:8bit GREEN
// B:8bit BLUE
void FishinoRA8875Class::setForegroundColor(uint8_t R,uint8_t G,uint8_t B)
{
	writeReg(RA8875_FGCR0,R);
	writeReg(RA8875_FGCR1,G);
	writeReg(RA8875_FGCR2,B);
}

// Sets set the background color using 16bit RGB565 color
// Parameters:
// color:16bit color RGB565
void FishinoRA8875Class::setBackgroundColor(uint16_t color)
{
	writeReg(RA8875_BGCR0,((color & 0xF800) >> 11));
	writeReg(RA8875_BGCR1,((color & 0x07E0) >> 5));
	writeReg(RA8875_BGCR2,(color & 0x001F));
}

// Sets set the background color using 8bit R,G,B
// Parameters:
// R:8bit RED
// G:8bit GREEN
// B:8bit BLUE
void FishinoRA8875Class::setBackgroundColor(uint8_t R,uint8_t G,uint8_t B)
{
	writeReg(RA8875_BGCR0,R);
	writeReg(RA8875_BGCR1,G);
	writeReg(RA8875_BGCR2,B);
}

// Sets set the trasparent background color using 16bit RGB565 color
// Parameters:
// color:16bit color RGB565
void FishinoRA8875Class::setTrasparentColor(uint16_t color)
{
	writeReg(RA8875_BGTR0,((color & 0xF800) >> 11));
	writeReg(RA8875_BGTR1,((color & 0x07E0) >> 5));
	writeReg(RA8875_BGTR2,(color & 0x001F));
}

// Sets set the Trasparent background color using 8bit R,G,B
// Parameters:
// R:8bit RED
// G:8bit GREEN
//  B:8bit BLUE
void FishinoRA8875Class::setTrasparentColor(uint8_t R,uint8_t G,uint8_t B)
{
	writeReg(RA8875_BGTR0,R);
	writeReg(RA8875_BGTR1,G);
	writeReg(RA8875_BGTR2,B);
}
