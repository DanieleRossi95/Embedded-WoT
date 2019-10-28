#include "FishinoGFX.h"
#include "FishinoGFX_defs.h"
#include "glcdfont.c"

// replaced with setFontScale(), which is more meaningful
//void FishinoGFX::setTextSize(uint8_t s)
void FishinoGFX::setFontScale(uint8_t s)
{
	_fontScale = (s > 0) ? s : 1;
}

void FishinoGFX::setTextColor(uint16_t c)
{
	// For 'transparent' background, we'll set the bg
	// to the same as fg instead of using a flag
	_textColor = _textBgColor = c;
}

void FishinoGFX::setTextColor(uint16_t c, uint16_t b)
{
	_textColor   = c;
	_textBgColor = b;
}

void FishinoGFX::setTextWrap(boolean w)
{
	_textWrap = w;
}

// Enable (or disable) Code Page 437-compatible charset.
// There was an error in glcdfont.c for the longest time -- one character
// (#176, the 'light shade' block) was missing -- this threw off the index
// of every character that followed it.  But a TON of code has been written
// with the erroneous character indices.  By default, the library uses the
// original 'wrong' behavior and old sketches will still work.  Pass 'true'
// to this function to use correct CP437 character values in your code.
void FishinoGFX::cp437(boolean x)
{
	_cp437 = x;
}

// sets an embedded font from Fonts folder
// beware, memory hungry!
void FishinoGFX::setFont(const GFXfont *f)
{
/*
	if (f)           // Font struct pointer passed in?
	{
		if (!_gfxFont)  // And no current font struct?
		{
			// Switching from classic to new font behavior.
			// Move cursor pos down 6 pixels so it's on baseline.
			_cursorY += 6;
		}
	}
	else if (_gfxFont)  // NULL passed.  Current font struct defined?
	{
		// Switching from new to classic font behavior.
		// Move cursor pos up 6 pixels so it's at top-left of char.
		_cursorY -= 6;
	}
*/
	_gfxFont = (GFXfont *)f;
}

// this one is foreseen for drivers with multiple internal fonts
// on normal drivers just resets to internal font
void FishinoGFX::setFontIndex(uint8_t fontIdx)
{
	setFont();
}

