#include <avr.io.h>
#include <util/delay.h>

#include "../lib/arduino/Arduino.h"
#include "SPI.h"
#include "../lib/nrflite/NRFLite.h"



const static uint8_t RECEIEVER_ID = 0;
const static uint8_t CHANNEL = 100;

const static uint8_t CE_PIN = 9;
const static uint8_t CSN_PIN = 10;

PROGMEM const char usbHidReportDescriptor[50] = 
{
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)
    
    // Axes
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x33,        //   Usage (Rx)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0xFF,  //   Logical Maximum (65535)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)

    // Buttons
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x0A,        //   Usage Maximum (Button 10)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0A,        //   Report Count (10)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    0x75, 0x06,        //   Report Size (6) (Padding for alignment)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Constant, Variable, Absolute)

    0xC0               // End Collection
}

struct ReceievedPacket
{
	//joystick is expected to have 4 axis and 10 butons
	uint8_t transmitter_id;
	uint16_t x1_axis;
	uint16_t y1_axis;
	uint16_t x2_axis;
	uint16_t y2_axis;
	uint16_t buttons;
};

typedef struct 
{
	uint16_t x1_axis;
	uint16_t y1_axis;
	uint16_t x2_axis;
	uint16_t y2_axis;
	uint16_t buttons;
}repor_data;

NRFLite radio;
ReceievedPacket joystick_data;

static report_data reportBuffer;
static uchar idleRate;

void initializeUSB()
{
    usbInit();                 // initialize library
    usbDeviceDisconnect();     // Force re-enumeration
    _delay_ms(250);            
    usbDeviceConnect();
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
    usbRequest_t *rq = (void *)data;

    if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
        if (rq->bRequest == USBRQ_HID_GET_REPORT) {
            usbMsgPtr = (void *)&reportBuffer; // Point to the HID report
            return sizeof(reportBuffer);
        } else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
            usbMsgPtr = &idleRate; // Return the idle rate
            return 1;
        } else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
            idleRate = rq->wValue.bytes[1]; // Set the idle rate
        }
    }
    return 0; // Unsupported request, return no data
}

void updateReport()
{
	reportBuffer.x1_axys = radio.x1_axys;
	reportBuffer.y1_axys = radio.y1_axys;
	reportBuffer.x2_axys = radio.x2_axys;
	reportBuffer.y2_axys = radio.y2_axys;
	reportBuffer.buttons = radio.buttons;
}

void debugPrint()
{
	Serial.println(joystick_data.x1_axis);
	Serial.println(joystick_data.y1_axis);
	Serial.println(joystick_data.x2_axis);
	Serial.println(joystick_data.y2_axis);
	Serial.println(joystick_data.buttons);
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
            		usbSetInterrupt((void *)&reportBuffer, sizeof(reportBuffer));
		}
		debugPrint();
	}
}

