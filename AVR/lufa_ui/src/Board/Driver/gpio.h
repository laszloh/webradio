/*
 * gpio.h
 *
 * Created: 21.04.2017 21:55:30
 *  Author: Simon
 */


#ifndef GPIO_H_
#define GPIO_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct _gpio {
	volatile uint8_t *_ddr;
	volatile uint8_t * _port;
	volatile uint8_t * _pin;
	uint8_t bit;
} gpio_t;

#define gpio_sfr(_port, _pin)				\
	(gpio_t){&DDR##_port, &PORT##_port, &PIN##_port, P##_port##_pin}

#define GPIO_OUTPUT		true
#define GPIO_INPUT		false

// GPIO directions
inline void gpio_direction(gpio_t io, bool output) __attribute__((__always_inline__));
inline void gpio_direction(gpio_t io, bool output) {
	if(output)
		*(io._ddr) |= _BV(io.bit);
	else
		*(io._ddr) &= ~_BV(io.bit);
}

// write to output
inline void gpio_write(gpio_t io, bool level) __attribute__((__always_inline__));
inline void gpio_write(gpio_t io, bool level) {
		if(level)
			*(io._port) |= _BV(io.bit);
		else
			*(io._port) &= ~_BV(io.bit);
}

// toggle gpio
inline void gpio_toggle(gpio_t io) __attribute__((__always_inline__));
inline void gpio_toggle(gpio_t io) {
	*(io._pin) |= _BV(io.bit);
}

// enable/disable pull-ups
#define gpio_pullup(io, enable)	gpio_write(io, enable)


// read port
inline bool gpio_read(gpio_t io) __attribute__((__always_inline__));
inline bool gpio_read(gpio_t io) {
	return (*(io._pin) & _BV(io.bit)) ? true : false;
}


#endif /* GPIO_H_ */