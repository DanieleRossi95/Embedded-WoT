#include "FishinoRA8875SPI.h"

// 1. Setting up the range of active windows RA8875_HSAW0..VEAW1 (REG[30h] ~REG[37h]) and memory write
//    cursor position RA8875_CURH0-CURH1-CURV0-CURV1 (REG[46h] ~REG[49h])
// 2. Setting up Serial Flash/ROM configuration RA8875_SROC (REG[05h]).
// 3. Setting up DMA source starting address RA8875_SSAR0..2(REG[B0h] ~REG[B2h]).
// 4. Setting up DMA transfer number RA8875_DTNR0..2 (REG[B4h], REG[B6h]and REG[B8h]).
// 5. Enable DMA start and check DMA busy signal by RA8875_DMACR REG[BFh] bit 0.
void FishinoRA8875Class::drawFlashImage0(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t address)
{
	// flash config
	uint8_t sroc =
		(1 << 7)	|		// flash1
		(0 << 6)	|		// serial flash
		(1 << 5)	|		// mode 3
		(0 << 3)	|		// no dummy cycle
		(1 << 2)	|		// DMA mode
		(3 << 0)			// dual mode 0 or 1 (NOT single mode)
	;
	writeReg(RA8875_SROC, sroc);
	
	// save cursor position
	int16_t xc, yc;
	getCursor0(xc, yc);
	
	// save previous active window
	uint16_t x1, y1, x2, y2;
	getActiveWindow(x1, x2, y1, y2);
	
	// set active window (30-37)
	setActiveWindow(x, y, x + w - 1, y + h - 1);
	
	// set cursor
	setCursor0(x, y);
	
	// DMA start address
	writeReg(RA8875_SSAR0, address & 0xff);
	writeReg(RA8875_SSAR1, (address >> 8) & 0xff);
	writeReg(RA8875_SSAR2, (address >> 16) & 0xff);
	
	// transfer count
	uint32_t count = (uint32_t)w * (uint32_t)h;
	writeReg(RA8875_DTNR0, count & 0xff);
	writeReg(RA8875_DTNR1, (count >> 8) & 0xff);
	writeReg(RA8875_DTNR2, (count >> 16) & 0xff);
	
	// start DMA in continuous mode
	writeReg(RA8875_DMACR, 0x01);
	
	// wait till finished
	while(readReg(RA8875_DMACR) & 0x01)
		;
	
	// reset curso
	setCursor0(xc, yc);
	
	// reset active window
	setActiveWindow(x1, y1, x2, y2);
}