// Pass string and a cursor position, returns UL corner and W,H.
void FishinoGFX::getTextBounds(const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w  = *h = 0;

	if (_gfxFont)
	{

		GFXglyph *glyph;
		uint8_t first = pgm_read_byte(&_gfxFont->first);
		uint8_t last  = pgm_read_byte(&_gfxFont->last);
		uint8_t gw, gh, xa;
		int8_t xo, yo;
		int16_t minx = _width;
		int16_t miny = _height;
		int16_t maxx = -1;
		int16_t maxy = -1;
		int16_t gx1, gy1, gx2, gy2;
		int16_t ts = (int16_t)_fontScale;
		int16_t ya = ts * (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);

		while ((c = *str++))
		{
			if (c != '\n')  // Not a newline
			{
				if (c != '\r')  // Not a carriage return, is normal char
				{
					if ((c >= first) && (c <= last))  // Char present in current font
					{
						c    -= first;
						glyph = &(((GFXglyph *)pgm_read_pointer(&_gfxFont->glyph))[c]);
						gw    = pgm_read_byte(&glyph->width);
						gh    = pgm_read_byte(&glyph->height);
						xa    = pgm_read_byte(&glyph->xAdvance);
						xo    = pgm_read_byte(&glyph->xOffset);
						yo    = pgm_read_byte(&glyph->yOffset);
						if (_textWrap && ((x + (((int16_t)xo + gw) * ts)) >= (int16_t)_width))
						{
							// Line wrap
							x  = 0;  // Reset x to 0
							y += ya; // Advance y by 1 line
						}
						gx1 = x   + xo * ts;
						gy1 = y   + yo * ts;
						gx2 = gx1 + gw * ts - 1;
						gy2 = gy1 + gh * ts - 1;
						if (gx1 < minx)
							minx = gx1;
						if (gy1 < miny)
							miny = gy1;
						if (gx2 > maxx)
							maxx = gx2;
						if (gy2 > maxy)
							maxy = gy2;
						x += xa * ts;
					}
				} // Carriage return = do nothing
			}
			else   // Newline
			{
				x  = 0;  // Reset x
				y += ya; // Advance y by 1 line
			}
		}
		// End of string
		*x1 = minx;
		*y1 = miny;
		if (maxx >= minx)
			*w  = maxx - minx + 1;
		if (maxy >= miny)
			*h  = maxy - miny + 1;

	}
	else   // Default font
	{

		uint16_t lineWidth = 0;
		uint16_t maxWidth = 0; // Width of current, all lines

		while ((c = *str++))
		{
			if (c != '\n')  // Not a newline
			{
				if (c != '\r')  // Not a carriage return, is normal char
				{
					if (_textWrap && ((x + _fontScale * 6) >= (int16_t)_width))
					{
						x  = 0;            // Reset x to 0
						y += _fontScale * 8; // Advance y by 1 line
						if (lineWidth > maxWidth)
							maxWidth = lineWidth; // Save widest line
						lineWidth = _fontScale * 6; // First char on new line
					}
					else   // No line wrap, just keep incrementing X
					{
						lineWidth += _fontScale * 6; // Includes interchar x gap
					}
				} // Carriage return = do nothing
			}
			else   // Newline
			{
				x  = 0;            // Reset x to 0
				y += _fontScale * 8; // Advance y by 1 line
				if (lineWidth > maxWidth)
					maxWidth = lineWidth; // Save widest line
				lineWidth = 0;     // Reset lineWidth for new line
			}
		}
		// End of string
		if (lineWidth)
			y += _fontScale * 8; // Add height of last (or only) line
		if (lineWidth > maxWidth)
			maxWidth = lineWidth; // Is the last or only line the widest?
		*w = maxWidth - 1;               // Don't include last interchar x gap
		*h = y - *y1;

	} // End classic vs custom font
}

