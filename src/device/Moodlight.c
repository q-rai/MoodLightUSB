/*
             LUFA Library
     Copyright (C) Dean Camera, 2011.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2011  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
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
 *  Main source file for the GenericHID demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Moodlight.h"

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevHIDReportBuffer[GENERIC_REPORT_SIZE];
/** RGB colour codes */
uint16_t red, green, blue;

/** Structure to contain reports from the host, so that they can be echoed back upon request */
static struct
{
	uint8_t  ReportID;
	uint16_t ReportSize;
	uint8_t  ReportData[GENERIC_REPORT_SIZE];
} HIDReportEcho;

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Generic_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = 0,

				.ReportINEndpointNumber       = GENERIC_IN_EPNUM,
				.ReportINEndpointSize         = GENERIC_EPSIZE,
				.ReportINEndpointDoubleBank   = false,

				.PrevReportINBuffer           = PrevHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
		HID_Device_USBTask(&Generic_HID_Interface);
		USB_USBTask();
        // demo(1); // demo forever!
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
	LEDs_Init();
	USB_Init();
        DDRB |= (1 << PB7);
        DDRC |= (1 << PC6) | (1 << PC5);
        TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1<<COM1C1) | (1<<WGM11) | (1<<WGM10); // 10 bit fast-pwm
        TCCR1B |= (1<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10); // clock/1 prescaler
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Generic_HID_Interface);

	USB_Device_EnableSOFEvents();

	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Generic_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Generic_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	if (HIDReportEcho.ReportID)
	  *ReportID = HIDReportEcho.ReportID;

	memcpy(ReportData, HIDReportEcho.ReportData, HIDReportEcho.ReportSize);

	*ReportSize = HIDReportEcho.ReportSize;
	return true;
}

/**
 * Set member variables for red, green, and blue light.
 * \param[in] r red
 * \param[in] g green
 * \param[in] b blue
 */
void setRGB( uint8_t r, uint8_t g, uint8_t b )
{
    red = r * 4;
    green = g * 4;
    blue = b * 4;
}

/**
 * Switch on light.
 */
void on()
{
    OCR1A = blue;
    OCR1B = green;
    OCR1C = red;        
}

/**
 * Switch off light.
 */
void off()
{
    OCR1A = 0;
    OCR1B = 0;
    OCR1C = 0;     
}

/**
 * Blink (1 second on, 1 second off).
 * \param[in] n number of times to blink
 */
void blink( uint8_t n )
{
    for( uint8_t i = 0; i < n; i++ )
    {
        on();      
        _delay_ms(1000);
        off();
        _delay_ms(1000);
    }
}
/**
 * Advanced fading taking into account how the colour is distributed.
 * e.g. ff 00 33 fades evenly from black to pink.
 * Replaces \ref fadeSimple().
 *
 * \param[in] n number of times to fade in and out
 */
void fade( uint8_t n )
{
    // use relative step sizes for each colour channel
    int32_t rStep, gStep, bStep, scale;
    scale = 48 * 4;
    rStep = red / scale;
    gStep = green / scale;
    bStep = blue / scale;

    // repeat n times
    for( uint8_t i = 0; i < n; i++ )
    {
        // fade in to colour maximum
        while( OCR1A < blue || OCR1B < green || OCR1C < red )
        {
            if( OCR1A < blue )
                OCR1A += bStep;
            if( OCR1B < green )
                OCR1B += gStep; 
            if( OCR1C < red )
                OCR1C += rStep; 
            _delay_ms(10);
        }
        // fade out to black
        while( OCR1A > bStep || OCR1B > gStep || OCR1C > rStep )
        {
            if( OCR1A >= bStep )
                OCR1A -= bStep;
            if( OCR1B >= gStep )
                OCR1B -= gStep;
            if( OCR1C >= rStep )
                OCR1C -= rStep;
            _delay_ms(10);
        }
    }
    off();
}

/**
 * Simple fading (constant increase/decrease of values, independent of colour distribution).
 * Looks good, if all channels are equal or zero, but e.g. for ff 00 33, red and blue fade up 
 * simultaneously until 33 00 33 is reached; afterwards, only red is increased up to ff 00 33.
 *
 * \param[in] n number of times to fade in and out
 */
void fadeSimple( uint8_t n )
{
    for( uint8_t i = 0; i < n; i++ )
    {
        while( OCR1A < blue || OCR1B < green || OCR1C < red )
        {
            if( OCR1A < blue )
                OCR1A++;
            if( OCR1B < green )
                OCR1B++; 
            if( OCR1C < red )
                OCR1C++; 
            _delay_ms(10);
        }
        while( OCR1A > 0 || OCR1B > 0 || OCR1C > 0 )
        {
            if( OCR1A > 0 )
                OCR1A--;
            if( OCR1B > 0 )
                OCR1B--;
            if( OCR1C > 0 )
                OCR1C--;
            _delay_ms(10);
        }
    }
}

void demo( uint8_t n )
{
    for( uint8_t i = 0; i < n; i++)
    {
	setRGB( 0xff, 0, 0 );
        fade(1);
        setRGB( 0xff, 0xff, 0 );
        fade(1);
        setRGB( 0, 0xff, 0 );
        fade(1);
        setRGB( 0, 0xff, 0xff );
        fade(1);
        setRGB( 0, 0, 0xff );
        fade(1);
        setRGB( 0xff, 0, 0xff );
        fade(1);
    }
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  Function call:
 *  [sudo] ./HidCom -s8 -d5 00 00 00 ff 01
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the created report has been stored
 *  [0] = r, [1] = g, [2] = b; [3] = mode (0 glow, 1 blink, 2 fade, ff demo), [4] = duration (in seconds/number of blinks)
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
    HIDReportEcho.ReportID   = ReportID;
    HIDReportEcho.ReportSize = ReportSize;
    memcpy(HIDReportEcho.ReportData, ReportData, ReportSize);

    uint8_t n = HIDReportEcho.ReportData[4];
    setRGB(HIDReportEcho.ReportData[0],HIDReportEcho.ReportData[1],HIDReportEcho.ReportData[2]);
    switch( HIDReportEcho.ReportData[3] )
    {
        case 0:
            on();
            for( uint8_t i = 0; i < n; i++ )
            {
                _delay_ms( 1000 );
            }
            off();
            break;
        case 1:
            blink( n );
            break;
        case 2:
            fade( n );
            break;
        case 0xff:
            demo( n );
            break;
    }        
}
