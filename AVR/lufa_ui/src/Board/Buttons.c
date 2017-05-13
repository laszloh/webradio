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

#define BUTTONS_PHY_POWER		(1UL << (PORTB6))
#define BUTTONS_PHY_SOURCE		(1UL << (PORTB5))
#define BUTTONS_PHY_PROGRAM		(1UL << (PORTB4))
#define BUTTONS_PHY_PHYSICAL	(BUTTONS_PHY_POWER | BUTTONS_PHY_SOURCE | BUTTONS_PHY_PROGRAM)

#define ADC_DELTA	3
#define PRESETP		0
#define PRESETN		1
#define VOLN		2
#define VOLP		3
#define DBB			4
#define REP			5
#define SKIPP		6
#define PLAY		7
#define STOP		8
#define SKIPN		9

#define BUTTONS_ADC_PRESETP	(1UL << (ADC_DELTA+PRESETP))
#define BUTTONS_ADC_PRESETN	(1UL << (ADC_DELTA+PRESETN))
#define BUTTONS_ADC_VOLN	(1UL << (ADC_DELTA+VOLN))
#define BUTTONS_ADC_VOLP	(1UL << (ADC_DELTA+VOLP))
#define BUTTONS_ADC_DBB		(1UL << (ADC_DELTA+DBB))
#define BUTTONS_ADC_REP		(1UL << (ADC_DELTA+REP))
#define BUTTONS_ADC_SKIPP	(1UL << (ADC_DELTA+SKIPP))
#define BUTTONS_ADC_PLAY	(1UL << (ADC_DELTA+PLAY))
#define BUTTONS_ADC_STOP	(1UL << (ADC_DELTA+STOP))
#define BUTTONS_ADC_SKIPN	(1UL << (ADC_DELTA+SKIPN))
#define BUTTONS_ADC			(BUTTONS_ADC_PRESETP | BUTTONS_ADC_PRESETN | BUTTONS_ADC_VOLN | BUTTONS_ADC_VOLP | \
							BUTTONS_ADC_DBB | BUTTONS_ADC_REP | BUTTONS_ADC_SKIPP | BUTTONS_ADC_PLAY | \
							BUTTONS_ADC_STOP | BUTTONS_ADC_SKIPN)


#define MAX_CHECKS			10

typedef struct _port{
	uint8_t State[MAX_CHECKS];
	uint8_t debounceState;
	uint8_t changedState;
} port_t;

static volatile port_t portb;
static volatile uint16_t adckey_state;
static volatile uint16_t adckey_changedstate;

static const uint8_t adc_thresholds[] PROGMEM = {
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
	portb.State[Index] = PINB ^ (BUTTONS_PHY_POWER | BUTTONS_PHY_SOURCE | BUTTONS_PHY_PROGRAM);
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

	DDRB &= ~(BUTTONS_PHY_POWER | BUTTONS_PHY_SOURCE | BUTTONS_PHY_PROGRAM);
	PORTB |= BUTTONS_PHY_POWER | BUTTONS_PHY_SOURCE | BUTTONS_PHY_PROGRAM;

	adc_init();
	adc_index = adc_register_channel(adc_chan);
}

buttons_t Buttons_All_GetStatus(void)
{
	buttons_t ret;

	ret.raw = 0x0000;
	ret.btn.power = (portb.debounceState & BUTTONS_PHY_POWER) ? 0x01 : 0x00;
	ret.btn.source = (portb.debounceState & BUTTONS_PHY_SOURCE) ? 0x01 : 0x00;
	ret.btn.program = (portb.debounceState & BUTTONS_PHY_PROGRAM) ? 0x01 : 0x00;

	ret.raw |= (adckey_state << ADC_DELTA);

	return ret;
}

buttons_t Buttons_All_Pressed(void)
{
	buttons_t ret;

	ret.raw = 0x0000;
	ret.btn.power = (portb.changedState & portb.debounceState & BUTTONS_PHY_POWER) ? 0x01 : 0x00;
	ret.btn.source = (portb.changedState & portb.debounceState & BUTTONS_PHY_SOURCE) ? 0x01 : 0x00;
	ret.btn.program = (portb.changedState & portb.debounceState & BUTTONS_PHY_PROGRAM) ? 0x01 : 0x00;

	ret.raw |= ((adckey_changedstate & adckey_state) << ADC_DELTA);

	return ret;
}

buttons_t Buttons_All_Released(void)
{
	buttons_t ret;

	ret.raw = 0x0000;
	ret.btn.power = (portb.changedState & (~portb.debounceState) & BUTTONS_PHY_POWER) ? 0x01 : 0x00;
	ret.btn.source = (portb.changedState & (~portb.debounceState) & BUTTONS_PHY_SOURCE) ? 0x01 : 0x00;
	ret.btn.program = (portb.changedState & (~portb.debounceState) & BUTTONS_PHY_PROGRAM) ? 0x01 : 0x00;

	ret.raw |= ((adckey_changedstate & (~adckey_state)) << ADC_DELTA);

	return ret;

}

