#include "TEEK_graphics.h"
#include <SdFat.h>
ScreenManager       __GUI;                // GUI handler 

// static allocation of the screens
MainMenuScreen      __mainMenuScreen;       // Main menu
FileMenuScreen      __fileMenuScreen;       // > Load from SD
SettingsMenuScreen  __settingsMenuScreen;   // > Settings
TargetUpdateScreen  __targetUpdateScreen;   // > Settings >> Target update functionality

ExecutionScreen     __executionScreen;      // Program execution screen
TuneScreen          __tuneScreen;           // Execution tuning screen

CriticalErrorScreen __criticalErrorScreen;  // Critical error screen




//* 0. Functions % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % 
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

    

}

// --------------------------------------------------------------------------------

// drawSoftError draws the UI for a non critical error 
// TODO that may require user intervention.
// It fills the screen with the TEEK_YELLOW colour, reports the error 
// message and asks the user to press the encoder button to continue.
void drawSoftError(TFT_HX8357& tft){

  // fill the screen with the YELLOW color
  tft.fillRect(5, 45, 470, 250, TEEK_YELLOW);
  
  // The "ERROR" warning is written in big letters, centered
  tft.setTextSize(4);
  tft.setTextColor(TEEK_BLACK);
  tft.drawCentreString((char *)"ERROR", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(errorStreamChar);
  tft.setCursor(10, 200);

  // clean the error message buffer
  sprintf(errorStreamChar, " ");

  delay(3000); 
};

void drawSoftError(TFT_HX8357& tft, char* message){

  // fill the screen with the YELLOW color
  tft.fillRect(5, 45, 470, 250, TEEK_YELLOW);
  
  // The "ERROR" warning is written in big letters, centered
  tft.setTextSize(4);
  tft.setTextColor(TEEK_BLACK);
  tft.drawCentreString((char *)"ERROR", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(message);
  tft.setCursor(10, 200);

  delay(3000);
};

// --------------------------------------------------------------------------------


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
  
  tft.drawCentreString((char *)"!!CRITICAL ERROR!!", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(errorStreamChar);
  tft.setCursor(10, 200);
  tft.print("System reset required.");
};

void drawCriticalError(TFT_HX8357& tft, char* message){

  // fill the screen with the RED color
  tft.fillRect(5, 45, 470, 250, RED);
  
  // The "ERROR" warning is written in big letters, centered
  tft.setTextSize(4);
  tft.setCursor(5, 100);
  tft.setTextColor(TEEK_BLACK);
  
  tft.drawCentreString((char *)"!!CRITICAL ERROR!!", 240, 100, 1);

  // The error message is written in smaller letters, centered
  tft.setTextSize(2);
  tft.setCursor(10, 150);
  tft.print(message);
  tft.setCursor(10, 200);
  tft.print("System reset required.");
};

//* % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % 
//
//*                         MENU SCREENS STRUCTURE
//
//* % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %

//* 1. Main Menu Screen =====================================================================
// Definition outside the class
const char* MainMenuScreen::menuItems[3] = {"> Select from SD", "> Settings", "> Stop"};

// Constructor
MainMenuScreen::MainMenuScreen() : menuIndex(0) {};

// Render the main menu
void MainMenuScreen::render(TFT_HX8357& tft) {
  drawBaseScreen(tft);
  // fill the screen with the SILVER color
  tft.fillRect(0, 40, 480, 260, TEEK_SILVER);
  
  // The temperature reading is written in big letters, centered 
  tft.setTextSize(3);
  tft.setCursor(30, 50);
  tft.print("TEMP:");
  tft.setTextSize(5);
  tft.setCursor(145, 50);
  tft.print(__core.CurrentTemperature(), 2);
  tft.setCursor(360,50);
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
  else tft.print(__core.TargetTemperature(), 0);

  tft.setCursor(240, 100);
  tft.print("Status: ");
  if(__core.isFiring()&&__core.isFiringAllowed()) {
    tft.setTextColor(GREEN);
    tft.print("ON");
  }
  else {
    tft.setTextColor(RED);
    tft.print("OFF");
  }

  // print the main menu options
  tft.setTextSize(2);
  tft.setCursor(30, 150);
  for (int i = 0; i < menuCount; i++) {
    tft.setCursor(30, 150 + i * 50); // Adjust position for each item
    if (i == menuIndex) {
      tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
    } else {
      tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
    }
    tft.print(menuItems[i]); // Print the menu item
  }

  tft.setTextColor(TEEK_BLACK);
};


// Handle rotary encoder inputs
void MainMenuScreen::handleSelection(){
  switch (menuIndex) {
    case 0: // "> Select from SD"

      // if there is an SD card, go to the file menu
      if (__sd.begin(PIN_SD_CS)) {
        __fileMenuScreen.Initialise();
        __GUI.setScreen(&__fileMenuScreen);
      } else {
        // otherwise, throw soft error
        drawSoftError(__screen,(char *) "ERROR: SD card not found.");
        delay(2000);
        __GUI.setScreen(&__mainMenuScreen);
      }
      break;
    case 1: // "> Settings"
      __GUI.setScreen(&__settingsMenuScreen);
      break;
    case 2: // "> Stop"
      __core.stopFiring();  // Stop firing
      __core.setTarget(0);  // Set target to 0
      __program.clearProgram();   // Reset the program
      render(__screen); // Return to the main menu
      break;
    default:
      render(__screen); // Invalid selection, refresh the screen
    break;
  }
};

void MainMenuScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
  
  int encoderValue = encoder.getValue();
  if (encoderValue != 0) {
    menuIndex = (menuIndex + encoderValue + menuCount) % menuCount; // Wrap-around menu navigation
    // update the highlighted menu item
    tft.setTextSize(2);
    tft.setCursor(30, 150);
    for (int i = 0; i < menuCount; i++) {
      tft.setCursor(30, 150 + i * 50); // Adjust position for each item
      if (i == menuIndex) {
        tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
      } else {
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      }
      tft.print(menuItems[i]); // Print the menu item
    }

    // reset the correct text color just to be sure
    tft.setTextColor(TEEK_BLACK);
  }
  
  // Update the fields at a fixed interval
  if(millis()-lastUpdateTime > updateInterval) {
    // update the temperature reading
    tft.fillRect(145, 50, 200, 40, TEEK_SILVER);
    tft.setTextSize(5);
    tft.setCursor(145, 50);
    tft.setTextColor(TEEK_BLUE, TEEK_SILVER);
    tft.print(__core.CurrentTemperature(), 2);
    tft.setTextSize(3);
    lastUpdateTime = millis();

      // update system status ASAP
    tft.setTextSize(2);
    tft.setCursor(240, 100);
    tft.print("Status: ");
    if(__core.IsOn()) {
      tft.setTextColor(GREEN, TEEK_SILVER);
      tft.print("ON ");
    }
    else {
      tft.setTextColor(RED, TEEK_SILVER);
      tft.print("OFF");
    }
    tft.setTextColor(TEEK_BLUE);
  }

  // Handle encoder button press
  if (encoder.getButton() == ClickEncoder::Clicked) {
    handleSelection(); // Perform action based on the current selection
  }

  
  
};

//* 2. SettingsMenuScreen Implementation ==================================================

const char* SettingsMenuScreen::menuItems[5] = {"< Back", "> Target: ", "> Unit: ", "> PID Autotune", "> Keep log: "};

SettingsMenuScreen::SettingsMenuScreen() : menuIndex(0) {};

/*
// Destructor
SettingsMenuScreen::~SettingsMenuScreen() {
    for (int i = 0; i < menuCount; i++) {
        free(menuItems[i]); // Free dynamically allocated memory
    }
};
*/

void SettingsMenuScreen::render(TFT_HX8357& tft) {
  // fill the screen with the SILVER color
  tft.fillRect(0, 40, 480, 300, TEEK_SILVER);

  // menu title
  tft.setTextColor(TEEK_BLUE, TEEK_SILVER);
  tft.setTextSize(3);
  tft.setCursor(30, 50);
  tft.print("Settings:");

  // menu options
  for(int i = 0; i < menuCount; i++) {
    tft.setCursor(30, 100 + i * 35);
    if (i == menuIndex) {
      tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
    } else {
      tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
    }
    tft.print(menuItems[i]); // Print the menu item
    switch(i){
    case 1: // "> Target: xxxx.xx"
      if(__core.TargetTemperature() == 0) tft.print(MIN_TEMPERATURE);
      else tft.print(__core.TargetTemperature(), 0);
      break;
    case 2: // "> Unit: [C/F/K]"
      tft.print(__core.getTextUnit());
      break;
    case 3: // "> PID Autotune"
      if(confirmPIDautotune) {
        tft.setCursor(320, 100 + i * 50);
        tft.setTextColor(TEEK_SILVER, RED);
        tft.print("Confirm?");
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
      }
      break;
    case 4: // "> Keep log: [Y/N]"
      if(__core.KeepLog()) tft.print("Yes");
      else tft.print("No ");
      break;
   } 
  }
};


void SettingsMenuScreen::handleSelection() {
  switch (menuIndex) {
    case 0: // "< Back"
      __GUI.setScreen(&__mainMenuScreen);  // Go back to the main menu
      break;

    case 1: // "> Target: xxxx.xx"
      __GUI.updatePrevious();
      __GUI.setScreen(&__targetUpdateScreen); // Go to the target update screen
      break;

    case 2: // "> Unit: [C/F/K]"
      currentUnit = (currentUnit + 1) % 3; // Cycle through the temperature units
      __core.setUnit((TemperatureUnit)currentUnit); // Update the unit in the core system
      __core.setTarget(MIN_TEMPERATURE); // Erase the target temperature
      render(__screen); // Refresh the screen
      break;

    case 3: // "> PID Autotune"
      
      if(confirmPIDautotune) {
        __core.PIDAutotune(); // Start the PID autotune process
        __GUI.setScreen(&__executionScreen); // move to the execution screen
        
        } else {
        confirmPIDautotune = true; // Set the flag to confirm PID autotune
        render(__screen); // Refresh the screen
      }
      break;
    case 4: // "> Keep log: [Y/N]"
      __core.setKeepLog(!__core.KeepLog()); // Toggle the keep log flag
      render(__screen); // Refresh the screen
      break;

    default:
      render(__screen); // Invalid selection, refresh the screen
      break;
  }

  // Reset the PID autotune confirmation flag
  if(menuIndex != 3 && confirmPIDautotune) {
    confirmPIDautotune = false;
  }
};

void SettingsMenuScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
  int encoderValue = encoder.getValue();
  if (encoderValue != 0) {
    menuIndex = (menuIndex + encoderValue + menuCount) % menuCount; // Wrap-around menu navigation
    // update the highlighted menu item
    tft.setTextSize(3);
    tft.setCursor(30, 150);
    for (int i = 0; i < menuCount; i++) {
      tft.setCursor(30, 100 + i * 35); // Adjust position for each item
      if (i == menuIndex) {
        tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
      } else {
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      }
      tft.print(menuItems[i]); // Print the menu item
    }

    // reset the correct text color just to be sure
    tft.setTextColor(TEEK_BLACK);

  }

  // Handle encoder button press
  if (encoder.getButton() == ClickEncoder::Clicked) {
    handleSelection(); // Perform action based on the current selection
  }
}


