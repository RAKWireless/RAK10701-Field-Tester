#include "../Inc/Smooth_font.h"

static fontMetrics gFont = {nullptr, 0, 0, 0, 0, 0, 0, 0};

// These are for the metrics for each individual glyph (so we don't need to seek this in file and waste time)
static uint16_t *gUnicode = NULL; // UTF-16 code, the codes are searched so do not need to be sequential
static uint8_t *gHeight = NULL;	  // cheight
static uint8_t *gWidth = NULL;	  // cwidth
static uint8_t *gxAdvance = NULL; // setWidth
static int16_t *gdY = NULL;		  // topExtent
static int8_t *gdX = NULL;		  // leftExtent
static uint32_t *gBitmap = NULL;  // file pointer to greyscale bitmap

static bool fontLoaded = false; // Flags when a anti-aliased font is loaded
static bool fontFile = true;
static uint8_t *fontPtr = nullptr;

static uint32_t textcolor = 0;
static uint32_t textbgcolor = 0; // Text foreground and background colours
static bool _fillbg;			 // Fill background flag (just for for smooth fonts at the moment)

static int32_t last_cursor_x;			 // Previous text cursor position when fill used
static int32_t bg_cursor_x;				 // Background fill cursor
static uint8_t decoderState = 0;		 // UTF8 decoder state        - not for user access
static uint16_t decoderBuffer;			 // Unicode code-point buffer - not for user access
static int32_t cursor_x, cursor_y, padX; // Text cursor x,y and padding setting
static bool textwrapX, textwrapY;		 // If set, 'wrap' text at right and optionally bottom edge of display

static int32_t _width, _height; // Display w/h as modified by current rotation

void sf_loadMetrics(void);
uint32_t sf_readInt32(void);

/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Serial UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t sf_decodeUTF8(uint8_t c)
{

	// 7 bit Unicode Code Point
	if ((c & 0x80) == 0x00)
	{
		decoderState = 0;
		return c;
	}

	if (decoderState == 0)
	{
		// 11 bit Unicode Code Point
		if ((c & 0xE0) == 0xC0)
		{
			decoderBuffer = ((c & 0x1F) << 6);
			decoderState = 1;
			return 0;
		}
		// 16 bit Unicode Code Point
		if ((c & 0xF0) == 0xE0)
		{
			decoderBuffer = ((c & 0x0F) << 12);
			decoderState = 2;
			return 0;
		}
		// 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
		// if ((c & 0xF8) == 0xF0) return c;
	}
	else
	{
		if (decoderState == 2)
		{
			decoderBuffer |= ((c & 0x3F) << 6);
			decoderState--;
			return 0;
		}
		else
		{
			decoderBuffer |= (c & 0x3F);
			decoderState = 0;
			return decoderBuffer;
		}
	}

	decoderState = 0;

	return c; // fall-back to extended ASCII
}

/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t sf_writeStr(const char *utf8, uint8_t len, uint8_t align)
{
	uint8_t *date = (uint8_t *)utf8;
	uint8_t subLen = strlen(utf8);
	if (align == LEFT_ALIGNED)
	{
		for (uint8_t i = 0; i < subLen; i++)
		{
			sf_write(date[i]);
		}
		if (len > subLen)
		{
			for (uint8_t i = 0; i < (len - subLen); i++)
			{
				sf_write(' ');
				sf_write(' ');
			}
		}
	}
	else if (align == RIGHT_ALIGNED)
	{
		if (len > subLen)
		{
			for (uint8_t i = 0; i < (len - subLen); i++)
			{
				sf_write(' ');
				sf_write(' ');
			}
		}
		for (uint8_t i = 0; i < subLen; i++)
		{
			sf_write(date[i]);
		}
	}
	else if (align == CENTER_ALIGNED)
	{
		if (len > subLen)
		{
			for (uint8_t i = 0; i < (len - subLen); i++)
			{
				sf_write(' ');
			}
			for (uint8_t i = 0; i < subLen; i++)
			{
				sf_write(date[i]);
			}
			for (uint8_t i = 0; i < (len - subLen); i++)
			{
				sf_write(' ');
			}
		}
		else
		{
			for (uint8_t i = 0; i < subLen; i++)
			{
				sf_write(date[i]);
			}
		}
	}
	return 1;
}

