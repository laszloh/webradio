/*
 * LCD.c
 *
 * Created: 30.04.2017 19:00:52
 *  Author: Simon
 */

#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "LCD.h"

#include "Driver/pt6524.h"
#include "Driver/backlight.h"

#define LCD_SEGMENT_COUNT	156
#define reinterpret(T,V) ({union{T a; typeof(V) b;} __x__; __x__.b=(V); __x__.a; })

#define FIRST_FONT		0x20
#define LAST_FONT		0x7E

typedef struct _segdef {
	uint8_t msg:2;
	uint8_t bit:6;
} segdef_t;

static const segdef_t character1[] PROGMEM = {
	{.msg = 0, .bit = 7},			// a
	{.msg = 0, .bit = 14},			// b
	{.msg = 0, .bit = 13},			// c
	{.msg = 0, .bit = 12},			// d
	{.msg = 0, .bit = 0},			// e
	{.msg = 0, .bit = 3},			// f
	{.msg = 0, .bit = 1},			// g1
	{.msg = 0, .bit = 9},			// g2
	{.msg = 0, .bit = 2},			// h
	{.msg = 0, .bit = 6},			// i
	{.msg = 0, .bit = 10},			// j
	{.msg = 0, .bit = 4},			// k
	{.msg = 0, .bit = 5},			// l
	{.msg = 0, .bit = 8},			// m
};

static const segdef_t character2[] PROGMEM = {
	{.msg = 0, .bit = 23},			// a
	{.msg = 0, .bit = 30},			// b
	{.msg = 0, .bit = 29},			// c
	{.msg = 0, .bit = 28},			// d
	{.msg = 0, .bit = 16},			// e
	{.msg = 0, .bit = 19},			// f
	{.msg = 0, .bit = 17},			// g1
	{.msg = 0, .bit = 25},			// g2
	{.msg = 0, .bit = 18},			// h
	{.msg = 0, .bit = 22},			// i
	{.msg = 0, .bit = 26},			// j
	{.msg = 0, .bit = 20},			// k
	{.msg = 0, .bit = 21},			// l
	{.msg = 0, .bit = 24},			// m
};

static const segdef_t character3[] PROGMEM = {
	{.msg = 0, .bit = 39},			// a
	{.msg = 0, .bit = 46},			// b
	{.msg = 0, .bit = 45},			// c
	{.msg = 0, .bit = 44},			// d
	{.msg = 0, .bit = 32},			// e
	{.msg = 0, .bit = 35},			// f
	{.msg = 0, .bit = 33},			// g1
	{.msg = 0, .bit = 41},			// g2
	{.msg = 0, .bit = 34},			// h
	{.msg = 0, .bit = 38},			// i
	{.msg = 0, .bit = 42},			// j
	{.msg = 0, .bit = 36},			// k
	{.msg = 0, .bit = 37},			// l
	{.msg = 0, .bit = 40},			// m
};

static const segdef_t character4[] PROGMEM = {
	{.msg = 2, .bit = 3},			// a
	{.msg = 2, .bit = 10},			// b
	{.msg = 2, .bit = 9},			// c
	{.msg = 2, .bit = 8},			// d
	{.msg = 0, .bit = 48},			// e
	{.msg = 0, .bit = 51},			// f
	{.msg = 0, .bit = 49},			// g1
	{.msg = 2, .bit = 5},			// g2
	{.msg = 0, .bit = 50},			// h
	{.msg = 2, .bit = 2},			// i
	{.msg = 2, .bit = 6},			// j
	{.msg = 2, .bit = 0},			// k
	{.msg = 2, .bit = 1},			// l
	{.msg = 2, .bit = 4},			// m
};

static const segdef_t character5[] PROGMEM = {
	{.msg = 2, .bit = 51},			// a
	{.msg = 1, .bit = 2},			// b
	{.msg = 1, .bit = 1},			// c
	{.msg = 1, .bit = 0},			// d
	{.msg = 2, .bit = 44},			// e
	{.msg = 2, .bit = 47},			// f
	{.msg = 2, .bit = 45},			// g1
	{.msg = 2, .bit = 17},			// g2
	{.msg = 2, .bit = 46},			// h
	{.msg = 2, .bit = 50},			// i
	{.msg = 2, .bit = 18},			// j
	{.msg = 2, .bit = 48},			// k
	{.msg = 2, .bit = 49},			// l
	{.msg = 2, .bit = 16},			// m
};