//* 3. TargetUpdateScreen Implementation ===================================================

void TargetUpdateScreen::render(TFT_HX8357& tft) {
    // rewrite only the current selected object:
    if(updatingTarget == false){
      if(__core.TargetTemperature() != 0) targetTemperature = __core.TargetTemperature();
      else targetTemperature = MIN_TEMPERATURE;

      updatingTarget = true;
    }
    tft.setCursor(210, 135);
    tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
    tft.setTextSize(3);
    tft.print("  ");
    tft.print(targetTemperature, 0);
    tft.print("  ");
    tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
}

void TargetUpdateScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    
    int encoderValue = encoder.getValue();  // Get the encoder value
    if (encoderValue != 0) {  // if we had a rotation
        if (updatingTarget) {
            targetTemperature += encoderValue;  // Adjust the target temperature 

            // Ensure the target temperature is within the valid range
            if (targetTemperature < MIN_TEMPERATURE_IN) targetTemperature = MIN_TEMPERATURE_IN;
            if (targetTemperature > MAX_TEMPERATURE_IN) targetTemperature = MAX_TEMPERATURE_IN;

            render(tft);  // Re-render the screen to show the updated target temperature
        }
    }

    // If encoder button is pressed, save the target and return to the main menu
    if (encoder.getButton() == ClickEncoder::Clicked) {
        __core.setTarget(targetTemperature);      // Set the new target temperature
        targetTemperature = MIN_TEMPERATURE;      // Reset the target temperature
        updatingTarget = false;                   // Reset the updating flag
        __core.startFiring();                     // Start firing
        __GUI.returnToPrevious();                 // Return to the parent screen
        //__GUI.setScreen(&__settingsMenuScreen);   // Return to the main menu
    }
}



