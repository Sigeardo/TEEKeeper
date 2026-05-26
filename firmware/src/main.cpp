
#include "TEEKeeper.h"
#include <arduino.h>

// == I/O 
ClickEncoder __encoder(PIN_ENCODER_S1, PIN_ENCODER_S2, PIN_ENCODER_KEY, ENCODER_STEPS); // Rotary encoder
TFT_HX8357 __screen; // TFT screen

// == TEEKeeper components
ProgramManager      __program;          // Program manager
TemperatureProbe    __probe;            // Temperature sensor
CoreSystem          __core(__probe);    // Core system
SdFat               __sd;               // SD card 
AutotuneParameters  __autPar;           // Autotune parameters wrapper
extern ScreenManager __GUI;            // Screen manager

// == Global variables
char errorStreamChar[ERROR_BUFF_SIZE]; // Error message buffer
char messageStream[ERROR_BUFF_SIZE];   // Message buffer

void setup(){
    TEEK_Setup(); // Setup & initialize the system

    #ifdef SERIAL_COMMS
        Serial.begin(9600); // Initialize the serial communication
    #endif 



    drawBaseScreen(__screen); // Draw the base screen

    __core.ReadTemperature(); // Read the temperature

    __GUI.renderCurrent(__screen); // Render the main menu screen
};

void loop(){

    __core.update(__program);                           // PWM cycle manager
    manageSystemState(__core, __program, __probe);      // Program execution manager
    __GUI.updateGraphics(__core, __screen, __encoder);  // Update the GUI
    delay(1);
};  


