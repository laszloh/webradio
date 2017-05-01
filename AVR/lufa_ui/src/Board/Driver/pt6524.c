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

//#include "config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <avr/io.h>
#include <LUFA/Drivers/Peripheral/SPI.h>

#include "pt6524.h"
#include "gpio.h"

#define PT_ADDRESS		0x41	// 41H
#define CHIPSEL			gpio_sfr(D,7)

typedef struct _frame {
	uint64_t data:52;
	uint8_t _res:2;
	uint8_t cu:1;
	uint8_t port:4;
	uint8_t dr:1;
	uint8_t sc:1;
	uint8_t bu:1;
	uint8_t dd:2;
}  __attribute__((packed)) pt6524_frame_t;

static void pt6524_write(pt6524_frame_t *buf);
static void pt6524_framesetup(pt6524_frame_t *frame);

void pt6524_Init(void)
{
	pt6524_frame_t frame;
    int i;

	// init the SPI
	SPI_Init(SPI_SPEED_FCPU_DIV_2 | SPI_SCK_LEAD_FALLING | SPI_ORDER_LSB_FIRST |
			 SPI_SAMPLE_TRAILING | SPI_MODE_MASTER);
	gpio_direction(CHIPSEL, true);

	pt6524_framesetup(&frame);

    for(i=0; i < 4; i++) {
		frame.dd = i;
        pt6524_write(&frame);
    }
}

void pt6524_write_raw(void *buffer, size_t size, uint16_t segments)
{
    pt6524_frame_t frame;
    uint8_t i;
    uint64_t *p = (uint64_t*)buffer;

	pt6524_framesetup(&frame);

    for(i=0; i<segments/52; i++) {
		frame.data = p[i];
		frame.dd = i;
		pt6524_write(&frame);
    }
}

static void pt6524_framesetup(pt6524_frame_t *frame)
{
	memset(frame, 0, sizeof(pt6524_frame_t));
}

static void pt6524_write(pt6524_frame_t *buf) {
	uint8_t i;
	uint8_t *p = (uint8_t*)buf;

	// send the address
	gpio_write(CHIPSEL, false);
	SPI_SendByte(PT_ADDRESS);
	gpio_write(CHIPSEL, true);
	// send the remaining data
	for(i=0; i<sizeof(pt6524_frame_t); i++)
		SPI_SendByte(p[i]);
	// disable CS
	gpio_write(CHIPSEL, false);
}

