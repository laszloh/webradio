/*
 * IncFile1.h
 *
 * Created: 25.03.2017 22:12:51
 *  Author: Simon
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#define BACKLIGHT_DDR	DDRB
#define BACKLIGHT_PORT	PORTB
#define BACKLIGHT		PORTB1

inline void Backlight_Init(void) __attribute__((always_inline));
inline void Backlight_Init(void)
{
	BACKLIGHT_DDR |= _BV(BACKLIGHT);
}

inline void backlight_change(bool state) __attribute__((always_inline));
inline void backlight_change(bool state) 
{
	if(state)
		BACKLIGHT_PORT |= _BV(BACKLIGHT);
	else
		BACKLIGHT_PORT &= ~_BV(BACKLIGHT);
}

#define backlight_set()			backlight_change(true)
#define backlight_clear()		backlight_change(false)

inline void backlight_toggle(void) __attribute__((always_inline));
inline void backlight_toggle(void)
{
	BACKLIGHT_PORT ^= _BV(BACKLIGHT);
}

#endif /* INCFILE1_H_ */