#include "FishinoRA8875SPI.h"

//////////////////////////////////////////////////////////////////////////
// 							BTE ENGINE SUPPORT							//
//////////////////////////////////////////////////////////////////////////

// set BTE block source
void FishinoRA8875Class::BteSource(uint16_t sx,uint16_t sy)
{
	writeReg(RA8875_HSBE0, sx);
	writeReg(RA8875_HSBE1, sx >> 8);

	writeReg(RA8875_VSBE0, sy);
	writeReg(RA8875_VSBE1, (readReg(RA8875_VSBE1) & 0x80) | (sy >> 8));
}

// set BTE block destination
void FishinoRA8875Class:: BteDest(uint16_t dx,uint16_t dy)
{
	writeReg(RA8875_HDBE0, dx);
	writeReg(RA8875_HDBE1, dx >> 8);

	writeReg(RA8875_VDBE0, dy);
	writeReg(RA8875_VDBE1, (readReg(RA8875_VDBE1) & 0x80) | (dy >> 8));
}

// set BTE block size
void FishinoRA8875Class::BteSize(uint16_t w, uint16_t h)
{
	writeReg(RA8875_BEWR0, w);
	writeReg(RA8875_BEWR1, w >> 8);
	writeReg(RA8875_BEHR0, h);
	writeReg(RA8875_BEHR1, h >> 8);
}

// sets operation and rop code
void FishinoRA8875Class::BteOpRop(uint8_t op, uint8_t rop)
{
	writeReg(RA8875_BECR1, op | rop);
}

// sets BTE background, foreground and transparent color
void FishinoRA8875Class::BteSetBackgroundColor(uint8_t r, uint8_t g, uint8_t b)
{
	writeReg(RA8875_BGCR0, r);
	writeReg(RA8875_BGCR1, g);
	writeReg(RA8875_BGCR2, b);
}

void FishinoRA8875Class::BteSetForegroundColor(uint8_t r, uint8_t g, uint8_t b)
{
	writeReg(RA8875_FGCR0, r);
	writeReg(RA8875_FGCR1, g);
	writeReg(RA8875_FGCR2, b);
}

// start BTE transfer
void FishinoRA8875Class::BteStart(void)
{
	writeReg(RA8875_BECR0, readReg(RA8875_BECR0) | 0x80);
}

// wait for BTE engine to complete
void FishinoRA8875Class::BteWait(void)
{
	// Check STSR Bit6
	while(readStatus() & 0x40)
		;
}