//* 4. CriticalErrorScreen Implementation ===================================================
void CriticalErrorScreen::render(TFT_HX8357& tft) {
  extern CoreSystem __core;
  __core.denyFiring();
  digitalWrite(PIN_HEATER, LOW);

  if(errorStreamChar[0] != '\0') {
    drawCriticalError(tft, errorStreamChar);
  } else {
    drawCriticalError(tft);
  }
}

void CriticalErrorScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
  delay(10);
};

//* 5. FileMenuScreen Implementation ========================================================

FileMenuScreen::FileMenuScreen() {
  fileCount = 0;
  fileIndex = 0;
};


void FileMenuScreen::render(TFT_HX8357& tft) {

  // If the file list is not initialized
  if (!isInitialized) {
    // Clear the files array
    for (int i = 0; i < MAX_FILES; i++) {
      files[i][0] = '\0';
    }

    fileCount = 0; // Reset the file count

    // Open the root directory of the SD card using SdFat
    FsFile root = __sd.open("/");

    if (root.isOpen()) { // Loop through all files in the root directory

      while (true) { // Loop freely and exit with a break

        FsFile entry = root.openNextFile();
        if (!entry) break;  // Exit the loop when no more files are found

        // Only add files (ignore directories)
        if (!entry.isDirectory()) {
          if (fileCount < MAX_FILES) {
            char buff[MAX_FILENAME_LENGTH]; // Temporary buffer to store the file name
            entry.getName(buff, MAX_FILENAME_LENGTH - 1); // get the file name
            strcpy(files[fileCount], buff); // copy the file name to the array
            files[fileCount][MAX_FILENAME_LENGTH - 1] = '\0';  // force null termination 
            fileCount++; // update the file count
          }
        }
        entry.close(); // Close the file
      }
      root.close(); // Close the root directory
    } else {
      // Handle SD root directory open failure
      drawSoftError(tft, (char *) "Unable to open SD card.");
      delay(2000);
      __GUI.setScreen(&__mainMenuScreen);
      return;
    }

    // Mark the initialization as complete
    isInitialized = true;
  }
  
  // Fill the screen with the SILVER color (only the content area)
  tft.fillRect(0, 40, 480, 300, TEEK_SILVER);

  // Menu title: "Select a file:"
  tft.setTextColor(TEEK_BLUE);
  tft.setTextSize(3);
  tft.setCursor(30, 50);
  tft.print("Select a file:");

  // Add the back button
  tft.setTextSize(2);
  tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
  tft.setCursor(30, 100);
  tft.print("< Back");

  // Calculate the number of files to display
  int displayCount = min(fileCount - startIndex, MAX_FILES_ON_SCREEN);  // Display only up to MAX_FILES_ON_SCREEN files


  // Render the file names
  tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
  for (int i = 0; i < displayCount; i++) {
    tft.setCursor(30, 125 + i * 25); // Adjust the vertical position of each file
    tft.print(files[i + startIndex]);  // Print the file name
  }

  // If there are more files than can be displayed, show the "..." at the end
  if (fileCount > MAX_FILES_ON_SCREEN + startIndex) {
    tft.setCursor(30, 125 + displayCount * 25);
    tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
    tft.print("...");
  }
}


void FileMenuScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    int encoderValue = encoder.getValue();

    if(encoderValue != 0){
      fileIndex += encoderValue;  // Adjust the file index based on the encoder value
      fileIndex = constrain(fileIndex, 0, fileCount);  // Limit to valid range

      if(fileIndex < startIndex) { // manage out of bounds values
        startIndex = fileIndex;  
      } else if (fileIndex >= startIndex + MAX_FILES_ON_SCREEN) {
        startIndex = fileIndex - MAX_FILES_ON_SCREEN + 1;  
      }


      // Render the "< Back" button
      tft.setCursor(30, 100);
      if (fileIndex == 0) {
          tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight the back button
      } else {
          tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal back button
      }
      tft.print("< Back");

      // Calculate the number of files to display
      int displayCount = min(fileCount - startIndex, MAX_FILES_ON_SCREEN);

      // Render the file names
      for (int i = 0; i < displayCount; i++) {
          tft.setCursor(30, 125 + i * 25); // Adjust the vertical position of each file

          // print a line of blank spaces to cover the previous text
          tft.fillRect(30, 125 + i * 25, 480, 25, TEEK_SILVER);

          if (i + startIndex == fileIndex - 1) {
              tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
          } else {
              tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
          }
          tft.print(files[i + startIndex]); // Print the file name
      }

      // Remove the "..." if at the end of the list
      if (fileIndex < fileCount && fileCount > MAX_FILES_ON_SCREEN + startIndex) {
          tft.setCursor(30, 125 + displayCount * 25);
          tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
          tft.print("...");
      }
      else {
          // cover the "..." with a rectangle
          tft.fillRect(30, 125 + displayCount * 25, 200, 25, TEEK_SILVER);
      }
    }

    // Handle selection (e.g., when the encoder button is clicked)
    if (encoder.getButton() == ClickEncoder::Clicked) {
        handleSelection();  // Handle the file selection
    }
}


