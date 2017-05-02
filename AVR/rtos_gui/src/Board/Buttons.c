/*
  Copyright 2017  Laszlo Hegedues (laszlo.hegedues [at] gmail [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *  \brief LUFA Custom Board Button Hardware Driver (Template)
 *
 *  This is a stub driver header file, for implementing custom board
 *  layout hardware with compatible LUFA board specific drivers. If
 *  the library is configured to use the BOARD_USER board mode, this
 *  driver file should be completed and copied into the "/Board/" folder
 *  inside the application's folder.
 *
 *  This stub is for the board-specific component of the LUFA Buttons driver,
 *  for the control of physical board-mounted GPIO pushbuttons.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "Driver/adc.h"

#define __INCLUDE_FROM_BUTTONS_H
#include "Buttons.h"

#ifndef CONCAT

#endif

#define CONCAT(A,B)			A ## B
#define EXPAND_CONCAT(A,B)  CONCAT(A, B)

#define ARGN(N,LIST)		EXPAND_CONCAT(ARG_,N) LIST
#define ARG_0(A0, ...)		(A0)
#define ARG_1(A0,A1, ...)	(A0+A1)
#define ARG_2(A0,A1,A2, ...)	(A0+A1+A2)
#define ARG_3(A0,A1,A2,A3, ...)	(A0+A1+A2+A3)
#define ARG_4(A0,A1,A2,A3,A4, ...)		(A0+A1+A2+A3+A4)
#define ARG_5(A0,A1,A2,A3,A4,A5, ...)	(A0+A1+A2+A3+A4+A5)
#define ARG_6(A0,A1,A2,A3,A4,A5,A6, ...)		(A0+A1+A2+A3+A4+A5+A6)
#define ARG_7(A0,A1,A2,A3,A4,A5,A6,A7, ...)		(A0+A1+A2+A3+A4+A5+A6+A7)
#define ARG_8(A0,A1,A2,A3,A4,A5,A6,A7,A8, ...)		(A0+A1+A2+A3+A4+A5+A6+A7+A8)
#define ARG_9(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9, ...)	(A0+A1+A2+A3+A4+A5+A6+A7+A8+A9)
#define ARG_10(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10, ...)	(A0+A1+A2+A3+A4+A5+A6+A7+A8+A9+A10)

#define RVCC			3900
#define RESISTOR_LIST	(680,470,470,470,820,820,1200,1500,2200,3300,UINT16_MAX)
#define ADCRES			256

#define ADCVAL(n)		ADCRES*(1-RVCC/(RVCC+ARGN(n,RESISTOR_LIST)))
#define THRESHOLD(n,m)	(ADCVAL(n)+ADCVAL(m))/2

typedef struct _port{
	uint8_t State[MAX_CHECKS];
	uint8_t debounceState;
	uint8_t changedState;
} port_t;

static volatile port_t portb;
static volatile uint16_t adckey_state;
static volatile uint16_t adckey_changedstate;

static const uint8_t adc_thresholds[] PROGMEM = {
#if 0
	THRESHOLD(SKIPN, 10),
	THRESHOLD(STOP, SKIPN),
	THRESHOLD(PLAY, STOP),
	THRESHOLD(SKIPP, PLAY),
	THRESHOLD(REP, SKIPP),
	THRESHOLD(DBB, REP),
	THRESHOLD(VOLP, DBB),
	THRESHOLD(VOLN, VOLP),
	THRESHOLD(PRESETN, VOLN),
	THRESHOLD(PRESETP, PRESETN),
	0
#endif
	224,
	185,
	168,
	151,
	134,
	117,
	99,
	82,
	67,
	48,
	0
};

static int8_t adc_index;

static inline uint16_t key_no(uint8_t adcval){

	uint16_t num = _BV(SKIPN+1);
	const uint8_t *thr = adc_thresholds;
	while(adcval < pgm_read_byte(thr)) {
		thr++;
		num >>= 1;
	}
	return num & ~_BV(SKIPN+1);
}

void Buttons_Debounce(void)
{
	static uint8_t Index;
	uint16_t tmp;
	uint8_t i, j;
	uint8_t adc;

	// Buttons are pulled low, so invert the logic
	portb.State[Index] = PINB ^ (BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM);
	Index++;
	if(Index >= MAX_CHECKS)
		Index = 0;

	// do the debounce
	for(i=0, j = 0xFF; i < MAX_CHECKS; i++) {
		j &= portb.State[i];
	}

	portb.changedState = portb.debounceState ^ j;
	portb.debounceState = j;


	adc = adc_get_result(adc_index);

	tmp = key_no(adc);
	adckey_changedstate = adckey_state ^ tmp;
	adckey_state = tmp;
}

void Buttons_Init(void)
{
	adc_t adc_chan = {
		.channel = 0x04,
		.vref = VREF_AVCC,
		.resolution = RES_8BIT,
		.samples = 1,
	};

	DDRB &= ~(BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM);
	PORTB |= BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM;

	adc_init();
	adc_index = adc_register_channel(adc_chan);
}

bool Buttons_GetStatus(uint32_t button)
{
	if(button & BUTTONS_PHYSICAL)
		return (portb.debounceState & (button & BUTTONS_PHYSICAL)) ? true : false;
	else {
		bool ret;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			ret = (adckey_state & (button >> ADC_DELTA)) ? true : false;
		}
		return ret;
	}
}

bool Buttons_Pressed(uint32_t button)
{
	if(button & BUTTONS_PHYSICAL)
		return ((portb.changedState & portb.debounceState) & (button & BUTTONS_PHYSICAL)) ? true : false;
	else {
		bool ret;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			ret = ((adckey_changedstate & adckey_state) & (button >> ADC_DELTA)) ? true : false;
		}
		return ret;
	}
}

bool Buttons_Released(uint32_t button)
{
	if(button & BUTTONS_PHYSICAL)
		return ((portb.changedState & (~portb.debounceState)) & (button & BUTTONS_PHYSICAL)) ? true : false;
	else {
		bool ret;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			ret = ((adckey_changedstate & (~adckey_state)) & (button >> ADC_DELTA)) ? true : false;
		}
		return ret;
	}
}

uint16_t get_key_press(void)
{
	return adckey_changedstate;
}

uint16_t get_key_state(void)
{
	return adckey_state;
}