// Same as above, but for PROGMEM strings
void FishinoGFX::getTextBounds(const __FlashStringHelper *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
	uint8_t *s = (uint8_t *)str, c;

	*x1 = x;
	*y1 = y;
	*w  = *h = 0;

	if (_gfxFont)
	{

		GFXglyph *glyph;
		uint8_t first = pgm_read_byte(&_gfxFont->first);
		uint8_t last  = pgm_read_byte(&_gfxFont->last);
		uint8_t gw, gh, xa;
		int8_t    xo, yo;
		int16_t minx = _width;
		int16_t miny = _height;
		int16_t maxx = -1;
		int16_t maxy = -1;
		int16_t gx1, gy1, gx2, gy2;
		int16_t ts = (int16_t)_fontScale;
		int16_t ya = ts * (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);

		while ((c = pgm_read_byte(s++)))
		{
			if (c != '\n')  // Not a newline
			{
				if (c != '\r')  // Not a carriage return, is normal char
				{
					if ((c >= first) && (c <= last))  // Char present in current font
					{
						c    -= first;
						glyph = &(((GFXglyph *)pgm_read_pointer(&_gfxFont->glyph))[c]);
						gw    = pgm_read_byte(&glyph->width);
						gh    = pgm_read_byte(&glyph->height);
						xa    = pgm_read_byte(&glyph->xAdvance);
						xo    = pgm_read_byte(&glyph->xOffset);
						yo    = pgm_read_byte(&glyph->yOffset);
						if (_textWrap && ((x + (((int16_t)xo + gw) * ts)) >= (int16_t)_width))
						{
							// Line wrap
							x  = 0;  // Reset x to 0
							y += ya; // Advance y by 1 line
						}
						gx1 = x   + xo * ts;
						gy1 = y   + yo * ts;
						gx2 = gx1 + gw * ts - 1;
						gy2 = gy1 + gh * ts - 1;
						if (gx1 < minx)
							minx = gx1;
						if (gy1 < miny)
							miny = gy1;
						if (gx2 > maxx)
							maxx = gx2;
						if (gy2 > maxy)
							maxy = gy2;
						x += xa * ts;
					}
				} // Carriage return = do nothing
			}
			else   // Newline
			{
				x  = 0;  // Reset x
				y += ya; // Advance y by 1 line
			}
		}
		// End of string
		*x1 = minx;
		*y1 = miny;
		if (maxx >= minx)
			*w  = maxx - minx + 1;
		if (maxy >= miny)
			*h  = maxy - miny + 1;
	}
	else   // Default font
	{

		uint16_t lineWidth = 0;
		uint16_t maxWidth = 0; // Width of current, all lines

		while ((c = pgm_read_byte(s++)))
		{
			if (c != '\n')  // Not a newline
			{
				if (c != '\r')  // Not a carriage return, is normal char
				{
					if (_textWrap && ((x + _fontScale * 6) >= (int16_t)_width))
					{
						x  = 0;            // Reset x to 0
						y += _fontScale * 8; // Advance y by 1 line
						if (lineWidth > maxWidth)
							maxWidth = lineWidth; // Save widest line
						lineWidth = _fontScale * 6; // First char on new line
					}
					else   // No line wrap, just keep incrementing X
					{
						lineWidth += _fontScale * 6; // Includes interchar x gap
					}
				} // Carriage return = do nothing
			}
			else   // Newline
			{
				x  = 0;            // Reset x to 0
				y += _fontScale * 8; // Advance y by 1 line
				if (lineWidth > maxWidth)
					maxWidth = lineWidth; // Save widest line
				lineWidth = 0;     // Reset lineWidth for new line
			}
		}
		// End of string
		if (lineWidth)
			y += _fontScale * 8; // Add height of last (or only) line
		if (lineWidth > maxWidth)
			maxWidth = lineWidth; // Is the last or only line the widest?
		*w = maxWidth - 1;               // Don't include last interchar x gap
		*h = y - *y1;

	} // End classic vs custom font
}

size_t FishinoGFX::textWrite(uint8_t c)
{
	int16_t x, y;
/*
	x = _cursorX;
	y = _cursorY;
*/
	getCursor0(x, y);
	
	if (!_gfxFont)  // 'Classic' built-in font
	{
		if (c == '\n')
		{
			x  = 0;
			y += _fontScale*8;
		}
		else if (c == '\r')
		{
			// skip em
		}
		else
		{
			if (_textWrap && ((x + _fontScale * 6) >= (int16_t)_width))  // Heading off edge?
			{
				// Reset x to zero
				x  = 0;
				// Advance y one line
				y += _fontScale * 8;
			}
			drawChar0(x, y, c, _textColor, _textBgColor, _fontScale);
			x += _fontScale * 6;
		}

	}
	else   // Custom font
	{

		if (c == '\n')
		{
			x  = 0;
			y += (int16_t)_fontScale * (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);
		}
		else if (c != '\r')
		{
			uint8_t first = pgm_read_byte(&_gfxFont->first);
			if ((c >= first) && (c <= (uint8_t)pgm_read_byte(&_gfxFont->last)))
			{
				uint8_t c2 = c - pgm_read_byte(&_gfxFont->first);
				GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&_gfxFont->glyph))[c2]);
				uint8_t w = pgm_read_byte(&glyph->width);
				uint8_t h     = pgm_read_byte(&glyph->height);
				// Is there an associated bitmap?
				if ((w > 0) && (h > 0))
				{
					// sic
					int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
					if (_textWrap && ((x + _fontScale * (xo + w)) >= (int16_t)_width))
					{
						// Drawing character would go off right edge; wrap to new line
						x  = 0;
						y += (int16_t)_fontScale * (uint8_t)pgm_read_byte(&_gfxFont->yAdvance);
					}
					drawChar0(x, y, c, _textColor, _textBgColor, _fontScale);
				}
				x += pgm_read_byte(&glyph->xAdvance) * (int16_t)_fontScale;
			}
		}

	}
