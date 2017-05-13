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
 *
 *  Header file for VirtualSerial.c.
 */

#ifndef _VIRTUALSERIAL_H_
#define _VIRTUALSERIAL_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <string.h>
		#include <stdio.h>

		#include "Descriptors.h"

		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/Board/Buttons.h>
		#include <LUFA/Drivers/Board/Board.h>

		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Platform/Platform.h>

	/* Typedefs: */
		typedef struct {
			union {
				uint16_t buttons;
				struct {
					uint8_t power:1;
					uint8_t source:1;
					uint8_t presetp:1;
					uint8_t presetn:1;
					uint8_t ubs:1;
					uint8_t volumep:1;
					uint8_t volumen:1;
					uint8_t program:1;
					uint8_t previous:1;
					uint8_t stop:1;
					uint8_t play:1;
					uint8_t next:1;
					uint8_t repeat:1;
					uint8_t res:3;
				} bits;
			};
		} ATTR_PACKED USB_ButtonReport_Data_t;

		typedef struct {
			uint8_t colums:4;
			uint8_t ascii:1;
			uint8_t readback:1;
			uint8_t vscroll:1;
			uint8_t font14:1;
		} ATTR_PACKED USB_DisplayFeature_t;

		typedef struct {
			uint8_t column:4;
			uint8_t row:4;
		} ATTR_PACKED USB_DisplayCursorPosition_t;

		typedef struct {
			char chars[8];
			uint32_t symbols;
		} ATTR_PACKED USB_DisplayCharacters_t;

		typedef struct {
			uint8_t clear:1;
			uint8_t state:4;
		} ATTR_PACKED USB_DisplayControl_t;

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      (0)

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED1)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED1 | LEDS_LED2)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED2)

	/* Function Prototypes: */
		void SetupHardware(void);
		void CDC_Task(void);
		void IMRP_Task(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
													uint8_t* const ReportID,
													const uint8_t ReportType,
													void* ReportData,
													uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
													const uint8_t ReportID,
													const uint8_t ReportType,
													const void* ReportData,
													const uint16_t ReportSize);

        void ParseCommand(unsigned char c);

#endif