/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t sf_write(uint8_t utf8)
{
	uint16_t uniCode = sf_decodeUTF8(utf8);

	if (!uniCode)
		return 1;

	if (utf8 == '\r')
		return 1;

	if (fontLoaded)
	{
		if (uniCode < 32 && utf8 != '\n')
			return 1;

		sf_drawGlyph(uniCode);
		return 1;
	}
	return 0;
}

/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position
***************************************************************************************/
void sf_setCursor(int16_t x, int16_t y)
{
	cursor_x = x;
	cursor_y = y;
}

/***************************************************************************************
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void sf_setTextWrap(bool wrapX, bool wrapY)
{
	textwrapX = wrapX;
	textwrapY = wrapY;
}

/***************************************************************************************
** Function name:           loadFont
** Description:             loads parameters from a font vlw array in memory
*************************************************************************************x*/
void sf_loadFont(const uint8_t array[])
{
	if (array == nullptr)
		return;
	fontPtr = (uint8_t *)array;
	sf_loadArray("", false);
	last_cursor_x = 0;
	bg_cursor_x = 0;
	cursor_x = 0;
	cursor_y = 0;
	_width = tft.width();
	_height = tft.height();
}

/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
// Smooth fonts use the background colour for anti-aliasing and by default the
// background is not filled. If bgfill = true, then a smooth font background fill will
// be used.
void sf_setTextColor(uint16_t c, uint16_t b, bool fill)
{
	textcolor = c;
	textbgcolor = b;
	_fillbg = true;
}

/***************************************************************************************
** Function name:           loadFont
** Description:             loads parameters from a font vlw file
*************************************************************************************x*/
void sf_loadArray(String fontName, bool flash)
{
	/*
	  The vlw font format does not appear to be documented anywhere, so some reverse
	  engineering has been applied!

	  Header of vlw file comprises 6 uint32_t parameters (24 bytes total):
		1. The gCount (number of character glyphs)
		2. A version number (0xB = 11 for the one I am using)
		3. The font size (in points, not pixels)
		4. Deprecated mboxY parameter (typically set to 0)
		5. Ascent in pixels from baseline to top of "d"
		6. Descent in pixels from baseline to bottom of "p"

	  Next are gCount sets of values for each glyph, each set comprises 7 int32t parameters (28 bytes):
		1. Glyph Unicode stored as a 32 bit value
		2. Height of bitmap bounding box
		3. Width of bitmap bounding box
		4. gxAdvance for cursor (setWidth in Processing)
		5. dY = distance from cursor baseline to top of glyph bitmap (signed value +ve = up)
		6. dX = distance from cursor to left side of glyph bitmap (signed value -ve = left)
		7. padding value, typically 0

	  The bitmaps start next at 24 + (28 * gCount) bytes from the start of the file.
	  Each pixel is 1 byte, an 8 bit Alpha value which represents the transparency from
	  0xFF foreground colour, 0x00 background. The library uses a linear interpolation
	  between the foreground and background RGB component colours. e.g.
		  pixelRed = ((fgRed * alpha) + (bgRed * (255 - alpha))/255
	  To gain a performance advantage fixed point arithmetic is used with rounding and
	  division by 256 (shift right 8 bits is faster).

	  After the bitmaps is:
		 1 byte for font name string length (excludes null)
		 a zero terminated character string giving the font name
		 1 byte for Postscript name string length
		 a zero/one terminated character string giving the font name
		 last byte is 0 for non-anti-aliased and 1 for anti-aliased (smoothed)


	  Glyph bitmap example is:
	  // Cursor coordinate positions for this and next character are marked by 'C'
	  // C<------- gxAdvance ------->C  gxAdvance is how far to move cursor for next glyph cursor position
	  // |                           |
	  // |                           |   ascent is top of "d", descent is bottom of "p"
	  // +-- gdX --+             ascent
	  // |         +-- gWidth--+     |   gdX is offset to left edge of glyph bitmap
	  // |   +     x@.........@x  +  |   gdX may be negative e.g. italic "y" tail extending to left of
	  // |   |     @@.........@@  |  |   cursor position, plot top left corner of bitmap at (cursorX + gdX)
	  // |   |     @@.........@@ gdY |   gWidth and gHeight are glyph bitmap dimensions
	  // |   |     .@@@.....@@@@  |  |
	  // | gHeight ....@@@@@..@@  +  +    <-- baseline
	  // |   |     ...........@@     |
	  // |   |     ...........@@     |   gdY is the offset to the top edge of the bitmap
	  // |   |     .@@.......@@. descent plot top edge of bitmap at (cursorY + ascent - gdY)
	  // |   +     x..@@@@@@@..x     |   x marks the corner pixels of the bitmap
	  // |                           |
	  // +---------------------------+   yAdvance is y delta for the next line, font size or (ascent + descent)
	  //                                 some fonts can overlay in y direction so may need a user adjust value

	*/

	if (fontLoaded)
		sf_unloadFont();

	gFont.gArray = (const uint8_t *)fontPtr;

	gFont.gCount = (uint16_t)sf_readInt32();   // glyph count in file
	sf_readInt32();							   // vlw encoder version - discard
	gFont.yAdvance = (uint16_t)sf_readInt32(); // Font size in points, not pixels
	sf_readInt32();							   // discard
	gFont.ascent = (uint16_t)sf_readInt32();   // top of "d"
	gFont.descent = (uint16_t)sf_readInt32();  // bottom of "p"

	// These next gFont values might be updated when the Metrics are fetched
	gFont.maxAscent = gFont.ascent;	  // Determined from metrics
	gFont.maxDescent = gFont.descent; // Determined from metrics
	gFont.yAdvance = gFont.ascent + gFont.descent;
	gFont.spaceWidth = gFont.yAdvance / 4; // Guess at space width

	fontLoaded = true;

	// Fetch the metrics for each glyph
	sf_loadMetrics();
}

