/**
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* AVR includes */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>

/* IRMP includes */
#undef CONCAT
#include "irmp.h"

/* LUFA includes */
#include "Descriptors.h"

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/Board/Buttons.h>
#include <LUFA/Drivers/Board/Board.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

/* custom driver includes */
#include "Board/LCD.h"

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"

/* Priority definitions for most tasks in the application.  Some
   tasks just use the idle priority. */
#define mainIRMP_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainUSB_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define mainGUI_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )

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

SemaphoreHandle_t xCdcInterface;


/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs.
 */
static FILE USBSerialStream;

/*
 * Error check task
 */
static void vGuiTask( void *pvParameters );
static void vUsbTask( void *pvParameters );
static void vIrmpTask( void *pvParameters );

/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationTickHook( void );
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint16_t *pusIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint16_t *pusTimerTaskStackSize );

/*
 * System death notifications
 */

#define STACK_OVERFLOW				3
#define MALLOC_FAILED				6
#define COULD_NOT_ALLOCATE_TASK		9
#define COULD_NOT_ALLOCATE_MUTEX	12

#define ERROR_FLASH_TIMEOUT			500
#define COUNT_WAIT_TIMEOUT			5000

static void die(uint8_t count);

/*-----------------------------------------------------------*/

int main( void )
{
	/* Create the tasks defined within this file. */
	if(xTaskCreate(vGuiTask, "GUI", configMINIMAL_STACK_SIZE+100, NULL, mainGUI_TASK_PRIORITY, NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
		die(COULD_NOT_ALLOCATE_TASK);
	if(xTaskCreate(vUsbTask, "USB", configMINIMAL_STACK_SIZE+100, NULL, mainUSB_TASK_PRIORITY, NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
		die(COULD_NOT_ALLOCATE_TASK);
	if(xTaskCreate(vIrmpTask, "imrp", configMINIMAL_STACK_SIZE+100, NULL, mainIRMP_TASK_PRIORITY, NULL) == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
		die(COULD_NOT_ALLOCATE_TASK);

	/* In this port, to use preemptive scheduler define configUSE_PREEMPTION
	as 1 in portmacro.h.  To use the cooperative scheduler define
	configUSE_PREEMPTION as 0. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/

#define FLASH_RATE (1000/portTICK_PERIOD_MS)

static void vGuiTask( void *pvParameters )
{
	TickType_t FlashTime;
	symbols_t s = SYM_CDROM;

	/* The parameters are not used. */
	( void ) pvParameters;

	LCD_Init();
	LCD_SetStandby(false);
	LCD_SetSymbol(s, true);

	LEDs_Init();
	FlashTime = xTaskGetTickCount();;

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error. */
	for( ;; )
	{
		if( (xTaskGetTickCount() - FlashTime) >= FLASH_RATE ) {
			FlashTime = xTaskGetTickCount();
			LEDs_ToggleLEDs(LEDS_LED1);

			LCD_SetSymbol(s, false);
			s = (symbols_t) s + 1;
			if(s >= SYM_MAX)
				s = SYM_CDROM;
			LCD_SetSymbol(s, true);
		}

		vTaskDelay(5);
	}
}


#define USB_PERIOD_MS	(2 / portTICK_PERIOD_MS)

static void vUsbTask( void *pvParameters )
{
	TickType_t xLastWakeTime;

	/* The parameters are not used. */
	( void ) pvParameters;

	USB_Init();
	CDC_Device_CreateStream(&VirtualSerial_CDC_Interface, &USBSerialStream);

	xCdcInterface = xSemaphoreCreateMutex();
	if(xCdcInterface == NULL) {
		die(COULD_NOT_ALLOCATE_MUTEX);
	}
	xSemaphoreGive(xCdcInterface);

	xLastWakeTime = xTaskGetTickCount();

	for( ;; )
	{
//		vTaskDelayUntil(&xLastWakeTime, USB_PERIOD_MS);

		xSemaphoreTake(xCdcInterface, portMAX_DELAY);
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		xSemaphoreGive(xCdcInterface);

		USB_USBTask();
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

ISR(TIMER3_COMPA_vect)
{
	(void) irmp_ISR();			// call irmp ISR
	Buttons_Debounce();
}

static void timer3_init(void)
{
	OCR3A = (F_CPU / F_INTERRUPTS) - 1;		// compare value: 1/15000 of CPU frequency
	TCCR3B = _BV(WGM12) | _BV(CS10);		// switch CTC Mode on, set prescaler to 1
	TIMSK3 = _BV(OCIE3A);					// OCIE1A: Interrupt by timer compare
}

static void vIrmpTask( void *pvParameters )
{
	IRMP_DATA irmp_data;
	char buf[5];

	/* The parameters are not used. */
	( void ) pvParameters;

	timer3_init();
	irmp_init();

	/* Cycle for ever, delaying then checking all the other tasks are still
	operating without error. */
	for( ;; )
	{
		if (irmp_get_data (&irmp_data)) {
			xSemaphoreTake(xCdcInterface, portMAX_DELAY);

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

			xSemaphoreGive(xCdcInterface);
		}

		vTaskDelay( 10 / portTICK_PERIOD_MS );
	}
}

/*-----------------------------------------------------------*/

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management task.
 */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  of the USB device after enumeration, and configures the generic HID device endpoints.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
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
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

void vApplicationMallocFailedHook(void)
{
	die(MALLOC_FAILED);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	die(STACK_OVERFLOW);
}

void vApplicationTickHook( void )
{

}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint16_t *pusIdleTaskStackSize )
{
/* The buffers used by the idle task must be static so they are persistent, and
so exist after this function returns. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

	/* configUSE_STATIC_ALLOCATION is set to 1, so the application has the
	opportunity to supply the buffers that will be used by the Idle task as its
	stack and to hold its TCB.  If these are set to NULL then the buffers will
	be allocated dynamically, just as if xTaskCreate() had been called. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;
	*pusIdleTaskStackSize = configMINIMAL_STACK_SIZE; /* In words.  NOT in bytes! */
}

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint16_t *pusTimerTaskStackSize )
{
/* The buffers used by the Timer/Daemon task must be static so they are
persistent, and so exist after this function returns. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

	/* configUSE_STATIC_ALLOCATION is set to 1, so the application has the
	opportunity to supply the buffers that will be used by the Timer/RTOS daemon
	task as its	stack and to hold its TCB.  If these are set to NULL then the
	buffers will be allocated dynamically, just as if xTaskCreate() had been
	called. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pusTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH; /* In words.  NOT in bytes! */
}

void die(uint8_t count)
{
	uint8_t i;

	cli();

	LEDs_Init();
	LCD_Init();
	LCD_Clear();
	LCD_SetStandby(false);
	
	LCD_PutChar('F', 0);
	LCD_PutChar('0' + count, 1);
	
	while(1) {
		for(i=0;i<count;i++) {
			LEDs_TurnOnLEDs(LEDS_ALL_LEDS);
			_delay_ms(ERROR_FLASH_TIMEOUT/2);
			LEDs_TurnOffLEDs(LEDS_ALL_LEDS);
			_delay_ms(ERROR_FLASH_TIMEOUT/2);
		}
		_delay_ms(COUNT_WAIT_TIMEOUT);
	}
}
