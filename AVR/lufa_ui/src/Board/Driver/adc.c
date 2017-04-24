

#include <avr/interrupt.h>

#include "adc.h"


static volatile uint16_t chan_result[MAX_REGISTERED_CHANNLES];
static volatile uint16_t chan_ready;

static volatile adc_t adcs[MAX_REGISTERED_CHANNLES];
static volatile uint8_t num_channels;

ISR(ADC_vect, ISR_BLOCK)
{
	static uint8_t cur_adc = 0;
	static uint8_t sample = 0;
	
	if (sample < adcs[cur_adc].samples) {
		chan_result[cur_adc] += ADCL | (ADCH<<8);
		sample++;
	} else {
		chan_result[cur_adc] /= adcs[cur_adc].samples;
		chan_ready = _BV(cur_adc);
		
		sample = 0;
		cur_adc++;
		if(cur_adc >= MAX_REGISTERED_CHANNLES)
			cur_adc = 0;
		chan_result[cur_adc] = 0;
		chan_ready &= ~_BV(cur_adc);
		
		ADCSRA = (ADCSRA & ~(0x03)) | adc.divider;
		ADMUX = (adcs[cur_adc].vref << 7) | (adcs[cur_adc].channel & 0x1F);
	}
}

void adc_init(void)
{
	chan_ready = 0;
	num_channels = 0;
	
	ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE);
}

uint16_t adc_read(adc_t adc)
{
	uint8_t i;
	uint16_t result = 0;
	
	while( (ADCSRA & _BV(ADSC)) & !(ADCSRA & _BV(ADIF)) );
	ADCSRA = (ADCSRA & ~(_BV(ADATE) | _BV(ADIE) | 0x03)) | _BV(ADIF) | adc.divider;
	ADMUX = (adc.vref << 7) | (adc.channel & 0x1F);
	
	for(i=0;i<adc.samples;i++) {
		ADCSRA |= _BV(ADSC);
		while(!(ADCSRA & _BV(ADIF)));
		
		result += ADCL | (ADCH << 8);
	}
	result /= adc.samples;
	
	ADCSRA |= _BV(ADATE) | _BV(ADIE);
	if(num_channels)
		ADCSRA |= _BV(ADSC);
	
	return result;
}

int8_t adc_register_channel(adc_t adc)
{
	if(num_channels >= MAX_REGISTERED_CHANNLES)
		return -1;
	
	num_channels++;
	memcpy(adcs[num_channels], &adc, sizeof(adc_t));
	ADCSRA |= _BV(ADSC);
	
	return num_channels;
}

uint8_t adc_get_result(uint8_t index, uint16_t *res)
{
	uint8_t result = 1;
	
	cli();
	
	if(chan_ready & _BV(index)) {
		*res = chan_result[index];
		chan_ready &= ~_BV(index);
		result = 0;
	}
	
	sei();
	
	return result;
}
