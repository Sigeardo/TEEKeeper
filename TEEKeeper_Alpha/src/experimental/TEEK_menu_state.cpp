#include "TEEK_menu_state.h"

// Static variables for encoder ISR
volatile int TEEK_MenusState::encoderDelta = 0;
volatile bool TEEK_MenusState::buttonPressed = false;

// Main Menu State
void MainMenuState::enter(TFT_HX8357& tft) {
    drawBaseScreen(tft); // Fill the base UI
    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);

    // Render temperature, target, and status
    tft.setCursor(30, 50);
    tft.print("TEMP: ");
    tft.print(__core.CurrentTemperature(), 2);
    tft.setCursor(30, 100);
    tft.print("Target: ");
    tft.print(__core.TargetTemperature(), 2);
    tft.setCursor(30, 150);
    tft.print("Status: ");
    tft.setTextColor(__core.isFiring() ? GREEN : RED);
    tft.print(__core.isFiring() ? "ON" : "OFF");

    // Render menu options
    for (int i = 0; i < 3; ++i) {
        tft.setCursor(30, 200 + (i * 40));
        tft.print(selectedOption == i ? ">" : " "); // Highlight selected option
        tft.print(options[i]);
    }
}


extern CoreSystem __core;  // Assuming the core system is declared globally

// Constructor: Initialize variables
MainMenuState::MainMenuState() : selectedOption(0), buttonPressed(false), encoderDelta(0) {
    options[0] = "> Load from SD";
    options[1] = "> Settings";
    options[2] = "> Stop";
}

// Called when entering the menu state (initialization)
void MainMenuState::enter(TFT_HX8357& tft) {
    render(tft);  // Render the initial menu on the screen
}

// Update function: Handles encoder rotation and button presses
void MainMenuState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Handle encoder rotation (non-blocking)
    if (encoderDelta != 0) {
        selectedOption = (selectedOption + encoderDelta + 3) % 3;  // Handle circular navigation
        encoderDelta = 0;  // Reset encoder delta after handling
    }

    // Handle button press
    if (buttonPressed) {
        handleButtonAction();  // Handle the action triggered by the button
        buttonPressed = false;  // Reset the button press flag
    }

    // Render the updated menu on the TFT screen
    render(tft);
}

// Return the next state to transition to based on the selected option
MenuStateType MainMenuState::nextState() {
    if (selectedOption == 0) {
        return LOAD_FROM_SD_MENU;
    } else if (selectedOption == 1) {
        return SETTINGS_MENU;
    } else if (selectedOption == 2) {
        return STOP_MENU;
    }
    return MAIN_MENU;  // Default to main menu
}

// Handle encoder rotation: Update delta based on encoder movement
void MainMenuState::handleEncoderRotation(int delta) {
    encoderDelta = delta;
}

// Handle button press: Set the buttonPressed flag
void MainMenuState::handleButtonPress() {
    buttonPressed = true;
}

// Render the menu on the TFT screen
void MainMenuState::render(TFT_HX8357& tft) {
    drawBaseScreen(tft);  // Draw the background for the menu

    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);

    // Display the menu options
    for (int i = 0; i < 3; ++i) {
        tft.setCursor(30, 50 + (i * 40));
        tft.print(selectedOption == i ? ">" : " ");  // Highlight the selected option
        tft.print(options[i]);
    }

    // Display temperature and system status
    tft.setCursor(30, 220);
    tft.print("TEMP: ");
    tft.print(__core.CurrentTemperature(), 2);  // Current temperature
    tft.setCursor(30, 250);
    tft.print("Target: ");
    tft.print(__core.TargetTemperature(), 2);  // Target temperature
    tft.setCursor(30, 280);
    tft.print("Status: ");
    tft.setTextColor(__core.isFiring() ? GREEN : RED);
    tft.print(__core.isFiring() ? "ON" : "OFF");  // System status (ON/OFF)
}

