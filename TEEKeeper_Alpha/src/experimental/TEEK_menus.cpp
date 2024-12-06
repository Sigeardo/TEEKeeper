#include "TEEK_menus.h"

// Global variables for encoder input (set in the ISR)
volatile int encoderDelta = 0;
volatile bool encoderPressed = false;

// MenuManager constructor
MenuManager::MenuManager(TFT_HX8357& display)
    : screen(display), activeMenu(nullptr) {}

// Set the active menu
void MenuManager::setActiveMenu(Menu* menu) {
    activeMenu = menu;
}

// Update the active menu based on encoder input
void MenuManager::update(ClickEncoder& encoder, int delta, bool pressed) {
    if (activeMenu) {
        // Update menu navigation with rotation input
        activeMenu->handleRotation(delta);

        // Handle button press
        if (pressed) {
            activeMenu->handleSelection();
        }

        // Render the updated menu
        activeMenu->render(screen);
    }
}

// MainMenu constructor
MainMenu::MainMenu() : selectedOption(0), numOptions(3) {
    options[0] = "Load from SD";
    options[1] = "Settings";
    options[2] = "Stop";
}

// Render the main menu
void MainMenu::render(TFT_HX8357& tft) {
    drawBaseScreen(tft); // Fill base UI
    tft.setTextSize(3);
    tft.setTextColor(TEEK_BLACK);

    // Display temperature and target
    tft.setCursor(30, 50);
    tft.print("TEMP: ");
    tft.print(__core.CurrentTemperature(), 2);
    tft.setCursor(30, 100);
    tft.print("Target: ");
    tft.print(__core.TargetTemperature(), 2);

    // System status
    tft.setCursor(30, 150);
    tft.print("Status: ");
    tft.setTextColor(__core.isFiring() ? GREEN : RED);
    tft.print(__core.isFiring() ? "ON" : "OFF");

    // Display menu options
    tft.setTextColor(TEEK_BLACK);
    for (int i = 0; i < numOptions; ++i) {
        tft.setCursor(30, 200 + (i * 40));
        tft.print(selectedOption == i ? ">" : " "); // Highlight the current option
        tft.print(options[i]);
    }
}

// Handle rotation in the main menu
void MainMenu::handleRotation(int delta) {
    selectedOption = (selectedOption + delta + numOptions) % numOptions; // Circular navigation
}

// Handle selection (button press) in the main menu
void MainMenu::handleSelection() {
    switch (selectedOption) {
        case 0: 

            // if there is an SD card
                // Navigate to Load from SD menu
            // else
                // Display a soft error message
                // wait two seconds
                // return to the main menu
            break;
        case 1: // Settings
            // Navigate to Settings menu
            break;
        case 2: // Stop
            // Stop the system
            __core.stopFiring();
            break;
    }
}

// LoadFromSDMenu, SettingsMenu, StopMenu - You can implement similar to MainMenu
void LoadFromSDMenu::render(TFT_HX8357& tft) {
    // Render Load from SD menu
}

void LoadFromSDMenu::handleRotation(int delta) {
    // Handle rotation for Load from SD menu
}

void LoadFromSDMenu::handleSelection() {
    // Handle selection for Load from SD menu
}

void SettingsMenu::render(TFT_HX8357& tft) {
    // Render Settings menu
}

void SettingsMenu::handleRotation(int delta) {
    // Handle rotation for Settings menu
}

void SettingsMenu::handleSelection() {
    // Handle selection for Settings menu
}

void StopMenu::render(TFT_HX8357& tft) {
    // Render Stop menu
}

void StopMenu::handleRotation(int delta) {
    // Handle rotation for Stop menu
}

void StopMenu::handleSelection() {
    // Handle selection for Stop menu
}

// Encoder ISR - This function will be triggered by the TimerOne interrupt
void encoderISR() {
    __encoder.service(); // Service the encoder input

    // Update rotation delta
    encoderDelta += __encoder.getValue();

    // Check if the encoder button was pressed
    ClickEncoder::Button buttonState = __encoder.getButton();
    if (buttonState == ClickEncoder::Pressed) {
        encoderPressed = true; // Set the flag if the button was pressed
    }
}
