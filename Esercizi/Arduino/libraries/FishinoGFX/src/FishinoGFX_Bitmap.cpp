#include "FishinoGFX.h"

// Draw a 1-bit image (bitmap) at the specified (x,y) position from the
// provided bitmap buffer (must be PROGMEM memory) using the specified
// foreground color (unset bits are transparent).
void FishinoGFX::drawBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
{
	int16_t i, j, byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	for (j = 0; j < (int16_t)h; j++)
	{
		for (i = 0; i < (int16_t)w; i++)
		{
			if (i & 7)
				byte <<= 1;
			else
				byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
			if (byte & 0x80)
				drawPixel0(x+i, y+j, color);
		}
	}
}

// Draw a 1-bit image (bitmap) at the specified (x,y) position from the
// provided bitmap buffer (must be PROGMEM memory) using the specified
// foreground (for set bits) and background (for clear bits) colors.
void FishinoGFX::drawBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color, uint16_t bg)
{
	int16_t i, j, byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	for (j = 0; j < (int16_t)h; j++)
	{
		for (i = 0; i < (int16_t)w; i++)
		{
			if (i & 7)
				byte <<= 1;
			else
				byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
			if (byte & 0x80)
				drawPixel0(x+i, y+j, color);
			else
				drawPixel0(x+i, y+j, bg);
		}
	}
}

// drawBitmap() variant for RAM-resident (not PROGMEM) bitmaps.
void FishinoGFX::drawBitmap0(int16_t x, int16_t y, uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
{
	int16_t i, j, byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	for (j = 0; j < (int16_t)h; j++)
	{
		for (i = 0; i < (int16_t)w; i++)
		{
			if (i & 7)
				byte <<= 1;
			else
				byte = bitmap[j * byteWidth + i / 8];
			if (byte & 0x80)
				drawPixel0(x+i, y+j, color);
		}
	}
}

// drawBitmap() variant w/background for RAM-resident (not PROGMEM) bitmaps.
void FishinoGFX::drawBitmap0(int16_t x, int16_t y, uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color, uint16_t bg)
{
	int16_t i, j, byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	for (j = 0; j < (int16_t)h; j++)
	{
		for (i = 0; i < (int16_t)w; i++)
		{
			if (i & 7)
				byte <<= 1;
			else
				byte = bitmap[j * byteWidth + i / 8];
			if (byte & 0x80)
				drawPixel0(x+i, y+j, color);
			else
				drawPixel0(x+i, y+j, bg);
		}
	}
}

//Draw XBitMap Files (*.xbm), exported from GIMP,
//Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
//C Array can be directly used with this function
void FishinoGFX::drawXBitmap0(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t w, uint16_t h, uint16_t color)
{
	int16_t i, j, byteWidth = (w + 7) / 8;
	uint8_t byte = 0;

	for (j = 0; j < (int16_t)h; j++)
	{
		for (i = 0; i < (int16_t)w; i++)
		{
			if (i & 7)
				byte >>= 1;
			else
				byte = pgm_read_byte(bitmap + j * byteWidth + i / 8);
			if (byte & 0x01)
				drawPixel0(x+i, y+j, color);
		}
	}
}

// draws a color bitmap on displays that supports 16 bit (565) color format
void FishinoGFX::drawBitmap0(ICON const &icon, int16_t x, int16_t y)
{
	if(icon.hasTransparency)
#ifdef __AVR__
		bitBltT0(x, y, pgm_read_word(&icon.width), pgm_read_word(&icon.height), pgm_read_word(&icon.pixels));
#else
		bitBltT0(x, y, icon.width, icon.height, icon.pixels);
#endif
	else
#ifdef __AVR__
		bitBlt0(x, y, pgm_read_word(&icon.width), pgm_read_word(&icon.height), pgm_read_word(&icon.pixels));
#else
		bitBlt0(x, y, icon.width, icon.height, icon.pixels);
#endif
}

void FishinoGFX::drawBitmapInverted0(ICON const &icon, int16_t x, int16_t y)
{
	if(icon.hasTransparency)
#ifdef __AVR__
		bitBltInvT0(x, y, pgm_read_word(&icon.width), pgm_read_word(&icon.height), pgm_read_word(&icon.pixels));
#else
		bitBltInvT0(x, y, icon.width, icon.height, icon.pixels);
#endif
	else
#ifdef __AVR__
		bitBltInv0(x, y, pgm_read_word(&icon.width), pgm_read_word(&icon.height), pgm_read_word(&icon.pixels));
#else
		bitBltInv0(x, y, icon.width, icon.height, icon.pixels);
#endif
}

