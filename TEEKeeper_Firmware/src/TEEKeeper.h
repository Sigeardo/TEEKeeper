#ifndef TEEKEEPER_H
#define TEEKEEPER_H

#include <Arduino.h>
#include <Adafruit_MAX31855.h>  // Temperature probe
#include <SPI.h>                // SPI communication
//#include <SD.h>                 // SD card
#include <SdFat.h>              // SD card
#include <ClickEncoder.h>       // Rotary encoder + push button
#include <EEPROM.h>             // Arduino EEPROM i/o
#include <TFT_HX8357.h>         // TFT screen
#include <TimerOne.h>           // Timer for encoder

#include "TEEK_constants.h"
#include "TEEK_pins.h"
#include "TEEK_dataStructures.h"
#include "TEEK_graphics.h"  // ScreenManager class definition



// == Globals
// buffer for error messages
extern char errorStreamChar[ERROR_BUFF_SIZE]; 

// buffer for messages to be displayed on the screen
extern char messageStream[ERROR_BUFF_SIZE];


extern ProgramManager   __program;  // Program manager
extern TemperatureProbe __probe;    // Temperature probe
extern CoreSystem       __core;     // Core management (PID, PWM, etc.)
extern File *           __file;     // Data log file / generic file pointer
extern TFT_HX8357       __screen;   // TFT screen
extern ClickEncoder     __encoder;  // Rotary encoder
extern SdFat            __sd;       // SD card handler
extern AutotuneParameters   __autPar; // Autotune parameters wrapper




// ============== FUNCTIONS ==============


// -- Program execution
bool manageSystemState(CoreSystem &__core, ProgramManager &__Program, TemperatureProbe &__Probe);
bool programExecution(CoreSystem &__core, ProgramManager &__Program, TemperatureProbe &__Probe);


// -- Log file management
File *createLog();
bool beginLog(File &log, ProgramManager &__prog);
bool updateLog(File &log, const char* name, unsigned long time, double temp, double target, double duty);
bool updateLog(File &log, const char* message, unsigned long time);
bool endLog(File &log);
bool closeLog(File &log, ProgramManager &__prog);


// -- Auxiliary functions
void timeStampConverter(unsigned long millisTime, char* buff, int terms = 3);
bool TEEK_Setup();


// -- Interrupt functions
void doorInterrupt();
void timerIsr();



#endif
