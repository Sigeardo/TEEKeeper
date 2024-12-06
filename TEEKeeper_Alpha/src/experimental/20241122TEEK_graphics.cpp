#include "TEEK_graphics.h"
#include <string.h>



// ===================================================================================================


// This function manages the display of the UI on the screen.
// It is called in the main loop and is responsible for updating the screen
// based on the current menu and user input.
void updateGraphics(Menu& menu, TFT_HX8357& tft) {
  
  int value = __encoder.getValue();
  if (value != 0) {
    menu.lastIndex = menu.Index;
    menu.Index = (menu.Index + value + menu.menuCount) % menu.menuCount; // Wrap around
    updateMenuSelection(tft, menu, value, 0); // Update only the changed items
  }

  // Handle button press
  if (__encoder.getButton() == ClickEncoder::Clicked) {
    executeMenuAction(menu, tft);
  }
};

// ===================================================================================================


// drawBaseScreen is used to fill the screen with the basic UI. 
/*
The screen is divided into 4 parts:
  - Top bar: 40px height, divided into two colors
  - Bottom bar: 20px height, divided into two colors
  - Main screen: 260px height, silver color
  - Top title: 40px height, divided into two colors
  - Bottom title: 20px height, divided into two colors
  - Main title: 260px height, silver color

The coordinate of the central silver screen is (0, 40) to (180, 300)

It is adivsed to leave at least 10px margin from the edge of the screen to the text
*/

void drawBaseScreen(TFT_HX8357& tft){
  
  tft.fillScreen(TEEK_SILVER); // fill the screen with black color
  
  // top bar
  tft.fillRect(0, 0, 180, 40, TEEK_BLUE);
  tft.fillRect(180, 0, 480, 40, TEEK_YELLOW);

  // top title
  tft.setCursor(135, 10);
  tft.setTextColor(TEEK_YELLOW);
  tft.setTextSize(2);
  tft.print("TEE");
  tft.setCursor(182, 10);
  tft.setTextColor(TEEK_BLUE);
  tft.print("Keeper --- EXP");

  // bottom bar
  tft.fillRect(0, 300, 180, 320, TEEK_BLUE);
  tft.fillRect(180, 300, 480, 320, TEEK_YELLOW);
};

// ===================================================================================================

// Updates the base screen with the main menu UI.
// Refer to the INIT_INTF.png image
// The main menu is divided into 4 parts:
//
// Temperature reading: TEMP: xxxx.xx [Unit]
// Current target: xxxx.xx   Status: [ON/OFF]
// 3 selectable options:
//  > Select from SD
//  > Settings 
//  > Stop
// 
// Draw the main menu UI
void drawMainMenu(TFT_HX8357& tft) {
    tft.setTextColor(TEEK_BLACK);

    // fill the screen with the SILVER color
    tft.fillRect(0, 40, 480, 260, TEEK_SILVER);
    
    // The temperature reading is written in big letters, centered 
    tft.setTextSize(3);
    tft.setCursor(30, 50);
    tft.print("TEMP:");
    tft.setTextSize(5);
    tft.setCursor(145, 50);
    tft.print(__core.CurrentTemperature(), 2);
    tft.setTextSize(3);
    switch(__core.Unit()) {
      case 0: tft.print(" [C]"); break;
      case 1: tft.print(" [F]"); break;
      case 2: tft.print(" [K]"); break;
    }

    // Write target and status
    tft.setTextSize(2);
    tft.setCursor(30, 100);
    tft.print("Target:");
    if(__core.TargetTemperature() == 0) tft.print("--");
    else tft.print(__core.TargetTemperature(), 2);

    tft.setCursor(240, 100);
    tft.print("Status: ");
    if(__core.isFiring()) {
      tft.setTextColor(GREEN);
      tft.print("ON");
    }
    else {
      tft.setTextColor(RED);
      tft.print("OFF");
    }

    // Write the menu options
    for (int i = 0; i < mainMenu.menuCount; i++) {
      tft.setCursor(30, 150 + i * 50);
      if (i == mainMenu.Index) {
        tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
      } else {
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      }
      tft.print(mainMenu.menuItems[i]);
    }
}

// ===================================================================================================


