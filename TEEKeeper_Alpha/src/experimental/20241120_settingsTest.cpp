/*
#include "TEEKeeper.h"
#include <arduino.h>

// == I/O 
ClickEncoder __encoder(PIN_ENCODER_S1, PIN_ENCODER_S2, PIN_ENCODER_KEY, ENCODER_STEPS); // Rotary encoder
//TFT_HX8357 __screen; // TFT screen

// == TEEKeeper components
ProgramManager      __program;
TemperatureProbe    __probe;
CoreSystem          __core(__probe);
SDClass             __sd;

File *__file; // Pointer to an SD card file

// == Global variables
String errorStream = "";


/*
void setup(){

    TEEK_Setup(); // Setup & initialize the system

    
    //TODO 
    errorStream = "Sgrogno";

    __probe.Sensor().begin();   // Initialize the temperature probe
    __screen.begin();           // Initialize the TFT screen
    __screen.setRotation(3);    // Set the screen orientation

    __core.setCurrentTemperature(1234.567);


    drawBaseScreen(__screen);   // Draw the base screen
    drawMainMenu(__screen);     // Draw the main menu
    drawSettingsMenu(__screen); // Draw the settings menu
};


void loop(){
    __core.update(__program); // PWM cycle manager
    manageSystemState(__core, __program, __probe); // Program execution manager


    
};  
*/
#include <TFT_HX8357.h>
#include <ClickEncoder.h>

// Initialize TFT screen and encoder
TFT_HX8357 tft = TFT_HX8357();
ClickEncoder encoder(4, 3, 2);  // Encoder pins (4, 3, 2)

void encoderService();
void handleSelection();
void handleNavigation(int8_t encoderValue);
void drawSettingsMenu();



// Define colors
#define TEEK_BLACK    TFT_BLACK
#define TEEK_SILVER   TFT_LIGHTGREY
#define TEEK_BLUE     TFT_BLUE
#define TEEK_YELLOW   TFT_YELLOW
#define GREEN         TFT_GREEN
#define RED           TFT_RED

// Mockup variables for settings
float targetTemperature = 100.0; // Initial target temperature
int currentUnit = 0; // 0 -> Celsius, 1 -> Fahrenheit, 2 -> Kelvin

// Define min/max temperature
#define MIN_TEMP 0
#define MAX_TEMP 1200

// States for the settings menu
enum SettingState {
  SETTINGS_MAIN,
  SETTINGS_TARGET,
  SETTINGS_UNIT,
  SETTINGS_AUTO_TUNE
};

SettingState currentSettingState = SETTINGS_MAIN;
bool targetEditing = false;  // Flag to handle if target is being edited
bool autoTuneActive = false; // Flag for auto-tune status

void setup() {
  tft.begin();
  tft.setRotation(3);  // Set the screen rotation
  encoder.setAccelerationEnabled(true);  // Enable encoder acceleration
  drawSettingsMenu();
}

void loop() {
  encoderService();
}

// Handle encoder input
void encoderService() {
  // Check if encoder button is clicked
  int btn = encoder.getButton();
  if (btn == ClickEncoder::Clicked) {
    handleSelection();
  }

  // Handle encoder rotation for navigation
  encoder.service();
  int8_t enc = encoder.getValue();
  if (enc != 0) {
    handleNavigation(enc);
  }
}

// Handle encoder button click actions
void handleSelection() {
  switch (currentSettingState) {
    case SETTINGS_MAIN:
      // Click on "< Back" will go to the main menu
      if (encoder.getButton() == ClickEncoder::Clicked) {
        // Add your logic to return to the main menu here
        drawBaseScreen(tft);
        drawMainMenu(tft);
      }
      break;

    case SETTINGS_TARGET:
      // Update target value when clicked
      if (targetEditing) {
        // Finalize target editing and update the system
        targetEditing = false;
        // Update target temperature on the screen
        drawSettingsMenu();
      } else {
        // Start editing the target value (allow input)
        targetEditing = true;
      }
      break;

    case SETTINGS_UNIT:
      // Rotate through temperature units (Celsius, Fahrenheit, Kelvin)
      if (encoder.getButton() == ClickEncoder::Clicked) {
        currentUnit = (currentUnit + 1) % 3;  // Cycle through 0 -> 1 -> 2 -> 0
        drawSettingsMenu();
      }
      break;

    case SETTINGS_AUTO_TUNE:
      // Start Auto-Tune PID (just mock it for now)
      if (encoder.getButton() == ClickEncoder::Clicked) {
        autoTuneActive = !autoTuneActive;
        drawSettingsMenu();
      }
      break;
  }
}

// Handle encoder rotation for navigating between settings
void handleNavigation(int8_t encoderValue) {
  // Navigate between fields
  if (encoderValue == 1) {
    // Move down (select next option)
    switch (currentSettingState) {
      case SETTINGS_MAIN:
        currentSettingState = SETTINGS_TARGET;
        break;
      case SETTINGS_TARGET:
        currentSettingState = SETTINGS_UNIT;
        break;
      case SETTINGS_UNIT:
        currentSettingState = SETTINGS_AUTO_TUNE;
        break;
      case SETTINGS_AUTO_TUNE:
        currentSettingState = SETTINGS_MAIN;
        break;
    }
  } else if (encoderValue == -1) {
    // Move up (select previous option)
    switch (currentSettingState) {
      case SETTINGS_MAIN:
        currentSettingState = SETTINGS_AUTO_TUNE;
        break;
      case SETTINGS_TARGET:
        currentSettingState = SETTINGS_MAIN;
        break;
      case SETTINGS_UNIT:
        currentSettingState = SETTINGS_TARGET;
        break;
      case SETTINGS_AUTO_TUNE:
        currentSettingState = SETTINGS_UNIT;
        break;
    }
  }
  drawSettingsMenu();
}

// Draw the settings menu
void drawSettingsMenu() {
  // Clear the screen area
  tft.setTextColor(TEEK_BLACK);
  tft.fillRect(0, 40, 480, 260, TEEK_SILVER);

  // Draw the back button
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.print("< Back");

  // Draw the Target field
  tft.setCursor(10, 100);
  tft.print("- Target: ");
  if (targetEditing) {
    // Mockup: Show edit mode for target temperature
    tft.setTextSize(3);
    tft.setCursor(120, 100);
    tft.print("Edit Mode");
  } else {
    // Show current target temperature
    tft.setTextSize(3);
    tft.setCursor(120, 100);
    if (targetTemperature == 0) {
      tft.print("--");
    } else {
      tft.print(targetTemperature, 2);
    }
  }

  // Draw the Unit field
  tft.setCursor(10, 150);
  tft.print("- Unit: ");
  switch (currentUnit) {
    case 0:
      tft.print("[C]");
      break;
    case 1:
      tft.print("[F]");
      break;
    case 2:
      tft.print("[K]");
      break;
  }

  // Draw Auto Tune PID button
  tft.setCursor(10, 200);
  tft.print("- Auto tune PID");
  if (autoTuneActive) {
    tft.setTextColor(GREEN);
    tft.print(" (Active)");
  } else {
    tft.setTextColor(RED);
    tft.print(" (Inactive)");
  }
}

*/