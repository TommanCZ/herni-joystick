#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "../lib/arduino/Arduino.h"
#include "SPI.h"
#include "../lib/nrflite/NRFLite.h"

const static uint8_t TRANSMITTER_ID = 1;           
const static uint8_t RECEIVER_ID = 0;
const static uint8_t CHANNEL = 100; //used by default, may change later
//IDs and channel are usefull for handling interfierence between more devices
const static uint8_t CE_PIN = 2;
const static uint8_t CSN_PIN = 3;
//MOSI PIN 11
//MISO PIN 12
//SCK  PIN 13
//for  Atmega328p

#define A_BUTTON_PIN PD2
#define B_BUTTON_PIN PD3
#define C_BUTTON_PIN PD4
#define D_BUTTON_PIN PD5
#define FIRE_BUTTON_PIN PD6
#define SPECIAL_BUTTON_PIN PD7

void setup_pins();
void setup_adc();
uint16_t read_adc();
void setJoystickData();
void clearJoystickData();

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

int main() 
{
  setup_pins();
  setup_adc();
  init();

  Serial.begin(9600); //Serial communication for debugging

  if (!radio.init(TRANSMITTER_ID, CE_PIN, CSN_PIN, NRFLite::BITRATE2MBPS, CHANNEL)) //2MBPS is default, might change later depending on the final usb_board design
  {
      while (1);
      Serial.println("radio failed to initalize");
      //here something for debbuging later
      //set firs led on!
  }

  joystick_data.transmitter_id = TRANSMITTER_ID;

  while(1)
  {
    clearJoystickData();
    setJoystickData();
    if(!radio.send(RECEIVER_ID, &joystick_data, sizeof(joystick_data), NRFLite::NO_ACK)) //NO_ACK -> I don't want to check if every packet was correctly received
    {
      Serial.println("radio failed to send data");
      //set second led on
    }
    else
    {
	//set thir led on
    }
  }

  return 0;
}

void setup_pins() {
    // Set button pins as inputs
    DDRD &= ~((1 << A_BUTTON_PIN) | (1 << B_BUTTON_PIN) | (1 << C_BUTTON_PIN) | (1 << D_BUTTON_PIN) | (1 << FIRE_BUTTON_PIN) | (1 << SPECIAL_BUTTON_PIN));
    // Enable pull-up resistors
    PORTD |= (1 << A_BUTTON_PIN) | (1 << B_BUTTON_PIN) | (1 << C_BUTTON_PIN) | (1 << D_BUTTON_PIN) | (1 << FIRE_BUTTON_PIN) | (1 << SPECIAL_BUTTON_PIN);
}

void setup_adc() {
    // Set ADC reference voltage to AVcc
    ADMUX |= (1 << REFS0);
    // Enable ADC and set prescaler to 128 (for 16 MHz clock, ADC clock = 125 kHz)
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t read_adc(uint8_t channel) {
    // Select ADC channel (0-7) by writing to ADMUX
    ADMUX = (ADMUX & 0xF8) | (channel & 0x07);
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));
    // Return ADC result
    return ADC;
}

void setJoystickData()
{
  joystick_data.x1_axis = read_adc(0) * 64;
  joystick_data.y1_axis = read_adc(1) * 64;
  joystick_data.x2_axis = read_adc(2) * 64;
  joystick_data.y2_axis = read_adc(3) * 64;

/* Since I have only six buttons on my device, I can use the SPECIAL button
to act as switch between two sets of buttons. So instead of 6 buttons I will
end up with 2*5 buttons. It is important to keep in mind that user can only
use one set of buttons at once.
*/
  for(int i = 0; i < 5; i++)
  {
    int mod_i = i;
    if (PIND & (1 << SPECIAL_BUTTON_PIN)) //asks if SPECIAL BUTTON is 1
    {
      mod_i += 5; //change the bit position by 5 to change the second set of buttons
    }
    if(PIND & (1 << (i + 2))) //asks if a currently itarated pin is 1
    {
      joystick_data.buttons |= (1 << mod_i); //sets pin in buttons corresponding to the physical to 1
    }
    else
    {
      joystick_data.buttons &= ~(1 << mod_i); //sets pin in buttons to 0
    }
  }
}

void clearJoystickData()
{
  joystick_data.buttons = 0;
}
