#ifndef TEEK_GRAPHICS_H
#define TEEK_GRAPHICS_H
#ifndef TEEKE_DATASTRUCTURES_H
  // just so the IDE doesn't complain
  #include <TEEKeeper.h>
  #include <Arduino.h>
  #include <TFT_HX8357.h>
  #include <ClickEncoder.h>  
  #include <TEEK_dataStructures.h>  
  #include <TEEK_constants.h>
  #include <SdFat.h>
#endif


extern CoreSystem     __core; // Core system
extern ProgramManager __program; // Program manager
extern TFT_HX8357     __screen; // TFT screen
extern ClickEncoder   __encoder; // Rotary encoder
extern SdFat          __sd; // SD card handler


extern char          errorStreamChar[ERROR_BUFF_SIZE]; // Error message buffer
extern char          messageStream[ERROR_BUFF_SIZE]; // Message buffer

/*
Current menu structure

Main Menu
  > Select from SD
    > list of files
  > Settings
    > Target
    > Unit
    > Pid Autotune
        > confirm?
  > Stop
*/


// === Colors ==================================================
// Colors use the RGB565 format

// TEEK PALETTE
#define TEEK_BLACK  (uint16_t) 0x0000 // #000000
#define TEEK_BLUE   (uint16_t) 0x1107 // #14213D
#define TEEK_YELLOW (uint16_t) 0xFD02 // #FCA311
#define TEEK_SILVER (uint16_t) 0xE73C // #E5E5E5
#define TEEK_WHITE  (uint16_t) 0xFFFF // #FFFFFF

// Auxiliar colors
#define RED   (uint16_t) 0xF800
#define GREEN (uint16_t) 0x07E0

// === Functions ===============================================

void drawBaseScreen(TFT_HX8357& tft);
void drawSoftError(TFT_HX8357& tft);
void drawSoftError(TFT_HX8357& tft, char* message);
void drawCriticalError(TFT_HX8357& tft);
void drawCriticalError(TFT_HX8357& tft, char* message);

short DrawLongMessage(TFT_HX8357& tft, int heigh, int padding, char* message);

// Inline metods for temperature limits conversion
inline double MAX_TEMP(int unit) {
    switch(unit){
        case CELSIUS:
            return MAX_TEMPERATURE;
        case FAHRENHEIT:
            return MAX_TEMPERATURE * 9.0/5.0 + 32;
        case KELVIN:
            return MAX_TEMPERATURE + 273.15;
        default:
            return MAX_TEMPERATURE;
    }
}
#define MAX_TEMPERATURE_IN MAX_TEMP(__core.Unit())

inline double MIN_TEMP(int unit) {
    switch(unit){
        case CELSIUS:
            return MIN_TEMPERATURE;
        case FAHRENHEIT:
            return MIN_TEMPERATURE * 9.0/5.0 + 32;
        case KELVIN:
            return MIN_TEMPERATURE + 273.15;
        default:
            return MIN_TEMPERATURE;
    }
}
#define MIN_TEMPERATURE_IN MIN_TEMP(__core.Unit())


// the screen can display only 38 characters in a line, with 10px padding
  #define MAX_CHAR_PER_LINE 38
  #define LAT_PADDING 10



// ========== MENU SCREENS STRUCTURE ==========
// The handling of the menu screens is done through a series of classes,
// based on the polymorphic approach.
// The base class is the BaseScreen class, which defines the interface for
// all the other menus.
// The other menus are:
// - MainMenuScreen: the main display screen, with the main menu options
// - SettingsMenuScreen: simple settings options
// - FileMenuScreen: list of files in the SD card
// - // TODO ExecutionScreen: screen to display the program execution
// - // TODO TuneScreen: screen to tune the system during execution

class BaseScreen {
public:
  virtual void render(TFT_HX8357& tft) = 0; // Pure virtual function for rendering
  virtual void update(ClickEncoder& encoder, TFT_HX8357& tft) = 0; // Pure virtual function for handling input
  virtual ~BaseScreen() {}
};

// ==== Main menu screen
class MainMenuScreen : public BaseScreen {

private: 
  unsigned long lastUpdateTime = 0;    // Tracks the last time system fields were updated
  const unsigned long updateInterval = MIN_TIME_BETWEEN_SCREEN_UPDATES; // 1000ms update interval
  static const char* menuItems[3];    // Array of menu items
  static const int menuCount = 3;     // Number of menu items
  int menuIndex = 0;                  // Tracks the current menu selection