/***************************************************************************************
** Function name:           loadMetrics
** Description:             Get the metrics for each glyph and store in RAM
*************************************************************************************x*/
// #define SHOW_ASCENT_DESCENT
void sf_loadMetrics(void)
{
	uint32_t headerPtr = 24;
	uint32_t bitmapPtr = headerPtr + gFont.gCount * 28;
	gUnicode = (uint16_t *)malloc(gFont.gCount * 2); // Unicode 16 bit Basic Multilingual Plane (0-FFFF)
	gHeight = (uint8_t *)malloc(gFont.gCount);		 // Height of glyph
	gWidth = (uint8_t *)malloc(gFont.gCount);		 // Width of glyph
	gxAdvance = (uint8_t *)malloc(gFont.gCount);	 // xAdvance - to move x cursor
	gdY = (int16_t *)malloc(gFont.gCount * 2);		 // offset from bitmap top edge from lowest point in any character
	gdX = (int8_t *)malloc(gFont.gCount);			 // offset for bitmap left edge relative to cursor X
	gBitmap = (uint32_t *)malloc(gFont.gCount * 4);	 // seek pointer to glyph bitmap in the file

	uint16_t gNum = 0;

	while (gNum < gFont.gCount)
	{
		gUnicode[gNum] = (uint16_t)sf_readInt32(); // Unicode code point value
		gHeight[gNum] = (uint8_t)sf_readInt32();   // Height of glyph
		gWidth[gNum] = (uint8_t)sf_readInt32();	   // Width of glyph
		gxAdvance[gNum] = (uint8_t)sf_readInt32(); // xAdvance - to move x cursor
		gdY[gNum] = (int16_t)sf_readInt32();	   // y delta from baseline
		gdX[gNum] = (int8_t)sf_readInt32();		   // x delta from cursor
		sf_readInt32();							   // ignored

		// Serial.print("Unicode = 0x"); Serial.print(gUnicode[gNum], HEX); Serial.print(", gHeight  = "); Serial.println(gHeight[gNum]);
		// Serial.print("Unicode = 0x"); Serial.print(gUnicode[gNum], HEX); Serial.print(", gWidth  = "); Serial.println(gWidth[gNum]);
		// Serial.print("Unicode = 0x"); Serial.print(gUnicode[gNum], HEX); Serial.print(", gxAdvance  = "); Serial.println(gxAdvance[gNum]);
		// Serial.print("Unicode = 0x"); Serial.print(gUnicode[gNum], HEX); Serial.print(", gdY  = "); Serial.println(gdY[gNum]);

		// Different glyph sets have different ascent values not always based on "d", so we could get
		// the maximum glyph ascent by checking all characters. BUT this method can generate bad values
		// for non-existent glyphs, so we will reply on processing for the value and disable this code for now...
		/*
		if (gdY[gNum] > gFont.maxAscent)
		{
		  // Try to avoid UTF coding values and characters that tend to give duff values
		  if (((gUnicode[gNum] > 0x20) && (gUnicode[gNum] < 0x7F)) || (gUnicode[gNum] > 0xA0))
		  {
			gFont.maxAscent   = gdY[gNum];
	#ifdef SHOW_ASCENT_DESCENT
			Serial.print("Unicode = 0x"); Serial.print(gUnicode[gNum], HEX); Serial.print(", maxAscent  = "); Serial.println(gFont.maxAscent);
	#endif
		  }
		}
		*/

		// Different glyph sets have different descent values not always based on "p", so get maximum glyph descent
		if (((int16_t)gHeight[gNum] - (int16_t)gdY[gNum]) > gFont.maxDescent)
		{
			// Avoid UTF coding values and characters that tend to give duff values
			if (((gUnicode[gNum] > 0x20) && (gUnicode[gNum] < 0xA0) && (gUnicode[gNum] != 0x7F)) || (gUnicode[gNum] > 0xFF))
			{
				gFont.maxDescent = gHeight[gNum] - gdY[gNum];
			}
		}

		gBitmap[gNum] = bitmapPtr;

		bitmapPtr += gWidth[gNum] * gHeight[gNum];

		gNum++;
		yield();
	}

	gFont.yAdvance = gFont.maxAscent + gFont.maxDescent;

	gFont.spaceWidth = (gFont.ascent + gFont.descent) * 2 / 7; // Guess at space width
}

