#include"Arduino.h"
#include "SPI.h"
#include "NRFLite.h"

const static uint8_t TRANSMITTER_ID = 1;           
const static uint8_t RECEIVER_ID = 0;
const static uint8_t CHANNEL = 100; //used by default, may change later
const static uint8_t CE_PIN = 9;
const static uint8_t CSN_PIN = 10;
//MOSI 11
//MISO 12
//SCK  13
//for  Atmega328p

const static uint8_t A_BUTTON_PIN = 2;
const static uint8_t B_BUTTON_PIN = 3;
const static uint8_t C_BUTTON_PIN = 4;
const static uint8_t D_BUTTON_PIN = 5;
const static uint8_t FIRE_BUTTON_PIN = 6;
const static uint8_t SPECIAL_BUTTON_PIN = 7;

struct TransmitterPacket
{
  uint8_t transmitter_id;
  uint16_t x1_axys;
  uint16_t y1_axys;
  uint16_t x2_axys;
  uint16_t y2_axys;
  uint16_t buttons;
};


NRFLite radio;
TransmitterPacket joystick_data;

int main() {
  init();

  Serial.begin(9600); //Serial communication for debugging

  if (!radio.init(TRANSMITTER_ID, CE_PIN, CSN_PIN, NRFLite::BITRATE2MBPS, CHANNEL)) //2MBPS is default, might change later depending on the final usb_RX design
  {
      Serial.println("nrf24 initialization failed - (line 40 for now)");
      while (1); // Wait here forever.
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
    if(!radio.send(RECEIVER_ID, &joystick_data, sizeof(joystick_data), NRFLite::NO_ACK)) //NO_ACK -> I don!t want to check if every packet was correctly received
    {
      Serial.println("radio failed to send data - line 62");
    }
  }

  return 0;
}

void setJoystickData()
{
  joystick_data.x1_axys = analogRead(A0);
  joystick_data.y1_axys = analogRead(A1);
  joystick_data.x2_axys = analogRead(A2);
  joystick_data.y2_axys = analogRead(A3);

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
  joystick_data.x1_axys = 512; //midlle value of 1024 (2^10 -> sensitivity of analog pin)
  joystick_data.y1_axys = 512;
  joystick_data.x2_axys = 512;
  joystick_data.y2_axys = 512;
  joystick_data.buttons = 0;
}
