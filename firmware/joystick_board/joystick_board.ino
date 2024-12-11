#include "Arduino.h"
#include "SPI.h"
#include "NRFLite.h"

const static uint8_t TRANSMITTER_ID = 1;           
const static uint8_t RECEIVER_ID = 0;
const static uint8_t CHANNEL = 100; //used by default, may change later
//IDs and channel are usefull for handling interfierence between more devices
const static uint8_t CE_PIN = 9;
const static uint8_t CSN_PIN = 10;
//MOSI PIN 11
//MISO PIN 12
//SCK  PIN 13
//for  Atmega328p

const static uint8_t A_BUTTON_PIN = 2;
const static uint8_t B_BUTTON_PIN = 3;
const static uint8_t C_BUTTON_PIN = 4;
const static uint8_t D_BUTTON_PIN = 5;
const static uint8_t FIRE_BUTTON_PIN = 6;
const static uint8_t SPECIAL_BUTTON_PIN = 7;

struct TransmitterPacket
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
TransmitterPacket joystick_data;

int main() {
  init();

  Serial.begin(9600); //Serial communication for debugging

  if (!radio.init(TRANSMITTER_ID, CE_PIN, CSN_PIN, NRFLite::BITRATE2MBPS, CHANNEL)) //2MBPS is default, might change later depending on the final usb_RX design
  {
      Serial.println("nrf24 initialization failed");
      while (1);
  }

  pinMode(A_BUTTON_PIN, INPUT_PULLUP);
  pinMode(B_BUTTON_PIN, INPUT_PULLUP);
  pinMode(C_BUTTON_PIN, INPUT_PULLUP);
  pinMode(D_BUTTON_PIN, INPUT_PULLUP);
  pinMode(FIRE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPECIAL_BUTTON_PIN, INPUT_PULLUP);

  joystick_data.transmitter_id = TRANSMITTER_ID;

  while(1)
  {
    clearJoystickData();
    setJoystickData();
    if(!radio.send(RECEIVER_ID, &joystick_data, sizeof(joystick_data), NRFLite::NO_ACK)) //NO_ACK -> I don't want to check if every packet was correctly received
    {
      Serial.println("radio failed to send data");
    }
  }

  return 0;
}

void setJoystickData()
{
  joystick_data.x1_axis = analogRead(A0);
  joystick_data.y1_axis = analogRead(A1);
  joystick_data.x2_axis = analogRead(A2);
  joystick_data.y2_axis = analogRead(A3);

/* Since I have only six buttons on my device, I can use the SPECIAL button
to act as switch between two sets of buttons. So instead of 6 buttons I will
end up with 2*5 buttons. It is important to keep in mind that user can only
use one set of buttons at once.
*/
  for(int i = 0; i < 5; i++)
  {
    int mod_i = i;
    if (digitalRead(SPECIAL_BUTTON_PIN) == 1)
    {
      mod_i += 5;
    }
    if(digitalRead(i + 2) == 0)
    {
      joystick_data.buttons &= ~(1 << mod_i);;
    }
    else
    {
      joystick_data.buttons |= (1 << mod_i);
    }
  }
}

void clearJoystickData()
{
  joystick_data.buttons = 0;
}