/***************************************************************************************
** Function name:           deleteMetrics
** Description:             Delete the old glyph metrics and free up the memory
*************************************************************************************x*/
void sf_unloadFont(void)
{
	if (gUnicode)
	{
		free(gUnicode);
		gUnicode = NULL;
	}

	if (gHeight)
	{
		free(gHeight);
		gHeight = NULL;
	}

	if (gWidth)
	{
		free(gWidth);
		gWidth = NULL;
	}

	if (gxAdvance)
	{
		free(gxAdvance);
		gxAdvance = NULL;
	}

	if (gdY)
	{
		free(gdY);
		gdY = NULL;
	}

	if (gdX)
	{
		free(gdX);
		gdX = NULL;
	}

	if (gBitmap)
	{
		free(gBitmap);
		gBitmap = NULL;
	}

	gFont.gArray = nullptr;

	fontLoaded = false;
}

/***************************************************************************************
** Function name:           readInt32
** Description:             Get a 32 bit integer from the font file
*************************************************************************************x*/
uint32_t sf_readInt32(void)
{
	uint32_t val = 0;

	val = pgm_read_byte(fontPtr++) << 24;
	val |= pgm_read_byte(fontPtr++) << 16;
	val |= pgm_read_byte(fontPtr++) << 8;
	val |= pgm_read_byte(fontPtr++);

	return val;
}

