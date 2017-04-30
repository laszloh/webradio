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
#include <stdint.h>

#define ENABLE_INTERRUPT_HANDLER	1
#define MAX_REGISTERED_CHANNLES		10

typedef enum _vref {
	VREF_AREF = 0,
	VREF_AVCC,
	VREF_RES,
	VREF_INTERNAL
} vref_t;

typedef enum _resultion {
	RES_10BIT = 0,
	RES_8BIT
} resolution_t;

typedef struct _adc {
	uint8_t channel;
	vref_t vref;
	resolution_t resolution;
	uint8_t samples;
} adc_t;

void adc_init(void);

int8_t adc_register_channel(adc_t adc);

uint16_t adc_get_result(uint8_t index);

extern volatile uint16_t isrcnt;
