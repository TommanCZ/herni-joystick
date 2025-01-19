#include <avr/io.h>
#include <util/delay.h>

#include "../lib/arduino/Arduino.h"
#include "SPI.h"
#include "../lib/nrflite/NRFLite.h"
extern "C" 
{
	#include "../lib/usbdrv/usbdrv.h"
}


const static uint8_t RECEIEVER_ID = 0;
const static uint8_t CHANNEL = 100;

const static uint8_t CE_PIN = 9;
const static uint8_t CSN_PIN = 10;

PROGMEM const char usbHidReportDescriptor[50] = 
{
    (char)0x05, (char)0x01,        // Usage Page (Generic Desktop)
    (char)0x09, (char)0x04,        // Usage (Joystick)
    (char)0xA1, (char)0x01,        // Collection (Application)
    
    // Axes
    (char)0x05, (char)0x01,        //   Usage Page (Generic Desktop)
    (char)0x09, (char)0x30,        //   Usage (X)
    (char)0x09, (char)0x31,        //   Usage (Y)
    (char)0x09, (char)0x32,        //   Usage (Z)
    (char)0x09, (char)0x33,        //   Usage (Rx)
    (char)0x15, (char)0x00,        //   Logical Minimum (0)
    (char)0x26, (char)0xFF, (char)0xFF,  //   Logical Maximum (65535)
    (char)0x75, (char)0x10,        //   Report Size (16)
    (char)0x95, (char)0x04,        //   Report Count (4)
    (char)0x81, (char)0x02,        //   Input (Data, Variable, Absolute)

    // Buttons
    (char)0x05, (char)0x09,        //   Usage Page (Button)
    (char)0x19, (char)0x01,        //   Usage Minimum (Button 1)
    (char)0x29, (char)0x0A,        //   Usage Maximum (Button 10)
    (char)0x15, (char)0x00,        //   Logical Minimum (0)
    (char)0x25, (char)0x01,        //   Logical Maximum (1)
    (char)0x75, (char)0x01,        //   Report Size (1)
    (char)0x95, (char)0x0A,        //   Report Count (10)
    (char)0x81, (char)0x02,        //   Input (Data, Variable, Absolute)
    (char)0x75, (char)0x06,        //   Report Size (6) (Padding for alignment)
    (char)0x95, (char)0x01,        //   Report Count (1)
    (char)0x81, (char)0x03,        //   Input (Constant, Variable, Absolute)

    (char)0xC0               // End Collection
};

typedef struct
{
	//joystick is expected to have 4 axis and 10 butons
	uint8_t transmitter_id;
	uint16_t x1_axis;
	uint16_t y1_axis;
	uint16_t x2_axis;
	uint16_t y2_axis;
	uint16_t buttons;
}received_packet;

typedef struct 
{
	uint16_t x1_axis;
	uint16_t y1_axis;
	uint16_t x2_axis;
	uint16_t y2_axis;
	uint16_t buttons;
}report_data;

NRFLite radio;
static received_packet joystick_data;

static report_data reportBuffer;
static uchar idleRate;

void initializeUSB()
{
    usbInit();                 // initialize library
    usbDeviceDisconnect();     // Force re-enumeration
    _delay_ms(250);            
    usbDeviceConnect();
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) 
{
    usbRequest_t *rq = (usbRequest_t *)(void *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS)
    {
        if (rq->bRequest == USBRQ_HID_GET_REPORT) 
	{
            usbMsgPtr = (unsigned int)(void *)&reportBuffer; // Point to the HID report
            return sizeof(reportBuffer);
        } 
	else if (rq->bRequest == USBRQ_HID_GET_IDLE) 
	{
            usbMsgPtr = (unsigned int)(void *)&idleRate; // Return the idle rate
            return 1;
        } 
	else if (rq->bRequest == USBRQ_HID_SET_IDLE) 
	{
            idleRate = rq->wValue.bytes[1]; // Set the idle rate
        }
    }
    return 0; // Unsupported request, return no data
}

void updateReport()
{
	reportBuffer.x1_axis = joystick_data.x1_axis;
	reportBuffer.y1_axis = joystick_data.y1_axis;
	reportBuffer.x2_axis = joystick_data.x2_axis;
	reportBuffer.y2_axis = joystick_data.y2_axis;
	reportBuffer.buttons = joystick_data.buttons;
}

void debugPrint()
{
	/*
	Serial.println(joystick_data.x1_axis);
	Serial.println(joystick_data.y1_axis);
	Serial.println(joystick_data.x2_axis);
	Serial.println(joystick_data.y2_axis);
	Serial.println(joystick_data.buttons);
	*/
}

int main()
{
	init();
	initializeUSB();
	sei();

	Serial.begin(9600);

	if(!radio.init(RECEIEVER_ID, CE_PIN, CSN_PIN, NRFLite::BITRATE2MBPS, CHANNEL))
	{
		Serial.println("nrf24 initialization failed");
		while (1);
	}
	
	while(1)
	{
		usbPoll();
		if(radio.hasData())
		{
			radio.readData(&joystick_data);
		}

		updateReport();

		if(usbInterruptIsReady())
		{
            		usbSetInterrupt((unsigned char *)&reportBuffer, sizeof(reportBuffer));
		}
		debugPrint();
	}
}