// drawSoftError draws the UI for a non critical error that requires 
// user input to be solved.
// It fills the screen with the TEEK_YELLOW colour, reports the error 
// message and asks the user to press the encoder button to continue.
void drawSoftError(TFT_HX8357& tft){

  // fill the screen with the YELLOW color
  tft.fillRect(5, 45, 470, 250, TEEK_YELLOW);
  
  // The "ERROR" warning is written in big letters, centered
  tft.setTextSize(4);
  tft.setTextColor(TEEK_BLACK);
  tft.drawCentreString("ERROR", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(errorStream);
  tft.setCursor(10, 200);
  tft.print("Press the encoder button to continue...");

};


// ===================================================================================================

// drawCriticalError draws the UI for a critical error that may
// endanger the system or the user.
// It fills the screen with the RED colour, reports the error
// message error and asks the user to reset the system.
void drawCriticalError(TFT_HX8357& tft){

  // fill the screen with the RED color
  tft.fillRect(5, 45, 470, 250, RED);
  
  // The "ERROR" warning is written in big letters, centered
  tft.setTextSize(4);
  tft.setCursor(5, 100);
  tft.setTextColor(TEEK_BLACK);
  
  tft.drawCentreString("!!CRITICAL ERROR!!", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(errorStream);
  tft.setCursor(10, 200);
  tft.print("System reset required.");
};






// ===================================================================================================

// drawSettingsMenu draws the UI for the settings menu.
// The structure of the settings menu is:
//  < Back
//  - Target: xxxx.xx 
//  - Unit: [C/F/K]
//  - Auto tune PID
void drawSettingsMenu(TFT_HX8357& tft){
  // clear the screen area
  tft.setTextColor(TEEK_BLACK);
  tft.fillRect(0, 40, 480, 260, TEEK_SILVER);

  // write the back button
  tft.setTextSize(2);
  tft.setCursor(10, 50);
  tft.print("< Back");

  tft.setCursor(10, 100);
  tft.print("- Target: ");
  if(__core.TargetTemperature() == 0) tft.print("--");
  else tft.print(__core.TargetTemperature(), 2);

  tft.setCursor(10, 150);
  tft.print("- Unit: ");
  switch(__core.Unit()){
    case CELSIUS: tft.print("[C]"); break;
    case FAHRENHEIT: tft.print("[F]"); break;
    case KELVIN: tft.print("[K]"); break;
  }

  tft.setCursor(10, 200);
  tft.print("- Auto tune PID");
};


// ===================================================================================================

// Updates the menu selection on the screen, highlighting the row that is currently
// selected.
void updateMenuSelection(TFT_HX8357& tft, Menu& menu, int selected, bool fullRedraw) {
  if (fullRedraw) {
    menu.Reset();
    switch(menu.type){
      case MAIN_MENU:
        drawMainMenu(tft);
        break;
      case SETTINGS_MENU:
        drawSettingsMenu(tft);
        break;

      case SD_MENU:
        //TODO
        break;
    }
  } else {
    // Clear and redraw only the affected menu items
    if (menu.lastIndex != -1) {
      tft.setCursor(30, 150 + menu.lastIndex * 50);
      tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      tft.print(menu.menuItems[menu.lastIndex]);
    }

    // Highlight current selection
    tft.setCursor(30, 150 + selected * 50);
    tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlighted text
    tft.print(menu.menuItems[selected]);
  }
};


// ===================================================================================================


// Handle menu actions
void executeMenuAction(Menu& menu, TFT_HX8357 &tft) {


  switch(menu.type){
    case MAIN_MENU:
      if (menu.Index == 0) {
        if(__sd.begin(PIN_SD_CS)) {
          tft.print("Loading from SD...");
        // if there is a SD card
          // Load file list from SD
          // Prepare the SD menu
          currentMenu = &sdMenu;// Change menu to SD_MENU
          // Draw the new menu
        }
        else {
          tft.setCursor(160, 150);
          tft.setTextColor(TEEK_BLUE, TEEK_YELLOW);
          tft.print("No SD card found!");
          delay(2000);
          drawMainMenu(tft);
        }
      } else if (menu.Index == 1) {
        tft.print("Opening Settings...");
        // Change menu to SETTINGS_MENU
        currentMenu = &settingsMenu;
        // Draw the new menu
      } else if (menu.Index == 2) {
        // If the system is running
          // Stop the system
          // Display message
          // wait for two seconds
          // redraw the main menu
        tft.print("Stopping system...");
        // else
          // do nothing
      }
      break;
    case SETTINGS_MENU:
      if(menu.Index == 0) {
        currentMenu = &mainMenu;
        drawMainMenu(tft);
      } else if (menu.Index == 1) {
        // Change menu to TARGET_MENU
        // Draw the new menu
      } else if (menu.Index == 2) {
        // Change menu to UNIT_MENU
        // Draw the new menu
      } else if (menu.Index == 3) {
        // Change menu to PID_MENU
        // Draw the new menu
      }

      break;
    case SD_MENU:
      //* handle the SD menu
      // if back, go back to main menu
      // if a file is selected, load the program and go to exec menu        
      break;
  }
  delay(200);
};

// ===================================================================================================