// Handle the action associated with a button press (e.g., "Stop")
void MainMenuState::handleButtonAction() {
    if (selectedOption == 2) {
        __core.stopFiring();  // Stop the system if "Stop" is selected
    }
    // Additional actions can be added here based on menu options
}


void MainMenuState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Handle encoder rotation from ISR (non-blocking)
    if (encoderDelta != 0) {
        selectedOption = (selectedOption + encoderDelta + 3) % 3;
        encoderDelta = 0;  // Reset delta after handling
    }

    if (buttonPressed) {
        __core.stopFiring();  // Action for "Stop" option
        buttonPressed = false;
    }
}

MenuStateType MainMenuState::nextState() {
    if (selectedOption == 0) return LOAD_FROM_SD_MENU;
    if (selectedOption == 1) return SETTINGS_MENU;
    if (selectedOption == 2) return STOP_MENU;
    return MAIN_MENU;
}

// Settings Menu State
void SettingsMenuState::enter(TFT_HX8357& tft) {
    tft.fillScreen(TEEK_SILVER);
    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);
    tft.setCursor(30, 50);
    tft.print("Settings Menu");
}

void SettingsMenuState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Handle settings navigation, example: change target temperature
}

MenuStateType SettingsMenuState::nextState() {
    return MAIN_MENU;
}

// Load from SD menu state
void LoadFromSDMenuState::enter(TFT_HX8357& tft) {
    tft.fillScreen(TEEK_SILVER);
    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);
    tft.setCursor(30, 50);
    tft.print("Load from SD");
}

void LoadFromSDMenuState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Handle SD file loading, update based on encoder
}

MenuStateType LoadFromSDMenuState::nextState() {
    return MAIN_MENU;
}

// Stop menu state
void StopMenuState::enter(TFT_HX8357& tft) {
    tft.fillScreen(TEEK_SILVER);
    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);
    tft.setCursor(30, 50);
    tft.print("System Stopped");
}

void StopMenuState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Handle stop menu logic, like resetting the system
}

MenuStateType StopMenuState::nextState() {
    return MAIN_MENU;
}

// FSM Manager Constructor and Methods
TEEK_MenusState::TEEK_MenusState() {
    currentState = &mainMenuState;  // Initial state
}

void TEEK_MenusState::init(TFT_HX8357& tft) {
    currentState->enter(tft);
    Timer1.initialize(1000);  // Set timer to 1ms
    Timer1.attachInterrupt(encoderISR);  // Attach the ISR to Timer1
}

void TEEK_MenusState::update(ClickEncoder& encoder, TFT_HX8357& tft) {
    // Update the current state
    currentState->update(encoder, tft);
    
    // Determine the next state
    MenuStateType next = currentState->nextState();

    // If the next state is different from MAIN_MENU, transition to the new state
    if (next != MAIN_MENU) {
        // Determine the new state based on the current state type
        switch (next) {
            case SETTINGS_MENU:
                currentState = &settingsMenuState;
                break;

            case LOAD_FROM_SD_MENU:
                currentState = &loadFromSDMenuState;
                break;

            case STOP_MENU:
                currentState = &stopMenuState;
                break;

            case MAIN_MENU: // fallthrough to MAIN_MENU state
            default:
                currentState = &mainMenuState;
                break;
        }

        // Enter the new state and render the corresponding menu
        currentState->enter(tft);
    }
};


// Encoder ISR to handle rotation and button press
void TEEK_MenusState::encoderISR() {
    static ClickEncoder encoder(PIN_ENCODER_S1, PIN_ENCODER_S2, PIN_ENCODER_KEY, ENCODER_STEPS);
    
    int newDelta = encoder.getValue();
    if (newDelta != 0) {
        encoderDelta = newDelta;
    }
    
    if (encoder.getButton() == ClickEncoder::Clicked) {
        buttonPressed = true;
    }
};
