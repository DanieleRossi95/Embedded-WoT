#include "FishinoRA8875SPI.h"

//////////////////////////////////////////////////////////////////////////
// 					IMAGE WRITE SUPPORT -- NON-BTE MODE					//
//		QUITE SLOW WITH TRANSPARENCY, SO PREFERE THE BTE VERSION		//
//////////////////////////////////////////////////////////////////////////

// copy an image buffer to image ram -- non-BTE version
// supports 4 sides image crop and transparency
// slow if used with transparency -- not used anymore, let here just for documentation
void FishinoRA8875Class::writeImageBuf(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t skipLeft, uint16_t skipRight, uint16_t skipTop, uint16_t skipBottom, bool inv, bool transp, uint16_t transpColor, bool flashBuf, uint16_t const *buf)
{
	// store current active window
	uint16_t wx1, wy1, wx2, wy2;
	getActiveWindow(wx1, wy1, wx2, wy2);
	
	// calculate image extents
	uint16_t ww = w - skipLeft - skipRight;
	uint16_t hh = h - skipTop - skipBottom;
	// write to image RAM
	writeTo(L1);
	
	// skip top image part, if needed
	buf += skipTop * w;
	
	// handle rotation
	uint16_t dx, dy;
	if(_swapxy)
	{
		swap(x, y);
		y -= h;
		dx = 1;
		dy = 0;
	}
	else
	{
		dx = 0;
		dy = 1;
	}
	
	// non-transparent path - faster
	if(!transp)
	{
		// transfer image, row by row, skipping unneeded parts
		for(uint16_t iy = 0; iy < hh; iy++)
		{
			// skip left cropped data, if any
			buf += skipLeft;
			
			// set cursor to start of line
			// (doing so we can recover from SPI error, partially)
			setCursor0(x, y);
			x += dx;
			y += dy;
			
			// start writing
			writeCommand(RA8875_MRWC);

			if(flashBuf)
				writeBuffer((const __FlashStringHelper *)buf, ww);
			else
				writeBuffer(buf, ww);
	
			// skip right cropped part, if needed
			buf += ww + skipRight;
		}
	}

	// transparent path - quite slow
	// must read each pixel before writing
	else
	{
		// transfer image, row by row, skipping unneeded parts
		for(uint16_t iy = 0; iy < hh; iy++)
		{
			setReadCursor0(x, y);
			setCursor0(x, y);
			x += dx;
			y += dy;
			writeCommand(RA8875_MRWC);

			// skip left cropped data, if any
			buf += skipLeft;
			
			if(flashBuf)
				writeBufferT((const __FlashStringHelper *)buf, ww, transpColor);
			else
				writeBufferT(buf, ww, transpColor);
			buf += ww + skipRight;
		}
	}
	
	// restore active window
	setActiveWindow(wx1, wy1, wx2, wy2);
}


//////////////////////////////////////////////////////////////////////////
// 							IMAGE WRITE SUPPORT -- BTE MODE				//
//////////////////////////////////////////////////////////////////////////

// copy an image buffer to image ram -- BTE version
// supports 4 sides image crop and transparency
void FishinoRA8875Class::writeBTEImageBuf(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t skipLeft, uint16_t skipRight, uint16_t skipTop, uint16_t skipBottom, bool inv, bool transp, uint16_t transpColor, bool flashBuf, uint16_t const *buf)
{

	uint16_t ww = w - skipLeft - skipRight;
	uint16_t hh = h - skipTop - skipBottom;

	// setup block sizes
	uint16_t dx, dy;
	if(_swapxy)
	{
		swap(x, y);
		dy = 0;
		dx = 1;
		BteSize(1, ww);
	}
	else
	{
		dy = 1;
		dx = 0;
		BteSize(ww, 1);
	}

	// BTE transparent color
	if(transp)
		BteSetTransparentColor(transpColor);

	// Setting BTE operation and ROP
	uint8_t op, rop;
	if(!inv && !transp)
	{
		op = RA8875_BECR1_OP_WRITE;
		rop = RA8875_BECR1_ROP_S;
	}
	else if(inv && !transp)
	{
		op = RA8875_BECR1_OP_WRITE;
		rop = RA8875_BECR1_ROP_NOTS;
	}
	else if(!inv && transp)
	{
		op = RA8875_BECR1_OP_WRITE_TRANSPARENT;
		rop = RA8875_BECR1_ROP_S;
	}
	else
	{
		op = RA8875_BECR1_OP_WRITE_TRANSPARENT;
		rop = RA8875_BECR1_ROP_NOTS;
	}
	BteOpRop(op, rop);
	
	// skip top part
	buf += skipTop * w;

	// transfer image, row by row, skipping unneeded parts
	for(uint16_t iy = 0; iy < hh; iy++)
	{
		// setup destination position
		BteDest(x, y);
		x += dx;
		y += dy;
	
		// skip left cropped data, if any
		buf += skipLeft;

		// Enable BTE function
		BteStart();
		
		// start data write operations
		writeCommand(RA8875_MRWC);
	
		// transfer the row
		if(flashBuf)
			writeBuffer((const __FlashStringHelper *)buf, ww);
		else
			writeBuffer(buf, ww);

		// skip transferred buffer and right cropped data, if any
		buf += ww + skipRight;
	}
}


//////////////////////////////////////////////////////////////////////////
// 								BITBLT									//
//////////////////////////////////////////////////////////////////////////


void FishinoRA8875Class::bitBltHelper(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t const *buf, bool flashBuf, bool inv, bool transp, uint16_t transpColor)
{
	uint16_t dxl, dxr;
	uint16_t dyt, dyb;
	uint16_t wc = w;
	uint16_t hc = h;
	
	// full out of screen image skip
	if(x + w < 0 || x >= width() || y + h < 0 || y >= height())
		return;

	// left/top crop
	if(x >= 0)
		dxl = 0;
	else
	{
		dxl = -x;
		x = 0;
		if(w < dxl)
			return;
		wc -= dxl;
	}
	if(y >= 0)
		dyt = 0;
	else
	{
		dyt = -y;
		y = 0;
		if(h < dyt)
			return;
		hc -= dyt;
	}
	
	// right/bottom crop
	if(x + (int16_t)wc > width())
	{
		dxr = x + wc - width();;
		wc -= dxr;
	}
	else
		dxr = 0;
	if(y + (int16_t)hc > height())
	{
		dyb = y + hc - height();
		hc -= dyb;
	}
	else
		dyb = 0;

	// write image using BTE engine (fast transparency!)
	writeBTEImageBuf(x, y, w, h, dxl, dxr, dyt, dyb, inv, transp, transpColor, flashBuf, buf);
}
