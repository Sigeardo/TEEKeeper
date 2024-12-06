#ifndef TEEK_MENUS_STATE_H
#define TEEK_MENUS_STATE_H

#include <ClickEncoder.h>
#include <TFT_HX8357.h>
#include <Arduino.h>
#include "TEEK_graphics.h" // for the drawing functions
#include <TimerOne.h> // Library for timer interrupts

// Forward declarations of menu states
class MenuState;
class MainMenuState;
class SettingsMenuState;
class LoadFromSDMenuState;
class StopMenuState;

// Enum to define menu states
enum MenuStateType {
    MAIN_MENU,
    SETTINGS_MENU,
    LOAD_FROM_SD_MENU,
    STOP_MENU
};

// MenuState class definition (base class for all menu states)
class MenuState {
public:
    virtual void enter(TFT_HX8357& tft) = 0;
    virtual void update(ClickEncoder& encoder, TFT_HX8357& tft) = 0;
    virtual MenuStateType nextState() = 0;
};

// Main menu state
class MainMenuState : public MenuState {
public:
    MainMenuState();

    // Enter the menu state, initializing it
    void enter(TFT_HX8357& tft) override;

    // Update the menu state, handle encoder and button inputs
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;

    // Determine the next menu state
    MenuStateType nextState() override;

    // Handle encoder rotation
    void handleEncoderRotation(int delta);

    // Handle button press events
    void handleButtonPress();

private:
    void render(TFT_HX8357& tft);  // Render the menu to the TFT screen
    void handleButtonAction();      // Action for button press (e.g., "Stop")

    int selectedOption;            // Currently selected option
    const char* options[3];        // Menu options: Load from SD, Settings, Stop
    bool buttonPressed;            // Flag indicating button press
    int encoderDelta;              // Encoder delta (rotation amount)
};

// Settings menu state
class SettingsMenuState : public MenuState {
public:
    void enter(TFT_HX8357& tft) override;
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
    MenuStateType nextState() override;
};

// Load from SD menu state
class LoadFromSDMenuState : public MenuState {
public:
    void enter(TFT_HX8357& tft) override;
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
    MenuStateType nextState() override;
};

// Stop menu state
class StopMenuState : public MenuState {
public:
    void enter(TFT_HX8357& tft) override;
    void update(ClickEncoder& encoder, TFT_HX8357& tft) override;
    MenuStateType nextState() override;
};

// FSM manager
class TEEK_MenusState {
public:
    TEEK_MenusState();
    void init(TFT_HX8357& tft);
    void update(ClickEncoder& encoder, TFT_HX8357& tft);

    // Encoder ISR
    static void encoderISR();

private:
    MenuState* currentState;
    MainMenuState mainMenuState;
    SettingsMenuState settingsMenuState;
    LoadFromSDMenuState loadFromSDMenuState;
    StopMenuState stopMenuState;

    static volatile int encoderDelta;
    static volatile bool buttonPressed;
};

#endif
