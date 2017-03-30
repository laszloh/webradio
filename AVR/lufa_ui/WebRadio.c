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

#include "WebRadio.h"
#include "Driver/pt6524.h"
#include "Driver/backlight.h"

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
    .Config =
        {
            .ControlInterfaceNumber   = INTERFACE_ID_CDC_CCI,
            .DataINEndpoint           =
                {
                    .Address          = CDC_TX_EPADDR,
                    .Size             = CDC_TXRX_EPSIZE,
                    .Banks            = 1,
                },
            .DataOUTEndpoint =
                {
                    .Address          = CDC_RX_EPADDR,
                    .Size             = CDC_TXRX_EPSIZE,
                    .Banks            = 1,
                },
            .NotificationEndpoint =
                {
                    .Address          = CDC_NOTIFICATION_EPADDR,
                    .Size             = CDC_NOTIFICATION_EPSIZE,
                    .Banks            = 1,
                },
        },
};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;

#define PT6524_LCD_SEGMENTS 144

uint16_t segments[ PT6524_LCD_SEGMENTS / sizeof(uint16_t) ];

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
        CDC_Task();
        
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
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
	Backlight_Init();
	pt6524_Init();

	LEDs_Init();
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

	/* Setup HID Report Endpoints */
	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);

	/* Indicate endpoint configuration success or failure */
	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo)
{
	/* You can get changes to the virtual CDC lines in this callback; a common
	   use-case is to use the Data Terminal Ready (DTR) flag to enable and
	   disable CDC communications in your application when set to avoid the
	   application blocking while waiting for a host to become ready and read
	   in the pending data from the USB endpoints.
	*/
	bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
}

void ShiftLeftByOne(uint16_t *arr, int len)
{
    int i;
    for (i = 0;  i < len - 1;  ++i) {
        arr[i] = (arr[i] << 1) | ((arr[i+1] >> 15) & 1);
    }
    arr[len-1] = arr[len-1] << 1;
}

void ParseCommand(unsigned char c)
{
    switch(c) {
    case 'n':
    case 'N':
        ShiftLeftByOne(segments, sizeof(segments));
		pt6524_write_raw(segments, sizeof(segments)/sizeof(uint16_t), PT6524_LCD_SEGMENTS);
        break;
        
    case 's':
    case 'S':
        memset(segments, 0, sizeof(segments));
        segments[0] = 0x01;
		pt6524_write_raw(segments, sizeof(segments)/sizeof(uint16_t), PT6524_LCD_SEGMENTS);
        break;
		
	case 'b':
	case 'B':
		// toggle backlight
		backlight_toggle();
		break;
        
    default:
        return;
    }
}

void CDC_Task(void)
{
    uint16_t bytes;
    
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

    bytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);
    
    while(bytes > 0) {
        uint16_t c = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        if(c >= 0)
            ParseCommand(c);
        bytes--;
    }
}
