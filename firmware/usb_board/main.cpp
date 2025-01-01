#include "Arduino.h"
#include "SPI.h"
#include "NRFLite.h"

const static uint8_t RECEIEVER_ID = 0;
const static uint8_t CHANNEL = 100;

const static uint8_t CE_PIN = 9;
const static uint8_t CSN_PIN = 10;

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

NRFLite radio;
ReceievedPacket joystick_data;

int main()
{
	init();

	Serial.begin(9600);

	if(!radio.init(RECEIEVER_ID, CE_PIN, CSN_PIN, NRFLite::BITRATE2MBPS, CHANNEL))
	{
		Serial.println("nrf24 initialization failed");
		while (1);
	}
	
	while(1)
	{
		if(radio.hasData())
		{
			radio.readData(&joystick_data);
		}
		
		debugPrint();
	}
}

void debugPrint()
{
	Serial.println(radio.x1_axys);
	Serial.println(radio.y1_axys);
	Serial.println(radio.x2_axys);
	Serial.println(radio.y2_axys);
	Serial.println(radio.buttons);
}





