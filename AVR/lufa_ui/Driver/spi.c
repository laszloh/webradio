/*
 * spi.c
 *
 * Created: 25.03.2017 21:39:53
 *  Author: Simon
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdint.h>

#include "spi.h"

#define PORT_SPI    PORTB
#define DDR_SPI     DDRB
#define DD_MISO     DDB4
#define DD_MOSI     DDB3
#define DD_SCK      DDB5


void spi_init(void)
{
	DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_MISO)|(1<<DD_SCK));
	// Define the following pins as output
	DDR_SPI |= ((1<<DD_MOSI)|(1<<DD_SCK));
	
	SPCR = ((1<<SPE)|       // SPI Enable
	(0<<SPIE)|              // SPI Interupt Enable
	(1<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
	(1<<MSTR)|              // Master/Slave select
	(0<<SPR1)|(1<<SPR0)|    // SPI Clock Rate
	(0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
	(0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

	SPSR = (1<<SPI2X);      // Double Clock Rate
}

void spi_byte_tx(uint8_t data)
{
	SPDR = data;
	while(!(SPSR & _BV(SPIF)));
}

void spi_buf_tx(uint8_t *buf, size_t count)
{
	do {
		uint8_t data = *buf;
		buf++;
		while(!(SPSR & _BV(SPIF)));
		SPDR = data;
	} while(--count);
}