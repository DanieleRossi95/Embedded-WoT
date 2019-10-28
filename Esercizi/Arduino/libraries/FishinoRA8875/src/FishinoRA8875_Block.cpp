#include "FishinoRA8875SPI.h"

// block move
void FishinoRA8875Class::blockMove(uint16_t sx, uint16_t sy, uint8_t sl, uint16_t dx, uint16_t dy, uint8_t dl, uint16_t w, uint16_t h)
{
	// horizontal source point
	writeReg(0x54, sx & 0xff);
	writeReg(0x55, (sx >> 8) & 0x03);
	
	// vertical source point and source layer
	writeReg(0x56, sy & 0xff);
	writeReg(0x57, ((sy >> 8) & 0x01) | (sl == L1 ? 0 : 0x80));
	
	// horizontal dest point
	writeReg(0x58, dx & 0xff);
	writeReg(0x59, (dx >> 8) & 0x03);
	
	// vertical dest point and source layer
	writeReg(0x5a, dy & 0xff);
	writeReg(0x5b, ((dy >> 8) & 0x01) | (dl == L1 ? 0 : 0x80));
	
	// width
	writeReg(0x5c, w & 0xff);
	writeReg(0x5d, (w >> 8) & 0x03);
	
	// height
	writeReg(0x5e, h & 0xff);
	writeReg(0x5f, (h >> 8) & 0x03);
	
	// rop function == MOVE
	uint8_t opRop;
	if((int16_t)dy - sy > 0 || (dy == sy && dx - sx > 0))
		opRop = 0x02;
	else
		opRop = 0x03;
	opRop |= 0xc0;
	writeReg(0x51, opRop);
	
	// start operation
	writeReg(0x50, 0x80);
	
	// wait till end
	while(readReg(0x50) & 0x80)
		;
}

// block vertical scroll area, current layer
void FishinoRA8875Class::blockVScroll(uint16_t sx, uint16_t sy, uint16_t w, uint16_t h, int16_t dy, uint8_t lay, uint16_t fillColor)
{
	// move the area
	if(dy > 0)
	{
		blockMove(sx, sy, lay, sx, sy + dy, lay, w, h - dy);
		fillRect(sx, sy, w, dy, fillColor);
	}
	else
	{
		blockMove(sx, sy - dy, lay, sx, sy, lay, w, h + dy);
		fillRect(sx, sy + h + dy, w, -dy, fillColor);
	}
}

/**************************************************************************/
void FishinoRA8875Class::setScrollWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
{
	checkLimitsHelper(XL,YT);
	checkLimitsHelper(XR,YB);

	_scrollXL = XL;
	_scrollXR = XR;
	_scrollYT = YT;
	_scrollYB = YB;

	writeReg(RA8875_HSSW0,_scrollXL);
	writeReg(RA8875_HSSW1,_scrollXL >> 8);

	writeReg(RA8875_HESW0,_scrollXR);
	writeReg(RA8875_HESW1,_scrollXR >> 8);

	writeReg(RA8875_VSSW0,_scrollYT);
	writeReg(RA8875_VSSW1,_scrollYT >> 8);

	writeReg(RA8875_VESW0,_scrollYB);
	writeReg(RA8875_VESW1,_scrollYB >> 8);
}

/**************************************************************************/
/*!
		Perform the scroll

*/
/**************************************************************************/
void FishinoRA8875Class::scroll(uint16_t x,uint16_t y)
{
	if ((int16_t)y > _scrollYB)
		y = _scrollYB;//??? mmmm... not sure
	if (_scrollXL == 0 && _scrollXR == 0 && _scrollYT == 0 && _scrollYB == 0)
	{
		//do nothing, scroll window inactive
	}
	else
	{
		writeReg(RA8875_HOFS0,x);
		writeReg(RA8875_HOFS1,x >> 8);

		writeReg(RA8875_VOFS0,y);
		writeReg(RA8875_VOFS1,y >> 8);
	}
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void FishinoRA8875Class::DMA_blockModeSize(int16_t BWR,int16_t BHR,int16_t SPWR)
{
	writeReg(RA8875_DTNR0,BWR);
	writeReg(RA8875_BWR1,BWR >> 8);

	writeReg(RA8875_DTNR1,BHR);
	writeReg(RA8875_BHR1,BHR >> 8);

	writeReg(RA8875_DTNR2,SPWR);
	writeReg(RA8875_SPWR1,SPWR >> 8);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void FishinoRA8875Class::DMA_startAddress(unsigned long adrs)
{
	writeReg(RA8875_SSAR0,adrs);
	writeReg(RA8875_SSAR1,adrs >> 8);
	writeReg(RA8875_SSAR2,adrs >> 16);
	//writeReg(0xB3,adrs >> 24);// not more in datasheet!
}