/*
	_cursorX = x;
	_cursorY = y;
*/

	setCursor0(x, y);
	return 1;
}

size_t FishinoGFX::textWrite(const char *buf, size_t len)
{
	if(len == 0)
		len = strlen(buf);
	for(size_t i = 0; i < len; i++)
		textWrite(buf[i]);
	return len;
}


// Draw a character
void FishinoGFX::drawChar0(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size)
{
	if (!_gfxFont)  // 'Classic' built-in font
	{

		if ((x >= (int16_t)_width)		||	// Clip right
			(y >= (int16_t)_height)		||	// Clip bottom
			((x + 6 * size - 1) < 0)	||	// Clip left
			((y + 8 * size - 1) < 0)	//	Clip top
		)
			return;

		// Handle 'classic' charset behavior
		if (!_cp437 && (c >= 176))
			c++;

		for (int8_t i = 0; i < 6; i++)
		{
			uint8_t line;
			if (i < 5)
				line = pgm_read_byte(font+(c*5)+i);
			else
				line = 0x0;
			for (int8_t j = 0; j < 8; j++, line >>= 1)
			{
				if (line & 0x1)
				{
					if (size == 1)
						drawPixel0(x + i, y + j, color);
					else
						fillRect0( x + ( i * size), y + ( j * size), size, size, color);
				}
				else if (bg != color)
				{
					if (size == 1)
						drawPixel0(x+i, y+j, bg);
					else
						fillRect0(x + i * size, y + j * size, size, size, bg);
				}
			}
		}

	}
	else   // Custom font
	{

		// Character is assumed previously filtered by write() to eliminate
		// newlines, returns, non-printable characters, etc.  Calling drawChar()
		// directly with 'bad' characters of font may cause mayhem!

		c -= pgm_read_byte(&_gfxFont->first);
		GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&_gfxFont->glyph))[c]);
		uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&_gfxFont->bitmap);

		uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
		uint8_t w  = pgm_read_byte(&glyph->width);
		uint8_t h  = pgm_read_byte(&glyph->height);
//		uint8_t xa = pgm_read_byte(&glyph->xAdvance);
		int8_t xo = pgm_read_byte(&glyph->xOffset);
		int8_t yo = pgm_read_byte(&glyph->yOffset);
		uint8_t xx, yy;
		uint8_t bits = 0;
		uint8_t bit = 0;
		int16_t xo16 = 0;
		int16_t yo16 = 0;

		if (size > 1)
		{
			xo16 = xo;
			yo16 = yo;
		}

		// Todo: Add character clipping here

		// NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
		// THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
		// has typically been used with the 'classic' font to overwrite old
		// screen contents with new data.  This ONLY works because the
		// characters are a uniform size; it's not a sensible thing to do with
		// proportionally-spaced fonts with glyphs of varying sizes (and that
		// may overlap).  To replace previously-drawn text when using a custom
		// font, use the getTextBounds() function to determine the smallest
		// rectangle encompassing a string, erase the area with fillRect(),
		// then draw new text.  This WILL infortunately 'blink' the text, but
		// is unavoidable.  Drawing 'background' pixels will NOT fix this,
		// only creates a new set of problems.  Have an idea to work around
		// this (a canvas object type for MCUs that can afford the RAM and
		// displays supporting setAddrWindow() and pushColors()), but haven't
		// implemented this yet.

		for (yy=0; yy<h; yy++)
		{
			for (xx=0; xx<w; xx++)
			{
				if (!(bit++ & 7))
					bits = pgm_read_byte(&bitmap[bo++]);
				if (bits & 0x80)
				{
					if (size == 1)
						drawPixel0(x + xo + xx, y + yo + yy, color);
					else
						fillRect0(x + (xo16 + xx) * size, y + (yo16 + yy) * size, size, size, color);
				}
				bits <<= 1;
			}
		}

	} // End classic vs custom font
}
