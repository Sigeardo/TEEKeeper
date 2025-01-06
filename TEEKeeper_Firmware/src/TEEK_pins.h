#ifndef TEEK_PINS_H
#define TEEK_PINS_H


// ===== System PINS ==================================================


// Board
#define SYSTEM_ARDUINO_MEGA 
//#define SYSTEN_ARDUINO_UNO

// === SPI interface
#ifdef SYSTEM_ARDUINO_MEGA
    // Arduino MEGA 2560 SPI interface
    #define PIN_SPI_MISO 50
    #define PIN_SPI_MOSI 51
    #define PIN_SPI_CLK 52
    #define PIN_SPI_SS 53
#endif

#ifdef
// Arduino UNO SPI interface
#define PIN_SPI_MISO 12
#define PIN_SPI_MOSI 11
#define PIN_SPI_CLK 13
#define PIN_SPI_SS 10
#endif


// == MAX31855 Select pin
#define PIN_PROBE_CS 17

// == SD Card PINS
#define PIN_SD_CS 18

// == Door interrupt pin
#define PIN_DOOR_INTERRUPT 19

// == Encoder PINS
#define PIN_ENCODER_S1  2
#define PIN_ENCODER_S2  3
#define PIN_ENCODER_KEY 4

// Heater control pin
#define PIN_HEATER 5

#endif