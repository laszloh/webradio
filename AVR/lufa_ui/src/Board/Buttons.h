/*
             LUFA Library
     Copyright (C) Dean Camera, 2016.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2016  Dean Camera (dean [at] fourwalledcubicle [dot] com)

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

#ifndef __BUTTONS_USER_H__
#define __BUTTONS_USER_H__

	/* Includes: */
	#include <avr/io.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_BUTTONS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Buttons.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
			/** Button mask for the first button on the board. */
			#define BUTTONS_POWER		_BV(PORTB6)
			#define BUTTONS_SOURCE		_BV(PORTB5)
			#define BUTTONS_PROGRAM		_BV(PORTB4)
			#define BUTTONS_PHYSICAL	(BUTTONS_POWER | BUTTONS_SOURCE | BUTTONS_PROGRAM)
			
			#define ADC_DELTA			8
			
			#define BUTTONS_ADC_PRESETP	_BV(ADC_DELTA+0)
			#define BUTTONS_ADC_PRESETN	_BV(ADC_DELTA+1)
			#define BUTTONS_ADC_DBB		_BV(ADC_DELTA+2)
			#define BUTTONS_ADC_VOLP	_BV(ADC_DELTA+3)
			#define BUTTONS_ADC_VOLN	_BV(ADC_DELTA+4)
			
			#define BUTTONS_ADC_SKIPN	_BV(ADC_DELTA+5)
			#define BUTTONS_ADC_STOP	_BV(ADC_DELTA+6)
			#define BUTTONS_ADC_PLAY	_BV(ADC_DELTA+7)
			#define BUTTONS_ADC_SKIPP	_BV(ADC_DELTA+8)
			#define BUTTONS_ADC_REP		_BV(ADC_DELTA+9)
			
			#define MAX_CHECKS			10

		/* Inline Functions: */
		#if !defined(__DOXYGEN__)
		
			inline void Buttons_Init(void);

			inline void Buttons_Disable(void);
			
			void Buttons_Debounce(void);

			inline uint8_t Buttons_GetStatus(uint32_t button);
			
			inline uint8_t Buttons_Pressed(uint32_t button);

			inline uint8_t Buttons_Released(uint32_t button);
			
			uint32_t getAdcButton(void);
			
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