static const segdef_t character6[] PROGMEM = {
	{.msg = 1, .bit = 11},			// a
	{.msg = 1, .bit = 14},			// b
	{.msg = 1, .bit = 13},			// c
	{.msg = 1, .bit = 12},			// d
	{.msg = 1, .bit = 4},			// e
	{.msg = 1, .bit = 7},			// f
	{.msg = 1, .bit = 5},			// g1
	{.msg = 2, .bit = 21},			// g2
	{.msg = 1, .bit = 6},			// h
	{.msg = 1, .bit = 10},			// i
	{.msg = 2, .bit = 22},			// j
	{.msg = 1, .bit = 8},			// k
	{.msg = 1, .bit = 9},			// l
	{.msg = 2, .bit = 20},			// m
};

static const segdef_t character7[] PROGMEM = {
	{.msg = 1, .bit = 23},			// a
	{.msg = 1, .bit = 26},			// b
	{.msg = 1, .bit = 25},			// c
	{.msg = 1, .bit = 24},			// d
	{.msg = 1, .bit = 16},			// e
	{.msg = 1, .bit = 19},			// f
	{.msg = 1, .bit = 17},			// g1
	{.msg = 2, .bit = 25},			// g2
	{.msg = 1, .bit = 18},			// h
	{.msg = 1, .bit = 22},			// i
	{.msg = 2, .bit = 26},			// j
	{.msg = 1, .bit = 20},			// k
	{.msg = 1, .bit = 21},			// l
	{.msg = 2, .bit = 24},			// m
};

static const segdef_t character8[] PROGMEM = {
	{.msg = 1, .bit = 35},			// a
	{.msg = 2, .bit = 34},			// b
	{.msg = 2, .bit = 33},			// c
	{.msg = 2, .bit = 32},			// d
	{.msg = 1, .bit = 28},			// e
	{.msg = 1, .bit = 31},			// f
	{.msg = 1, .bit = 29},			// g1
	{.msg = 2, .bit = 29},			// g2
	{.msg = 1, .bit = 30},			// h
	{.msg = 1, .bit = 34},			// i
	{.msg = 2, .bit = 30},			// j
	{.msg = 1, .bit = 32},			// k
	{.msg = 1, .bit = 33},			// l
	{.msg = 2, .bit = 28},			// m
};

static const segdef_t symbols[] PROGMEM = {
	{.msg = 0, .bit = 11},			// Kreis Punkt
	{.msg = 0, .bit = 15},			// USB
	{.msg = 0, .bit = 27},			// SD-Karte
	{.msg = 0, .bit = 31},			// Antenne
	{.msg = 0, .bit = 43},			// Kreis mit Pfeil
	{.msg = 0, .bit = 47},			// Kasette
	{.msg = 2, .bit = 7},			// Random
	{.msg = 2, .bit = 11},			// Prog
	{.msg = 2, .bit = 15},			// Play
	{.msg = 2, .bit = 19},			// Repeat
	{.msg = 2, .bit = 23},			// All
	{.msg = 2, .bit = 31},			// Album
	{.msg = 2, .bit = 27},			// zwei Kreise
	{.msg = 2, .bit = 35},			// Timer
	{.msg = 2, .bit = 37},			// Record
	{.msg = 2, .bit = 38},			// UBS
	{.msg = 2, .bit = 36},			// Pop
	{.msg = 2, .bit = 39},			// Jazz
	{.msg = 2, .bit = 40},			// Classic
	{.msg = 2, .bit = 43},			// Rock
	{.msg = 1, .bit = 3},			// R-D-S
	{.msg = 1, .bit = 15},			// Sleep
	{.msg = 1, .bit = 27},			// PM
	{.msg = 1, .bit = 36},			// FM
	{.msg = 1, .bit = 39},			// MW
	{.msg = 1, .bit = 37},			// MHz
	{.msg = 1, .bit = 38},			// kHz
	{.msg = 2, .bit = 13},			// dot
	{.msg = 2, .bit = 14},			// Doppelpunkt
};

static const segdef_t * const display[] PROGMEM = {
	character1,
	character2,
	character3,
	character4,
	character5,
	character6,
	character7,
	character8,
	symbols,
};

