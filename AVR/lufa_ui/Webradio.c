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

#include "Board/Driver/bootloader.h"
#include "Board/LCD.h"

volatile uint16_t isrcnt = 0;

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

	/* Create a regular character stream for the interface so that it can be used with the stdio.h functions */
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();

	for (;;)
	{
        CDC_Task();

		IMRP_Task();

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
	//bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;
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

#if 0
    int i;
	uint8_t carry = 0, x = 0;
    for (i = 0;  i < len;  ++i) {
		if(carry) {
			arr[i] |= carry;
			x = 1;
		}
		carry = (arr[i] & 0x8000) ? 0x01 : 0x00;
		if (!x)
			arr[i] <<= 1;
		x = 0;
    }
#endif
}

void ParseCommand(unsigned char c)
{
	char buffer[128];
	char *pos = buffer;
	static uint16_t index = 0;
	static bool sym_state = true;

	memset(buffer, 0, sizeof(buffer));

    switch(c) {
#if 0
    case 'n':
    case 'N':
        ShiftLeftByOne((uint64_t*)segments, sizeof(segments)/sizeof(uint64_t));
		pt6524_write_raw(segments, sizeof(segments)/sizeof(uint16_t), PT6524_LCD_SEGMENTS);
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("Shifting Bit left...\n\rPattern: "));
		int i = 12;
		do {
			pos += sprintf(pos, "%04X ", segments[i-1]);
		} while(--i);
		sprintf(pos, "\n\r");
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buffer);
		sprintf(buffer, "  idx: %d\n\r", index);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buffer);
		index++;
		if(index > 52)
			index = 0;
        break;

    case 's':
    case 'S':
        memset(segments, 0, sizeof(segments));
        segments[0] = 0x01;
		index = 1;
		pt6524_write_raw(segments, sizeof(segments)/sizeof(uint16_t), PT6524_LCD_SEGMENTS);
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("Reseting LCD...\n\r"));
        break;

	case 'b':
	case 'B':
		// toggle backlight
		backlight_toggle();
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("Backlight toggle...\n\r"));
		break;

	case 'a':
	case 'A':
		memset(segments, 0xFF, sizeof(segments));
		pt6524_write_raw(segments, sizeof(segments)/sizeof(uint16_t), PT6524_LCD_SEGMENTS);
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("Enabling all segments...\n\r"));
		break;
#endif

	case 's':
		LCD_SetSymbol(SYM_USB, sym_state);
		sym_state ^= true;
		break;

	case 'h':
		LCD_SetString_P(PSTR("Hello Wrld"));
		break;

	case 'p':
	case 'P':
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("Entering bootloader...\n\r"));
		Jump_To_Bootloader();
		break;

	case 'k':
		sprintf(buffer, "state: 0x%04x; press: 0x%04x; isr: %d\n\r", get_key_state(), get_key_press(), isrcnt);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buffer);
		break;

    default:
        return;
    }

}

void CDC_Task(void)
{
//	static uint8_t idx = 0;
    uint32_t bytes;
//	char buf[64];

	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
        return;

	if(Buttons_Pressed(BUTTONS_POWER))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    pwr pressed\n\r"));
	else if(Buttons_Released(BUTTONS_POWER))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    pwr released\n\r"));

	if(Buttons_Pressed(BUTTONS_SOURCE))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    scr pressed\n\r"));
	else if(Buttons_Released(BUTTONS_SOURCE))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    scr released\n\r"));

	if(Buttons_Pressed(BUTTONS_PROGRAM))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    prg pressed\n\r"));
	else if(Buttons_Released(BUTTONS_PROGRAM))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    prg released\n\r"));

	if(Buttons_Pressed(BUTTONS_ADC))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    adc pressed\n\r"));
	else if(Buttons_Released(BUTTONS_ADC))
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("    adc released\n\r"));


//	sprintf(buf, "state: 0x%08lx\n\r", BUTTONS_ADC);
//	CDC_Device_SendString(&VirtualSerial_CDC_Interface, buf);


    bytes = CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface);

    while(bytes > 0) {
        uint16_t c = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
        if(c >= 0) {
			CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (uint8_t)c);
			CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("\n\r"));
            ParseCommand(c);
		}
        bytes--;
    }
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
	IRMP_DATA irmp_data;
	char buf[5];

	if (irmp_get_data (&irmp_data)) {
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("protocol: 0x"));
		itoh(buf, 2, irmp_data.protocol);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buf);
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("   "));
		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, (const char*)pgm_read_word (&(irmp_protocol_names[irmp_data.protocol])));

		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("   address: 0x"));
		itoh (buf, 4, irmp_data.address);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buf);

		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("   command: 0x"));
		itoh (buf, 4, irmp_data.command);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buf);

		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("   flags: 0x"));
		itoh (buf, 2, irmp_data.flags);
		CDC_Device_SendString(&VirtualSerial_CDC_Interface, buf);

		CDC_Device_SendString_P(&VirtualSerial_CDC_Interface, PSTR("\r\n"));
	}
}