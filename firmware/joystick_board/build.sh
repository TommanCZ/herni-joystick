#!/bin/sh
mkdir -p build
rm -r ./build/*
cd ./build
avr-gcc -mmcu=atmega328p -DF_CPU=8000000UL -Os -I../lib/arduino -c ../lib/arduino/*.{c,S}
avr-g++ -mmcu=atmega328p -DF_CPU=8000000UL -Os -I../lib/arduino -c ../lib/arduino/*.cpp
avr-g++ -mmcu=atmega328p -DF_CPU=8000000UL -Os -I../lib/arduino -I../lib/nrflite -c ../lib/nrflite/NRFLite.cpp
avr-g++ -mmcu=atmega328p -DF_CPU=8000000UL -Os -I../lib/arduino -I../lib/nrflite -I. -c ../src/main.cpp
avr-g++ -mmcu=atmega328p -Os *.o -o main.elf
avr-objcopy -O ihex -R .eeprom main.elf main.hex
