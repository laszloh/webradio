
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

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "adc.h"


static volatile uint16_t chan_result[MAX_REGISTERED_CHANNLES];
static volatile uint16_t chan_ready;

static volatile adc_t adcs[MAX_REGISTERED_CHANNLES];
static volatile uint8_t num_channels;

ISR(ADC_vect, ISR_BLOCK)
{
	static uint8_t cur_adc = 0;
	static uint8_t sample = 0;
	static uint16_t res = 0;
	
	if (sample < adcs[cur_adc].samples) {
		if(adcs[cur_adc].resolution == RES_10BIT)
			res += ADCL | (ADCH<<8);
		else
			res += ADCH;
		sample++;
	} else {
		chan_result[cur_adc] = res / adcs[cur_adc].samples;
		chan_ready |= _BV(cur_adc);

		res = 0;
		sample = 0;
		
		cur_adc++;
		if(cur_adc >= num_channels)
			cur_adc = 0;
		
		ADCSRA = (ADCSRA & ~(0x03)) | adcs[cur_adc].divider;
		ADMUX = (adcs[cur_adc].vref << 7) | (adcs[cur_adc].resolution <<  ADLAR) | (adcs[cur_adc].channel & 0x1F);
	}
}

void adc_init(void)
{
	chan_ready = 0;
	num_channels = 0;
	
	ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE);
}

int8_t adc_register_channel(adc_t adc)
{
	if(num_channels >= MAX_REGISTERED_CHANNLES)
		return -1;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		num_channels++;
		memcpy(&adcs[num_channels], &adc, sizeof(adc_t));
		ADCSRA |= _BV(ADSC);
	}
	
	return num_channels;
}

uint8_t adc_get_result(uint8_t index, uint16_t *res)
{
	uint8_t result = 1;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if(chan_ready & _BV(index)) {
			*res = chan_result[index];
			chan_ready &= ~_BV(index);
			result = 0;
		}
	}
	
	return result;
}