  void renderMenu(TFT_HX8357& tft, int menuIndex); // Render the menu options
  void handleSelection();                          // Perform action on selection

public:
  MainMenuScreen();
  void render(TFT_HX8357& tft) override;
  void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
};


// ==== Settings menu screen
class SettingsMenuScreen : public BaseScreen {
private: 
  static const char* menuItems[5];    // Array of menu items
  static const int menuCount = 5;     // Number of menu items
  int menuIndex;
  bool isAdjustingTarget = false;
  bool encoderRotated = false;
  int currentUnit = (int) __core.Unit();
  bool confirmPIDautotune = false;

  void renderMenu(TFT_HX8357& tft, int menuIndex); // Render the menu options
  void handleSelection();                          // Perform action on selection
public:
  SettingsMenuScreen();
  //~SettingsMenuScreen();
  void render(TFT_HX8357& tft) override;
  void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
};

// ==== targetUpdateScreen
class TargetUpdateScreen : public BaseScreen {
public:
    TargetUpdateScreen(){};
    void render(TFT_HX8357& tft) override;
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
    void handleSelection();
    
private:
    float targetTemperature;  // Initial target temperature
    bool updatingTarget = false;  // Flag to track if we're in the target setting mode
};

// ==== Critical error screen
class CriticalErrorScreen : public BaseScreen {
public:
  void render(TFT_HX8357& tft) override;
  void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
};


// ==== SD File list screen
class FileMenuScreen : public BaseScreen {
  private:
    char files[MAX_FILES][MAX_FILENAME_LENGTH];  // Fine names array
    int fileCount;  // Total number of files found on the SD card
    int fileIndex;  // Current file index
    int startIndex; // Indexing for rendering
    bool isInitialized = false; // Flag to track if the screen has been initialized
    

    void renderMenu(TFT_HX8357& tft);
    void handleSelection();
public:
  FileMenuScreen();
  void render(TFT_HX8357& tft) override;
  void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
  void Initialise(){isInitialized = false; startIndex = 0;};
};


// ==== Execution screen

class ExecutionScreen : public BaseScreen {

private: 
  unsigned long lastUpdateTime = 0;
  unsigned long lastTimerUpdate = 0;
  double lastTarget = 0;
  unsigned int lastInstructionIndex = 0;

public:
  ExecutionScreen();
  void render(TFT_HX8357& tft) override;
  void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
  void handleSelection();
};

class TuneScreen : public BaseScreen {
  private:
    static const char* menuItems[4];
    int menuCount = 4; // Number of menu items - starting from 0
    int menuIndex = 0;
    bool confirmStop = false;
  public:
    TuneScreen();
    void render(TFT_HX8357& tft) override;
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
    void handleSelection();
};



// ==== Screen manager class
class ScreenManager {
  BaseScreen* currentScreen;
  BaseScreen* previousScreen = nullptr;
  SystemState lastState;
  unsigned long lastUpdateTime = 0;
  unsigned long updateInterval = MIN_TIME_BETWEEN_SCREEN_UPDATES;
public:
  ScreenManager() : currentScreen(new MainMenuScreen), previousScreen(new MainMenuScreen) {lastState = IDLE;};
  void setScreen(BaseScreen* screen);     // Set the new screen to be displayed
  void renderCurrent(TFT_HX8357& tft);    // render the current set screen
  void updateCurrent(ClickEncoder& encoder, TFT_HX8357& tft); // update the screen
  void returnToPrevious();               // return to the previous screen 
  void updateGraphics(CoreSystem& core, TFT_HX8357& tft, ClickEncoder& encoder); // manage inputs
  void deleteCurrentScreen();            // free the memory
  void updatePrevious() {previousScreen = currentScreen;};  // set previous screen
  void clearPrevious() {previousScreen = nullptr;};         // clear the previous screen

/*
  ~ScreenManager() {  // implemented but not used
    delete currentScreen;
    currentScreen = nullptr;
    delete previousScreen;
    previousScreen = nullptr;
  }
*/

}; // Global instance of the screen manager






#endif