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
 *  Main source file for the GenericHID demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.
 */

#include "Webradio.h"

#undef CONCAT
#include "irmp.h"

#include "Board/LCD.h"
#include "Board/Buttons.h"
#include "Board/Driver/backlight.h"
#include "Board/Driver/pt6524.h"
#include "Board/Driver/gpio.h"
#include "Board/Driver/adc.h"

volatile uint16_t isrcnt = 0;

static char *itoh (char * buf, uint8_t digits, uint16_t number);

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[HID_EPSIZE];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Device_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_RemoteAndDisplay,
				.ReportINEndpoint             =
					{
						.Address              = HID_IN_EPADDR,
						.Size                 = HID_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};

#define PT6524_LCD_SEGMENTS 192
uint16_t segments[ PT6524_LCD_SEGMENTS / sizeof(uint16_t) ];

static void timer1_Init (void)
{
	OCR1A = (F_CPU / F_INTERRUPTS) - 1;		// compare value: 1/15000 of CPU frequency
	TCCR1B = _BV(WGM12) | _BV(CS10);		// switch CTC Mode on, set prescaler to 1
	TIMSK1 = _BV(OCIE1A);					// OCIE1A: Interrupt by timer compare
}

// Timer1 output compare A interrupt service routine, called every 1/15000 sec
ISR(TIMER1_COMPA_vect)
{
	(void) irmp_ISR();			// call irmp ISR
	Buttons_Debounce();
}

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
		IMRP_Task();

		HID_Device_USBTask(&Device_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	LCD_Init();

	irmp_init();
	timer1_Init();

	LEDs_Init();
	LCD_Init();
	Buttons_Init();
	USB_Init();
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management task.
 */
void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the generic HID device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Device_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Device_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Device_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	bool force = false;
	IRMP_DATA data;

	if(ReportType == HID_REPORT_ITEM_Feature && (*ReportID & 0xF0) == HID_REPORTID_DisplayReport) {
		switch((*ReportID & 0x0F)) {
			case 0x01:
			{
				USB_DisplayFeature_t* r = (USB_DisplayFeature_t*) ReportData;
				*ReportSize = sizeof(USB_DisplayFeature_t);

				r->colums = 8;
				r->ascii = 1;
				r->font14 = 1;
				r->vscroll = 0;
			}
			break;
#if 0
			case 0x02:
			{
				USB_DisplayCursorPosition_t* r = (USB_DisplayCursorPosition_t*) ReportData;
				*ReportSize = sizeof(USB_DisplayCursorPosition_t);

				r->row = 1;
				r->column = LCD_GetCursor();
			}
			break;

			case 0x03:
			{
				USB_DisplayCharacters_t* r = (USB_DisplayCharacters_t*) ReportData;
				*ReportSize = sizeof(USB_DisplayCharacters_t);

				memcpy(r->chars, "Hlo wrld", 8);
				for(uint8_t i=SYM_USB;i<SYM_MAX;i++) {
					r->symbols = LCD
				}
			}
			break;
#endif

			default:
				break;
		}
		return true;
	}


	if(!(*ReportID)) {
		// find a report ID to return
		if(irmp_get_data(&data))
			*ReportID = HID_REPORTID_RemoteReport;
		else
			*ReportID = HID_REPORTID_ButtonsReport;
	}

	switch(*ReportID) {
		case HID_REPORTID_RemoteReport:
			memcpy(ReportData, &data, sizeof(IRMP_DATA));
			*ReportSize = sizeof(IRMP_DATA);
			force = true;
			break;

		case HID_REPORTID_ButtonsReport:
			{
				buttons_t data;
				data = Buttons_All_GetStatus();
				memcpy(ReportData, &data, sizeof(buttons_t));
				*ReportSize = 6;
			}
			break;
	}

	return force;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void __attribute__ ((noinline)) CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	if((ReportID & 0xF0) != HID_REPORTID_DisplayReport)
		return;

	// we only process character reports
	switch(ReportID & 0x0F) {
		case HID_DISPLAYSUBID_charreport:
		{
			// sanity check
			if(ReportSize != sizeof(USB_DisplayCharacters_t))
				return;
			USB_DisplayCharacters_t *r = (USB_DisplayCharacters_t*)ReportData;
			for(uint8_t i=SYM_CDROM;i<SYM_MAX;i++) {
				bool en = (r->symbols & (1UL << i));
				LCD_SetSymbol(i, en);
			}

			LCD_PutString(r->chars, 0);
		}
		break;

		case HID_DISPLAYSUBID_displayreport:
		{
			char buf[3];

			// sanity check
			if(ReportSize != sizeof(USB_DisplayControl_t))
				return;
			USB_DisplayControl_t *r = (USB_DisplayControl_t*)ReportData;

			itoh(buf, 2, r->state);
			LCD_PutString_P(PSTR("0x"), 0);
			LCD_PutString(buf, 2);

			if(r->clear)
				LCD_Clear();

			LCD_SetState((display_state_t)r->state);
		}
		break;

		default:
			break;
	}
}

void ShiftLeftByOne(uint64_t *arr, int len)
{
	int i;
	uint8_t carry = 0, done = 0;

	for(i=0;i<len;i++) {
		if(carry) {
			arr[i] |= carry;
			done = 1;
		}
		carry = (arr[i] >> 52) & 0x01;
		if(!done)
			arr[i] <<= 1;
		done = 0;
	}
}

void ParseCommand(unsigned char c)
{
}

void CDC_Task(void)
{

}

static char *itoh (char * buf, uint8_t digits, uint16_t number)
{
	for (buf[digits] = 0; digits--; number >>= 4)
	{
		buf[digits] = "0123456789ABCDEF"[number & 0x0F];
	}
	return buf;
}

void IMRP_Task(void)
{

}