/***************************************************************************************
** Function name:           getUnicodeIndex
** Description:             Get the font file index of a Unicode character
*************************************************************************************x*/
bool sf_getUnicodeIndex(uint16_t unicode, uint16_t *index)
{
	for (uint16_t i = 0; i < gFont.gCount; i++)
	{
		if (gUnicode[i] == unicode)
		{
			*index = i;
			return true;
		}
	}
	return false;
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 16bit foreground and background
*************************************************************************************x*/
uint16_t sf_alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
{
	// For speed use fixed point maths and rounding to permit a power of 2 division
	uint16_t fgR = ((fgc >> 10) & 0x3E) + 1;
	uint16_t fgG = ((fgc >> 4) & 0x7E) + 1;
	uint16_t fgB = ((fgc << 1) & 0x3E) + 1;

	uint16_t bgR = ((bgc >> 10) & 0x3E) + 1;
	uint16_t bgG = ((bgc >> 4) & 0x7E) + 1;
	uint16_t bgB = ((bgc << 1) & 0x3E) + 1;

	// Shift right 1 to drop rounding bit and shift right 8 to divide by 256
	uint16_t r = (((fgR * alpha) + (bgR * (255 - alpha))) >> 9);
	uint16_t g = (((fgG * alpha) + (bgG * (255 - alpha))) >> 9);
	uint16_t b = (((fgB * alpha) + (bgB * (255 - alpha))) >> 9);

	// Combine RGB565 colours into 16 bits
	// return ((r&0x18) << 11) | ((g&0x30) << 5) | ((b&0x18) << 0); // 2 bit greyscale
	// return ((r&0x1E) << 11) | ((g&0x3C) << 5) | ((b&0x1E) << 0); // 4 bit greyscale
	return (r << 11) | (g << 5) | (b << 0);
}

/***************************************************************************************
** Function name:           drawGlyph
** Description:             Write a character to the TFT cursor position
*************************************************************************************x*/
// Expects file to be open
void sf_drawGlyph(uint16_t code)
{
	uint16_t fg = textcolor;
	uint16_t bg = textbgcolor;

	// Serial.printf("code = %d\r\n",code);

	// Check if cursor has moved
	if (last_cursor_x != cursor_x)
	{
		bg_cursor_x = cursor_x;
		last_cursor_x = cursor_x;
	}

	if (code < 0x21)
	{
		if (code == 0x20)
		{
			if (_fillbg)
				tft.fillRect(bg_cursor_x, cursor_y, (cursor_x + gFont.spaceWidth) - bg_cursor_x, gFont.yAdvance, bg);
			cursor_x += gFont.spaceWidth;
			bg_cursor_x = cursor_x;
			last_cursor_x = cursor_x;
			return;
		}

		if (code == '\n')
		{
			cursor_x = 0;
			bg_cursor_x = 0;
			last_cursor_x = 0;
			cursor_y += gFont.yAdvance;
			if (textwrapY && (cursor_y >= _height))
				cursor_y = 0;
			return;
		}
	}

	uint16_t gNum = 0;
	bool found = sf_getUnicodeIndex(code, &gNum);

	if (found)
	{

		if (textwrapX && (cursor_x + gWidth[gNum] + gdX[gNum] > _width))
		{
			cursor_y += gFont.yAdvance;
			cursor_x = 0;
			bg_cursor_x = 0;
		}
		if (textwrapY && ((cursor_y + gFont.yAdvance) >= _height))
			cursor_y = 0;
		if (cursor_x == 0)
			cursor_x -= gdX[gNum];

		uint8_t *pbuffer = nullptr;
		const uint8_t *gPtr = (const uint8_t *)gFont.gArray;

		int16_t cy = cursor_y + gFont.maxAscent - gdY[gNum];
		int16_t cx = cursor_x + gdX[gNum];

		//  if (cx > width() && bg_cursor_x > width()) return;
		//  if (cursor_y > height()) return;

		int16_t fxs = cx;
		uint32_t fl = 0;
		int16_t bxs = cx;
		uint32_t bl = 0;
		int16_t bx = 0;
		uint8_t pixel;

		// TODO
		// startWrite(); // Avoid slow ESP32 transaction overhead for every pixel
		//  tft.startWrite();

		int16_t fillwidth = 0;
		int16_t fillheight = 0;

		// Fill area above glyph
		//  if (_fillbg)
		//  {
		// fillwidth  = (cursor_x + gxAdvance[gNum]) - bg_cursor_x;
		// if (fillwidth > 0)
		// {
		// fillheight = gFont.maxAscent - gdY[gNum];
		// if (fillheight > 0)
		// {
		// tft.fillRect(bg_cursor_x, cursor_y, fillwidth, fillheight, textbgcolor);
		// }
		// }
		// else
		// {
		// // Could be negative
		// fillwidth = 0;
		// }

		// // Fill any area to left of glyph
		// if (bg_cursor_x < cx)
		// {
		// tft.fillRect(bg_cursor_x, cy, cx - bg_cursor_x, gHeight[gNum], textbgcolor);
		// }
		// // Set x position in glyph area where background starts
		// if (bg_cursor_x > cx)
		// {
		// bx = bg_cursor_x - cx;
		// }
		// // Fill any area to right of glyph
		// if (cx + gWidth[gNum] < cursor_x + gxAdvance[gNum])
		// {
		// tft.fillRect(cx + gWidth[gNum], cy, (cursor_x + gxAdvance[gNum]) - (cx + gWidth[gNum]), gHeight[gNum], textbgcolor);
		// }
		// }

		// Serial.printf("gdX[gNum] = %d , gxAdvance[gNum] = %d , cx = %d, cy = %d, gWidth[gNum] = %d , gHeight[gNum] = %d \r\n",gdX[gNum] , gxAdvance[gNum] ,cx,cy,gWidth[gNum],gHeight[gNum]);

		tft.fillRect(cursor_x, cursor_y, gxAdvance[gNum], gFont.descent + gFont.maxAscent, textbgcolor);
		tft.startWrite();
		tft.setAddrWindow(cx, cy, gWidth[gNum], gHeight[gNum]);
		for (int32_t y = 0; y < gHeight[gNum]; y++)
		{
			for (int32_t x = 0; x < gWidth[gNum]; x++)
			{
				pixel = pgm_read_byte(gPtr + gBitmap[gNum] + x + gWidth[gNum] * y);

				if (pixel)
				{
					if (bl)
					{
						tft.writeColor(bg, bl);
						// tft.drawFastHLine( bxs, y + cy, bl, bg);
						bl = 0;
					}
					if (pixel != 0xFF)
					{
						if (fl)
						{
							if (fl == 1)
								tft.writeColor(fg, 1);
							// tft.drawPixel(fxs, y + cy, fg);
							else
								tft.writeColor(fg, fl);
							// tft.drawFastHLine( fxs, y + cy, fl, fg);
							fl = 0;
						}
						//            if (getColor)
						//							bg = getColor(x + cx, y + cy);
						tft.writeColor(sf_alphaBlend(pixel, fg, bg), 1);
						// tft.drawPixel(x + cx, y + cy, sf_alphaBlend(pixel, fg, bg));
					}
					else
					{
						if (fl == 0)
							fxs = x + cx;
						fl++;
					}
				}
				else
				{
					if (fl)
					{
						tft.writeColor(fg, fl);
						fl = 0;
						// tft.drawFastHLine( fxs, y + cy, fl, fg); fl = 0;
					}
					// if (_fillbg)
					// {
					// if (x >= bx)
					// {
					// if (bl==0)
					// bxs = x + cx;
					// bl++;
					// }
					// }
					if (x >= bx)
					{
						if (bl == 0)
							bxs = x + cx;
						bl++;
					}
				}
			}
			if (fl)
			{
				tft.writeColor(fg, fl);
				// tft.drawFastHLine( fxs, y + cy, fl, fg);
				fl = 0;
			}
			if (bl)
			{
				tft.writeColor(bg, bl);
				// tft.drawFastHLine( bxs, y + cy, bl, bg);
				bl = 0;
			}
		}

		// Fill area below glyph
		if (fillwidth > 0)
		{
			fillheight = (cursor_y + gFont.yAdvance) - (cy + gHeight[gNum]);
			if (fillheight > 0)
			{
				tft.fillRect(bg_cursor_x, cy + gHeight[gNum], fillwidth, fillheight, textbgcolor);
			}
		}

		if (pbuffer)
			free(pbuffer);
		cursor_x += gxAdvance[gNum];
		tft.endWrite();
	}
	else
	{
		// Point code not in font so draw a rectangle and move on cursor
		tft.drawRect(cursor_x, cursor_y + gFont.maxAscent - gFont.ascent, gFont.spaceWidth, gFont.ascent, fg);
		cursor_x += gFont.spaceWidth + 1;
	}
	bg_cursor_x = cursor_x;
	last_cursor_x = cursor_x;
}

/***************************************************************************************
** Function name:           showFont
** Description:             Page through all characters in font, td ms between screens
*************************************************************************************x*/
// void showFont(uint32_t td)
// {
// if(!fontLoaded) return;

// int16_t cursorX = width(); // Force start of new page to initialise cursor
// int16_t cursorY = height();// for the first character
// uint32_t timeDelay = 0;    // No delay before first page

// fillScreen(textbgcolor);

// for (uint16_t i = 0; i < gFont.gCount; i++)
// {
// // Check if this will need a new screen
// if (cursorX + gdX[i] + gWidth[i] >= width())
// {
// cursorX = -gdX[i];

// cursorY += gFont.yAdvance;
// if (cursorY + gFont.maxAscent + gFont.descent >= height())
// {
// cursorX = -gdX[i];
// cursorY = 0;
// delay(timeDelay);
// timeDelay = td;
// fillScreen(textbgcolor);
// }
// }

// setCursor(cursorX, cursorY);
// drawGlyph(gUnicode[i]);
// cursorX += gxAdvance[i];
// yield();
// }

// delay(timeDelay);
// fillScreen(textbgcolor);
// }
