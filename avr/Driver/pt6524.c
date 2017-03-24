//
//  Copyright (C) 2017 Laszlo Hegedues <laszlo.hegedues [at] gmail [dot] com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a 
//  copy of this software and associated documentation files (the "Software"), 
//  to deal in the Software without restriction, including without limitation 
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//  and/or sell copies of the Software, and to permit persons to whom the 
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in 
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//  DEALINGS IN THE SOFTWARE.
//
//
//  Created by Laszlo Hegedues on 21.03.2017.
//

#include "config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include "utils/io.h"
#include "utils/spi.h"

#define PT_ADDRESS		0x82	// 41H in the "stupid" datasheet configuration

typedef struct _segment {
	uint8_t nibble:4;
} pt6524_seg_t;

typedef struct _frame __attribute__((packed)) {
	pt6524_seg_t segments[13];
	uint8_t _res:2;
	uint8_t cu:1;
	uint8_t port:3;
	uint8_t dr:1;
	uint8_t sc:1;
	uint8_t bu:1;
	uint8_t dd:2;
} pt6524_frame_t;

uint8_t pt6524_init() {
	pt6524_frame_t frame;
	
	// init the SPI
	spi_init();
	DDR_PT |= _BV(DDR_PTS);		// and the CS-Line
	
	memset(frame, 0, sizeof(pt6524_frame_t));
	frame.dr = 1;
	
	pt6524_write(&buffer);
}

void pt6524_write(uint8_t *buf) {
	uint8_t i;
	
	// send the address
	BIT_CLEAR(&PORT_PT, PORT_PTS);
	spi_transmit(PT_ADDRESS);
	BIT_SET(&PORT_PT, PORT_PTS);
	// send the remaining data
	for(i=0;i<PT_FRAME_SIZE;i++) {
		spi_transmit(buf[i]);
	}
}

