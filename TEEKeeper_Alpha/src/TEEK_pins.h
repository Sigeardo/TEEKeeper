#ifndef TEEK_PINS_H
#define TEEK_PINS_H


// ===== System PINS ==================================================

// === SPI interface
// Arduino MEGA 2560 SPI interface
//#define PIN_SPI_MISO 50
//#define PIN_SPI_MOSI 51
//#define PIN_SPI_SCK 52
//#define PIN_SPI_SS 53


// Arduino UNO SPI interface
//#define PIN_SPI_MISO 12
//#define PIN_SPI_MOSI 11
//#define PIN_SPI_SCK 13
//#define PIN_SPI_SS 10

// == MAX31855 Select pin
#define PIN_PROBE_CS 7

// == SD Card PINS
#define PIN_SD_CS 6

// == Door interrupt pin
#define PIN_DOOR_INTERRUPT 19

// == Encoder PINS
#define PIN_ENCODER_S1  2
#define PIN_ENCODER_S2  3
#define PIN_ENCODER_KEY 4

// Heater control pin
#define PIN_HEATER 5

#endif