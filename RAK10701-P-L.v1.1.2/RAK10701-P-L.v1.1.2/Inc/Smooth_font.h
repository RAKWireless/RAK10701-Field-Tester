
#ifndef __SMOOTH_FONT_H__
#define __SMOOTH_FONT_H__

#include <Arduino.h>
#include "../Inc/tft.h"

#undef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// This is for the whole font
typedef struct
{
	const uint8_t *gArray; // array start pointer
	uint16_t gCount;	   // Total number of characters
	uint16_t yAdvance;	   // Line advance
	uint16_t spaceWidth;   // Width of a space character
	int16_t ascent;		   // Height of top of 'd' above baseline, other characters may be taller
	int16_t descent;	   // Offset to bottom of 'p', other characters may have a larger descent
	uint16_t maxAscent;	   // Maximum ascent found in font
	uint16_t maxDescent;   // Maximum descent found in font
} fontMetrics;

/*
 * @brief Font alignment.
 */
typedef enum
{
	LEFT_ALIGNED,
	RIGHT_ALIGNED,
	CENTER_ALIGNED,
} aligned_t;

// These are for the new antialiased fonts
void sf_setCursor(int16_t x, int16_t y);
void sf_setTextWrap(bool wrapX, bool wrapY);
size_t sf_writeStr(const char *utf8, uint8_t len, uint8_t align);
size_t sf_write(uint8_t utf8);
void sf_setTextColor(uint16_t c, uint16_t b, bool fill);

void sf_loadFont(const uint8_t array[]);
void sf_loadArray(String fontName, bool flash);
void sf_unloadFont(void);
bool sf_getUnicodeIndex(uint16_t unicode, uint16_t *index);

void sf_drawGlyph(uint16_t code);

#endif
