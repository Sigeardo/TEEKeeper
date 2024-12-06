#ifndef TEEK_MENUS_H
#define TEEK_MENUS_H

#include <Arduino.h>
#include <TFT_HX8357.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include "TEEK_graphics.h" // Assuming this contains UI rendering functions and the CoreSystem class

// Forward declarations
class MenuManager;
class Menu;
class MainMenu;

// Encoder ISR flag
extern volatile int encoderDelta;
extern volatile bool encoderPressed;

// MenuManager class
class MenuManager {
public:
    MenuManager(TFT_HX8357& display);  // Constructor
    void setActiveMenu(Menu* menu);    // Set the active menu
    void update(ClickEncoder& encoder, int delta, bool pressed);  // Update the active menu based on encoder input

private:
    TFT_HX8357& screen; // Screen reference
    Menu* activeMenu;   // Pointer to the current active menu
};

// Abstract base class for all menus
class Menu {
public:
    virtual void render(TFT_HX8357& tft) = 0;          // Virtual render method to be implemented by derived classes
    virtual void handleRotation(int delta) = 0;         // Handle rotation input (turning the encoder)
    virtual void handleSelection() = 0;                 // Handle button press (selecting the option)
};

// Main Menu class, inherited from Menu
class MainMenu : public Menu {
public:
    MainMenu(); // Constructor

    void render(TFT_HX8357& tft) override;     // Render main menu
    void handleRotation(int delta) override;  // Handle rotation for main menu
    void handleSelection() override;          // Handle button press for main menu

private:
    String options[3];    // Menu options (Load from SD, Settings, Stop)
    int numOptions;       // Number of options
    int selectedOption;   // Currently selected option
};

// Load from SD Menu class, inherited from Menu
class LoadFromSDMenu : public Menu {
public:
    void render(TFT_HX8357& tft) override;
    void handleRotation(int delta) override;
    void handleSelection() override;
};

// Settings Menu class, inherited from Menu
class SettingsMenu : public Menu {
public:
    void render(TFT_HX8357& tft) override;
    void handleRotation(int delta) override;
    void handleSelection() override;
};

// Stop Menu class, inherited from Menu
class StopMenu : public Menu {
public:
    void render(TFT_HX8357& tft) override;
    void handleRotation(int delta) override;
    void handleSelection() override;
};

// Global function to handle encoder input in interrupt
void encoderISR();

#endif  // TEEK_MENUS_H