void FileMenuScreen::handleSelection() {
  if (fileIndex == 0) {
    // Return to the main menu
    __GUI.setScreen(&__mainMenuScreen);
  } else {
    // Open the selected file
    File file = __sd.open(files[fileIndex - 1]);
    if (file) {
      // Load the program from the file
      extern ProgramManager __program;
      if (__program.loadProgram(file)) {  
        __core.updateStatus(BEGIN);           // start execution
        __GUI.setScreen(&__executionScreen);  // move to the execution screen
      } else {
        drawSoftError(__screen, (char*) "Invalid or corrupted program file.");
        delay(2000);
        __GUI.setScreen(&__mainMenuScreen);
        file.close();  // Close the file when done
      }
    } else {
      // Handle file open failure
      drawSoftError(__screen, (char*)"Unable to open file.");
      delay(2000);
      __GUI.setScreen(&__mainMenuScreen);      
      file.close();  // Close the file when done
    }
  }
};



//* 6. ExecutionScreen Implementation =======================================================
// Constructor
ExecutionScreen::ExecutionScreen() {
  // Initialization logic
}


// Implement the render method
void ExecutionScreen::render(TFT_HX8357& tft) {
  drawBaseScreen(tft);
  uint16_t bgColour;
  if(__core.getControlMode() == NORMAL){  
    bgColour = TEEK_SILVER;  // Normal mode has the regular BG
  } else {
    bgColour = TEEK_YELLOW;  // PID autotune mode has a yellow BG
  }

  //fill the screen with the correspondat colour
  tft.fillRect(0, 40, 480, 260, bgColour);

  // The temperature reading is written in big letters, centered 
  tft.setTextSize(3);
  tft.setCursor(30, 50);
  tft.print("TEMP:");
  tft.setTextSize(5);
  tft.setCursor(145, 50);
  tft.print(__core.CurrentTemperature(), 2);
  tft.setCursor(360,50);
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
  else tft.print(__core.TargetTemperature(), 0);

  tft.setCursor(240, 100);
  tft.print("Status: ");
  if(__core.IsOn()) {
    tft.setTextColor(GREEN);
    tft.print("ON");
  }
  else {
    tft.setTextColor(RED);
    tft.print("OFF");
  }

  // Complete the screen with the execution information
  if(__core.getControlMode() == NORMAL){ // ----------------------------------------
    
//    In normal mode, the screen should display the following information:
//    ____________________________________________________________________________________
//    |
//    |   TEMP: [Current Temperature] [Unit]
//    |   Target: [Target Temperature] [Unit]   Status: [ON/OFF]
//    |
//    |   System: [RAMPING/STABLE/SOAKING]  [HH:MM:SS, Soaking Time Remaining]
//    |
//    |   TODO [MESSAGES FROM THE SYSTEM]
//    |
//    |   Executing: [Program Name]
//    |   Instr # [Instruction Index] of [Total Instructions] - [Current Instruction Name]
//    |   Elapsed: [HH:MM, Elapsed Time] 
//    |___________________________________________________________________________________
//  
//    ! the first two lines are already displayed by the main menu screen, and don't need
//    ! to be rendered a second time (prevents flickering)
//    
    // Normal mode:
    tft.setTextSize(2);
    tft.setTextColor(TEEK_BLUE, bgColour);

    tft.setCursor(30, 130);
    tft.print("System:"); // Current temperature behaviour: RAMPING, STABLE, SOAKING
    tft.setCursor(120, 130);
    if(__core.IsStable()){
      if(__program.IsSoaking()){
        tft.setTextColor(TEEK_YELLOW, bgColour);
        tft.print("SOAKING");
        // print the time remaining for soaking
        tft.setCursor(240, 130);
        char buff[9];
        tft.setTextColor(TEEK_BLUE, TEEK_YELLOW);
        timeStampConverter(__program.remainingSoakTime(), buff, 3);
        tft.setTextColor(TEEK_BLUE, bgColour);  //reset the regular text color
      }
      else {
        tft.setTextColor(GREEN, bgColour);
        tft.print("STABLE ");
        tft.fillRect(30, 230, 250, 25, bgColour); // fill the rest of the line to cover the previous text
        tft.setTextColor(TEEK_BLUE, bgColour);
      }
    }
    else {
      tft.setTextColor(RED, bgColour);
      tft.print("RAMPING");
      tft.fillRect(30, 230, 250, 25, bgColour); // fill the rest of the line to cover the previous text
      tft.setTextColor(TEEK_BLUE, bgColour);
    }  

    tft.setCursor(30, 210);
    tft.print("Executing:   "); tft.print(__program.Name());
    tft.setCursor(30, 240);
    tft.print("Instr #"); tft.print(__program.InstructionIndex()+1); 
    tft.print(" of "); tft.print(__program.NumOfInstructions());
    char buff[13];
    snprintf(buff, 13, "%s...", __program.CurrentInstruction().name);
    tft.print(" - "); tft.print(__program.CurrentInstruction().name);
    tft.setCursor(30, 270);
    tft.setTextColor(TEEK_BLUE, bgColour);
    tft.print("Elapsed: ");
    // == END OF NORMAL MODE
  }
  else {  // ------------------------------------------------------------------------
    // PID autotune mode:

    // During the PID autotune, the screen should display the following information:
    // PID Autotune, do not shut off!
    // It may take a while...

    // Oscillations #[oscillationCount] of [MIN_N_OSCILLATIONS]
    // Elapsed: [HH:MM, Elapsed Time]

    tft.setTextSize(2);
    tft.setTextColor(TEEK_BLACK, bgColour);

    extern AutotuneParameters __autPar;
    tft.setCursor(30, 140);
    tft.print("PID Autotune, do not shut off!");
    tft.setCursor(30, 180);
    tft.print("It may take a while...");
    tft.setCursor(30, 220);
    tft.print("Oscillation #"); 
    tft.setCursor(320, 220); tft.print(__autPar.oscillationCount);
    tft.print(" of "); tft.print(PID_N_OSCILLATIONS);
    tft.setCursor(30, 270);
    tft.print("Elapsed: ");

  }  
};

void ExecutionScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // update the fileds for the execution screen
    char buff[9];
    uint16_t bgColour;


    // Select the correct background color based on the control mode
    switch(__core.getControlMode()){
      case NORMAL: bgColour = TEEK_SILVER; break;
      case PID_AUTOTUNE: bgColour = TEEK_YELLOW; break;
    } 

    // Update the fields at a fixed interval
    if(millis()-lastUpdateTime > MIN_TIME_BETWEEN_SCREEN_UPDATES) {
      // update the temperature reading
      tft.fillRect(145, 50, 200, 40, bgColour);
      tft.setTextSize(5);
      tft.setCursor(145, 50);
      tft.setTextColor(TEEK_BLUE, bgColour);
      tft.print(__core.CurrentTemperature(), 2);
      tft.setTextSize(3);
      lastUpdateTime = millis();

      // update the target temperature
      tft.setCursor(30, 100);
      tft.setTextSize(2);
      tft.print("Target:");
      tft.print(__core.TargetTemperature(), 0);

      tft.setTextColor(TEEK_BLUE, bgColour);

      // update system fields based on the control mode
      switch(__core.getControlMode()){
        case NORMAL: // -------------------------------------------------------------
          tft.setTextColor(TEEK_BLUE, bgColour);
          tft.setCursor(30, 130);
          tft.print("System:"); // Current temperature behaviour: RAMPING, STABLE, SOAKING
          tft.setCursor(120, 130);
          if(__core.IsStable()){
            if(__program.IsSoaking()){
              tft.setTextColor(TEEK_YELLOW, bgColour);
              tft.print("SOAKING");
            }
            else {
              tft.setTextColor(GREEN, bgColour);
              tft.fillRect(30, 160, 200, 25, bgColour); // fill the line with a rectangle to cover the previous text
              tft.print("STABLE ");
              tft.fillRect(30, 230, 250, 25, bgColour); // fill the rest of the line to cover the previous text
              tft.setTextColor(TEEK_BLUE, bgColour);
            }
          }
          else {
            tft.setTextColor(RED, bgColour);
            tft.fillRect(30, 160, 200, 25, bgColour);
            tft.print("RAMPING");
            tft.fillRect(30, 230, 250, 25, bgColour); // fill the rest of the line to cover the previous text
            tft.setTextColor(TEEK_BLUE, bgColour);            
          }

          // update the instruction index
          if(lastInstructionIndex != __program.InstructionIndex()){
            tft.setCursor(30, 240);
            tft.fillRect(30, 240, 450, 25, bgColour);
            tft.setTextColor(TEEK_BLUE, bgColour);
            tft.print("Instr #"); tft.print(__program.InstructionIndex()+1); 
            tft.print(" of "); tft.print(__program.NumOfInstructions());
            char instrName[13];
            snprintf(instrName, 12, "%s", __program.CurrentInstruction().name);
            // truncate the instruction name if it is too long
            if(strlen(instrName) > 11){
              instrName[12] = '\0';
              instrName[11] = '.';
              instrName[10] = '.';
              instrName[9] = '.';
            }

            tft.print(" - "); tft.print(instrName);
            lastInstructionIndex = __program.InstructionIndex();
          }

        case PID_AUTOTUNE: // ---------------------------------------------------------
          tft.setTextColor(TEEK_BLACK, bgColour);
          extern AutotuneParameters __autPar;
          tft.setCursor(320, 220); tft.print(__autPar.oscillationCount);  // Number of oscillations
      }
    }

    // update timers at a 1 sec intervals
    // This update time is also used for time critical events
    if(millis()-lastTimerUpdate > 1000){
      switch(__core.getControlMode()){

        case NORMAL:
          buff[0] = '\0'; // clear the buffer
          // update the soak timer
          if(__core.IsStable() && __program.IsSoaking()){
            tft.setCursor(240, 130);
            timeStampConverter(__program.remainingSoakTime(), buff, 3);
            tft.setTextColor(TEEK_BLUE, TEEK_YELLOW);  // print the timer with a yellow background
            tft.print(buff);
            tft.setTextColor(TEEK_BLUE, bgColour);  // reset the regular text color
          }

          // update the elapsed timer
          tft.setTextColor(TEEK_BLACK, bgColour);
          tft.setCursor(140, 270);
          buff[0] = '\0'; // clear the buffer
          timeStampConverter(__program.elapsedTime(), buff);
          tft.print(buff);

          // update the system's status
          // update system status ASAP
          tft.setCursor(240, 100);
          tft.print("Status: ");
          if(__core.IsOn()) {
            tft.setTextColor(GREEN, bgColour);
            tft.print("ON  ");
          }
          else {
            tft.setTextColor(RED, bgColour);
            tft.print("OFF  ");
          }

          // update the message from the system
          if(messageStream[0] != '\0'){
            tft.setCursor(30, 150);
            tft.setTextColor(TEEK_BLUE, TEEK_YELLOW);
            tft.print(messageStream);
            tft.setTextColor(TEEK_BLUE, bgColour);
          }
          else {
            // fill the line with a rectangle to cover the previous text
            tft.fillRect(30, 150, 480, 25, bgColour); 
          }
          break;

        case PID_AUTOTUNE:
          // Elapsed time
          buff[0] = '\0'; // clear the buffer
          tft.setCursor(140, 270);
          tft.setTextColor(TEEK_BLACK, bgColour);
          extern AutotuneParameters __autPar;
          timeStampConverter(millis() - __autPar.startTime, buff);
          tft.print(buff);

          break;
      }

      // update timer timestamp
      lastTimerUpdate = 0;
    }

  // Handle inputs
  if (encoder.getButton() == ClickEncoder::Clicked) {
    handleSelection();  
  }    
}

