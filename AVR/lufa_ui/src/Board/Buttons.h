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
	#include <stdint.h>
	#include <stdbool.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			extern "C" {
		#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_BUTTONS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Buttons.h instead.
		#endif

	/* Typedef: */
		typedef struct {
			union {
				uint16_t raw;
				struct {
					uint8_t power:1;
					uint8_t source:1;
					uint8_t program:1;
					uint8_t presetp:1;
					uint8_t presetn:1;
					uint8_t voln:1;
					uint8_t volp:1;
					uint8_t dbb:1;
					uint8_t rep:1;
					uint8_t skipp:1;
					uint8_t play:1;
					uint8_t stop:1;
					uint8_t skipn:1;
				} btn;
			};
		} buttons_t;

	/* Functions: */
		#if !defined(__DOXYGEN__)

			void Buttons_Init(void);

			void Buttons_Debounce(void);

			buttons_t Buttons_All_GetStatus(void);

			buttons_t Buttons_All_Pressed(void);

			buttons_t Buttons_All_Released(void);

		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
			}
		#endif

#endif