static const uint16_t chars[] PROGMEM = {
	0x0000,            // 0x20
	0x0000,            // 0x21
	0x0220,            // 0x22
	0x0000,            // 0x23
	0x12ED,            // 0x24
	0x2DE4,            // 0x25
	0x2B0D,            // 0x26
	0x0200,            // 0x27
	0x2400,            // 0x28
	0x0900,            // 0x29
	0x3FC0,            // 0x2A
	0x12C0,            // 0x2B
	0x0800,            // 0x2C
	0x00C0,            // 0x2D
	0x0000,            // 0x2E
	0x0C00,            // 0x2F
	0x0C3F,            // 0x30
	0x0006,            // 0x31
	0x00DB,            // 0x32
	0x00CF,            // 0x33
	0x00E6,            // 0x34
	0x00ED,            // 0x35
	0x00FD,            // 0x36
	0x0027,            // 0x37
	0x00FF,            // 0x38
	0x00EF,            // 0x39
	0x0000,            // 0x3A
	0x0000,            // 0x3B
	0x0C08,            // 0x3C
	0x00C8,            // 0x3D
	0x2108,            // 0x3E
	0x10A3,            // 0x3F
	0x10BF,            // 0x40
	0x00F7,            // 0x41
	0x128F,            // 0x42
	0x0039,            // 0x43
	0x120F,            // 0x44
	0x00F9,            // 0x45
	0x00F1,            // 0x46
	0x00BD,            // 0x47
	0x00F6,            // 0x48
	0x1209,            // 0x49
	0x001E,            // 0x4A
	0x3680,            // 0x4B
	0x0038,            // 0x4C
	0x1536,            // 0x4D
	0x2136,            // 0x4E
	0x003F,            // 0x4F
	0x00F3,            // 0x50
	0x203F,            // 0x51
	0x20F3,            // 0x52
	0x21ED,            // 0x53
	0x1201,            // 0x54
	0x003E,            // 0x55
	0x0C30,            // 0x56
	0x2A36,            // 0x57
	0x2D00,            // 0x58
	0x1500,            // 0x59
	0x0C09,            // 0x5A
	0x0000,            // 0x5B
	0x2100,            // 0x5C
	0x0000,            // 0x5D
	0x0120,            // 0x5E
	0x0008,            // 0x5F
	0x0100,            // 0x60
	0x00F7,            // 0x61
	0x12CF,            // 0x62
	0x0039,            // 0x63
	0x120F,            // 0x64
	0x00F9,            // 0x65
	0x00F1,            // 0x66
	0x00BD,            // 0x67
	0x00F6,            // 0x68
	0x1209,            // 0x69
	0x001E,            // 0x6A
	0x3680,            // 0x6B
	0x0038,            // 0x6C
	0x1536,            // 0x6D
	0x2136,            // 0x6E
	0x003F,            // 0x6F
	0x00F3,            // 0x70
	0x203F,            // 0x71
	0x20F3,            // 0x72
	0x21ED,            // 0x73
	0x1201,            // 0x74
	0x003E,            // 0x75
	0x0C30,            // 0x76
	0x2A36,            // 0x77
	0x2D00,            // 0x78
	0x1500,            // 0x79
	0x0C09,            // 0x7A
	0x0000,            // 0x7B
	0x1200,            // 0x7C
	0x0000,            // 0x7D
	0x3FFF,            // 0x7E
};

static uint64_t memory[3];

static inline void setSegment(segdef_t s, bool state);

void LCD_Init()
{
	pt6524_Init();
	backlight_Init();

	LCD_SetStandby(false);
}

void LCD_SetSymbol(symbols_t sym, bool enable)
{
	uint8_t d;
	if(sym >= SYM_MAX)
		return;

	d = pgm_read_byte(&(symbols[sym]));
	setSegment(reinterpret(segdef_t, d), enable);

	pt6524_write_raw(memory, sizeof(memory), LCD_SEGMENT_COUNT);
}

uint16_t LCD_PutChar(char c, uint8_t pos)
{
	uint8_t i;
	segdef_t *character;
	uint16_t font;

	if( (pos >= 8) || (c < FIRST_FONT) || (c > LAST_FONT) )
		return 1;

	font = pgm_read_word(&(chars[c - 0x20]));
	character = (segdef_t *)pgm_read_word(&(display[pos]));

	for(i=0;i<14;i++) {
		uint8_t d = pgm_read_byte(&(character[i]));
		setSegment(reinterpret(segdef_t, d), font & (1 << i) );
	}

	pt6524_write_raw(memory, sizeof(memory), LCD_SEGMENT_COUNT);
	return 0;
}

uint8_t LCD_PutString(const char *str, uint8_t pos)
{
	while(*str) {
		if(LCD_PutChar(*str, pos++))
			return 1;
		str++;
	}
	return 0;
}

uint8_t LCD_PutString_P(const char *str, uint8_t pos)
{
	char c = pgm_read_byte(str++);

	while(c) {
		if(LCD_PutChar(c, pos++))
			return 1;
		c = pgm_read_byte(str++);
	}
	return 0;
}

void LCD_Clear(void)
{
	memset(memory, 0, sizeof(memory));
	pt6524_write_raw(memory, sizeof(memory), LCD_SEGMENT_COUNT);
}

void LCD_SetBacklight(bool state)
{
    backlight_change(state);
}

void LCD_SetStandby(bool enable)
{
	backlight_change(enable);
	pt6524_set_standby(enable);
	pt6524_write_raw(memory, sizeof(memory), LCD_SEGMENT_COUNT);
}

static inline void setSegment(segdef_t s, bool state)
{
	if(state)
		memory[s.msg] |= (1ULL << s.bit);
	else
		memory[s.msg] &= ~(1ULL << s.bit);
}