void ExecutionScreen::handleSelection() {
  // No action is required in the execution screen

  if(__core.getControlMode() == PID_AUTOTUNE){
    // if the PID autotune is running, the user should not be able to stop it
    return;
  }
  else {
    // if we are expecting an action from the user, move to the next instruction
    extern ProgramManager __program;
    if(__program.CurrentInstruction().waitForButtonPress){
      __program.ConfirmButtonPressed();  // confirm the button press
      messageStream[0] = '\0';        // clear the message stream
    }
    else {
      // if the program is running normally, move to the tune screen
      __GUI.setScreen(&__tuneScreen);
    }
  }

};


//* 7. TuneScreen Implementation ========================================================
TuneScreen::TuneScreen() : menuIndex(0), confirmStop(false) {};


const char * TuneScreen::menuItems[4] = {"< Back", "> Target: ", "> Keep log: ", "> Stop"};

void TuneScreen::render(TFT_HX8357& tft){

  // fill screen with the background color
  tft.fillRect(0, 40, 480, 260, TEEK_SILVER);

  // top menu title
  tft.setTextColor(TEEK_BLUE, TEEK_SILVER);
  tft.setTextSize(3);
  tft.setCursor(30, 50);
  tft.print("Tuning:");

  // menu options
  for(int i = 0; i < menuCount; i++) {
    tft.setCursor(30, 100 + i * 35);
    if (i == menuIndex) {
      tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
    } else {
      tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
    }
    tft.print(menuItems[i]); // Print the menu item
    switch(i){
    case 1: // "> Target: xxxx.xx"
      tft.print(__core.TargetTemperature(), 0);
      break;
    case 2: // "> Keep log: [Y/N]"
      if(__core.KeepLog()) tft.print("Yes");
      else tft.print("No ");
      break;
    case 3:  // "> Stop"
      if(confirmStop) {
        tft.setCursor(320, 100 + i * 35);
        tft.setTextColor(TEEK_SILVER, RED);
        tft.print("Confirm?");
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER);
      }
      break;
   } 
  }
};

