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
#include <stdint.h>

#include "Driver/adc.h"

#define __INCLUDE_FROM_BUTTONS_H
#include "Buttons.h"


// copied from excel
#define BUTTONS_ADC_VAL_NONE_MAX	(1024)
#define BUTTONS_ADC_VAL_NONE_MIN	(897)
#define BUTTONS_ADC_VAL_SKIPN_MAX	(896)
#define BUTTONS_ADC_VAL_SKIPN_MIN	(738)
#define BUTTONS_ADC_VAL_STOP_MAX	(737)
#define BUTTONS_ADC_VAL_STOP_MIN	(671)
#define BUTTONS_ADC_VAL_PLAY_MAX	(670)
#define BUTTONS_ADC_VAL_PLAY_MIN	(604)
#define BUTTONS_ADC_VAL_SKIPP_MAX	(603)
#define BUTTONS_ADC_VAL_SKIPP_MIN	(536)
#define BUTTONS_ADC_VAL_REP_MAX		(535)
#define BUTTONS_ADC_VAL_REP_MIN		(469)
#define BUTTONS_ADC_VAL_DBB_MAX		(468)
#define BUTTONS_ADC_VAL_DBB_MIN		(397)
#define BUTTONS_ADC_VAL_VOLP_MAX	(396)
#define BUTTONS_ADC_VAL_VOLP_MIN	(328)
#define BUTTONS_ADC_VAL_VOLN_MAX	(327)
#define BUTTONS_ADC_VAL_VOLN_MIN	(266)
#define BUTTONS_ADC_VAL_PRESETN_MAX	(265)
#define BUTTONS_ADC_VAL_PRESETN_MIN	(192)
#define BUTTONS_ADC_VAL_PRESETP_MAX	(191)
#define BUTTONS_ADC_VAL_PRESETP_MIN	(76)


typedef struct _port{
	uint8_t State[MAX_CHECKS];
	uint8_t debounceState;
	uint8_t changedState;
} port_t;

static volatile port_t portb;
static uint16_t prevState;

static adc_t adc = {
	.channel = 0x04,
	.vref = VREF_AVCC,
	.divider = CDIV_128,
	.samples = 4,
};

//static uint32_t getAdcButton(void);

void Buttons_Init(void)
{
	DDRB &= ~(BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM);
	PORTB |= BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM;
	
	adc_init();
}

void Buttons_Disable(void)
{
	DDRB &= ~(BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM);
	PORTB &= ~(BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM);
}

void Buttons_Debounce(void)
{
	static uint8_t Index = 0;
	uint8_t i, j;
				
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
}

uint8_t Buttons_GetStatus(uint32_t button)
{
	if(button & BUTTONS_PHYSICAL)
		return portb.debounceState & (button & BUTTONS_PHYSICAL) ;
	else
		return getAdcButton() & button;
}
			
uint8_t Buttons_Pressed(uint32_t button)
{	
	if(button & BUTTONS_PHYSICAL)
		return (portb.changedState & portb.debounceState) & (button & BUTTONS_PHYSICAL);
	else {
		uint8_t retVal = (prevState < (getAdcButton() & button));
		prevState = getAdcButton();
		return retVal;
	}
}

uint8_t Buttons_Released(uint32_t button)
{
	if(button & BUTTONS_PHYSICAL)
		return (portb.changedState & (~portb.debounceState)) & (button & BUTTONS_PHYSICAL);
	else {
		uint8_t retVal = (prevState < (getAdcButton() & button));
		prevState = getAdcButton();
		return retVal;
	}
}

uint32_t getAdcButton(void)
{
	uint16_t res;
	
	res = adc_read(adc);
	
	// we got an ADC result
	if(res >= BUTTONS_ADC_VAL_NONE_MIN)
		return ((uint32_t)0x00);
	else if(res <= BUTTONS_ADC_VAL_SKIPN_MAX && res >= BUTTONS_ADC_VAL_SKIPN_MIN)
		return ((uint32_t)BUTTONS_ADC_SKIPN);
	else if(res <= BUTTONS_ADC_VAL_STOP_MAX && res >= BUTTONS_ADC_VAL_STOP_MIN)
		return ((uint32_t)BUTTONS_ADC_STOP);
	else if(res <= BUTTONS_ADC_VAL_PLAY_MAX && res >= BUTTONS_ADC_VAL_PLAY_MIN)
		return ((uint32_t)BUTTONS_ADC_PLAY);
	else if(res <= BUTTONS_ADC_VAL_SKIPP_MAX && res >= BUTTONS_ADC_VAL_SKIPP_MIN)
		return ((uint32_t)BUTTONS_ADC_SKIPP);
	else if(res <= BUTTONS_ADC_VAL_REP_MAX && res >= BUTTONS_ADC_VAL_REP_MIN)
		return ((uint32_t)BUTTONS_ADC_REP);
	else if(res <= BUTTONS_ADC_VAL_DBB_MAX && res >= BUTTONS_ADC_VAL_DBB_MIN)
		return ((uint32_t)BUTTONS_ADC_DBB);
	else if(res <= BUTTONS_ADC_VAL_VOLP_MAX && res >= BUTTONS_ADC_VAL_VOLP_MIN)
		return ((uint32_t)BUTTONS_ADC_VOLP);
	else if(res <= BUTTONS_ADC_VAL_VOLN_MAX && res >= BUTTONS_ADC_VAL_VOLN_MIN)
		return ((uint32_t)BUTTONS_ADC_VOLN);
	else if(res <= BUTTONS_ADC_VAL_PRESETN_MAX && res >= BUTTONS_ADC_VAL_PRESETN_MIN)
		return ((uint32_t)BUTTONS_ADC_PRESETN);
	else
		return ((uint32_t)BUTTONS_ADC_PRESETP);
}


