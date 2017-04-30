
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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "adc.h"

typedef struct _adcint {
	uint8_t samples;
	uint8_t admux;
	bool finished;
	uint16_t result;
	struct _adcint *next;
} adcint_t;

static volatile adcint_t *first;
static volatile adcint_t *current;

ISR(ADC_vect)
{
	static uint8_t sample;
	static uint16_t res;
	
	if(sample < current->samples) {
		if(ADMUX & _BV(ADLAR))
			res += ADCH;				// 8 bit mode
		else
			res += ADCL | (ADCH<<8);	// 10 bit mode
		sample++;
	} else {
		// sampling finished
		current->result = res / current->samples;
		
		res = 0;
		sample = 0;
		
		current = current->next;
		if(current == NULL)
			current = first;
		
		ADMUX = current->admux;
	}
	ADCSRA |= _BV(ADSC) | _BV(ADIF);
}

void adc_init(void)
{
	ADCSRA = _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

int8_t adc_register_channel(adc_t adc)
{
	uint8_t idx = 0;
	adcint_t *head = first;
	adcint_t *n;
	
	n = malloc(sizeof(adcint_t));
	if(n == NULL)
		return -1;

	memset(n, 0, sizeof(adcint_t));
	n->samples = adc.samples;
	n->admux = (adc.vref << REFS0) | (1 << ADLAR) | (adc.channel & 0x1F);

	if(head == NULL) {
		first = n;
		current = n;
		ADMUX = n->admux;
		sei();
		ADCSRA |= _BV(ADSC) | _BV(ADIF);
	} else {
		// walk to the end of the list
		while(head->next) {
			idx++;
			head = head->next;
		}
	
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			head->next = n;
		}
	}
	
	return idx;
}

uint16_t adc_get_result(uint8_t index)
{
	adcint_t *p = first;
	uint16_t res = 0;
	
	while(index && p->next) {
		index--;
		p = p->next;
	}
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		res = p->result;
	}

	return res;
}