void TuneScreen::handleSelection() {
  switch (menuIndex) {
    case 0: // "< Back"
      __GUI.setScreen(&__executionScreen);  // Go back to the main menu
      break;

    case 1: // "> Target: xxxx.xx"
      __GUI.updatePrevious();    // remember the previous screen
      __GUI.setScreen(&__targetUpdateScreen); // Go to the target update screen
      break;

    case 2: // "> Keep log: [Y/N]"
      __core.setKeepLog(!__core.KeepLog()); // Toggle the keep log flag
      render(__screen); // Refresh the screen
      break;

    case 3: // "> Stop"
      // manage double confirmation
      if(confirmStop) {
        __core.updateStatus(USER_STOP);     // Update the system status
        __core.stopFiring();                // Stop the firing
        __core.setTarget(0);                 // Reset the target temperature
        __GUI.clearPrevious();              // clear the previous screen
        confirmStop = false;                // Reset the confirmation flag
        menuIndex = 0;                      // Reset the menu index
        __GUI.setScreen(&__mainMenuScreen); // go back to main menu
        } else {
        confirmStop = true; // Set the flag to confirm PID autotune
        render(__screen); // Refresh the screen
      }
      break;

    default:
      render(__screen); // Invalid selection, refresh the screen
      break;
  }

  // Reset the PID autotune confirmation flag
  if(menuIndex != 3 && confirmStop) {
    confirmStop = false;
  }
};


void TuneScreen::update(ClickEncoder& encoder, TFT_HX8357& tft) {
  int encoderValue = encoder.getValue();
  if (encoderValue != 0) {
    menuIndex = (menuIndex + encoderValue + menuCount) % menuCount; // Wrap-around menu navigation
    // update the highlighted menu item
    tft.setTextSize(3);
    tft.setCursor(30, 150);
    for (int i = 0; i < menuCount; i++) {
      tft.setCursor(30, 100 + i * 35); // Adjust position for each item
      if (i == menuIndex) {
        tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
      } else {
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      }
      tft.print(menuItems[i]); // Print the menu item
    }

    // reset the correct text color just to be sure
    tft.setTextColor(TEEK_BLACK);

  }

  // Handle encoder button press
  if (encoder.getButton() == ClickEncoder::Clicked) {
    handleSelection(); // Perform action based on the current selection
  }
}




//* % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %

//* ScreenManager Implementation ==================================

void ScreenManager::deleteCurrentScreen() {
  if (currentScreen != nullptr) {
    delete currentScreen;
    currentScreen = nullptr;
    }
};

void ScreenManager::setScreen(BaseScreen* screen) {
    //deleteCurrentScreen(); // free the memory

    currentScreen = screen; // set the new screen

    if (currentScreen) {
        currentScreen->render(__screen);  // Render the new screen only once
        lastUpdateTime = millis();        // Reset the timer for updates
    }
};

void ScreenManager::renderCurrent(TFT_HX8357& tft) {
  if (currentScreen) {
    currentScreen->render(tft);
  }
};

void ScreenManager::updateCurrent(ClickEncoder& encoder, TFT_HX8357& tft) {
    if(currentScreen) {
        currentScreen->update(encoder, tft);
    }
};

void ScreenManager::returnToPrevious() {
  if (previousScreen) {
    this->setScreen(previousScreen); // set the previous screen as the current screen
    previousScreen = nullptr; // clear the previous screen
  }
};

void ScreenManager::updateGraphics(CoreSystem& core, TFT_HX8357& tft, ClickEncoder& encoder) {
  
  SystemState currentState = core.Status(); // get the current state of the system

  // Manage state based screen changes
  if(currentState != lastState){  // if the state has changed   
    switch(currentState){
      case IDLE:  // if the system has changed to idle (i.e: program finished)
        setScreen(&__mainMenuScreen); // return to the main menu
        break;
      case ERROR: // if the system has entered an error state
        setScreen(&__criticalErrorScreen); // get stuck in the critical error screen
        // force user to reset the system
        break;
      default: 
        break;
    }

    // update the state
    lastState = currentState;
  }

  // manage inputs
  updateCurrent(encoder, tft);
};