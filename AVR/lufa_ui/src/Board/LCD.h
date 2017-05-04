/*
 * LCD.h
 *
 * Created: 30.04.2017 19:01:05
 *  Author: Simon
 */


#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum _symbols {
	SYM_CDROM=0,
	SYM_USB,
	SYM_SDCARD,
	SYM_TUNER,
	SYM_CASETTE,
	SYM_RAND,
	SYM_PROG,
	SYM_PLAY,
	SYM_REP,
	SYM_ALL,
	SYM_ALBUM,
	SYM_STEREO,
	SYM_TIMER,
	SYM_REC,
	SYM_UBS,
	SYM_EQ_POP,
	SYM_EQ_CLASSIC,
	SYM_EQ_JAZZ,
	SYM_EQ_ROCK,
	SYM_RDS,
	SYM_SLEEP,
	SYM_PM,
	SYM_FM,
	SYM_MW,
	SYM_MHZ,
	SYM_KHZ,
	SYM_DOT,
	SYM_DOUBLEPOINT,
	SYM_MAX
} symbols_t;


void LCD_Init(void);

void LCD_SetSymbol(symbols_t symbol, bool enable);

void LCD_PutChar(char s, uint8_t pos);

void LCD_PutString(const char *str, uint8_t pos);

void LCD_PutString_P(const char *str, uint8_t pos);

void LCD_Clear(void);

void LCD_SetStandby(bool enable);

#endif /* LCD_H_ */