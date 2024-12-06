#ifndef TEEK_GRAPHICS_H
#define TEEK_GRAPHICS_H
#ifndef TEEK_DATASTRUCTURES_H
#include <Arduino.h>
#include <TFT_HX8357.h>
#include <ClickEncoder.h>  
#include <TEEK_dataStructures.h>
#include <SD.h>
#endif


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


// === Externs 
extern ClickEncoder __encoder;   
extern TFT_HX8357 __screen;

// === Menu defined classes/objects ============================

enum MenuType {
    MAIN_MENU,
    SETTINGS_MENU,
    SD_MENU
};



class Menu {
    public:
    Menu(MenuType name, const char** menuItems) : menuItems(menuItems) {
        type = name;
        Index = 0;
        lastIndex = -1;
    }

    void Reset() {
        Index = 0;
        lastIndex = -1;
    }
    
    MenuType type;
    const char** menuItems;
    const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);
    int Index;
    int lastIndex;
};


// == Menus
Menu mainMenu(MAIN_MENU, (const char*[]) {"> Select from SD", "> Settings", "> Stop"});
Menu settingsMenu(SETTINGS_MENU, (const char*[]) {"- Target", "- Unit", "- PID Autotune"});
Menu sdMenu(SD_MENU, (const char*[]) {""});


Menu* currentMenu = &mainMenu;

// === Colors ==================================================
// Colors use the RGB565 format

// TEEK PALETTE
#define TEEK_BLACK  (uint16_t) 0x0000 //
#define TEEK_BLUE   (uint16_t) 0x1107 //
#define TEEK_YELLOW (uint16_t) 0xfd02 // HEX #ffa210
#define TEEK_SILVER (uint16_t) 0xe73c //  
#define TEEK_WHITE  (uint16_t) 0xffff // 

// Auxiliar colors
#define RED (uint16_t) 0xF800
#define GREEN (uint16_t) 0x07E0

// === Constants ===============================================

extern CoreSystem __core;
extern String errorStream;
extern SDClass __sd;



// === Functions ===============================================


void updateGraphics(Menu& menu, TFT_HX8357& tft);


void drawBaseScreen(TFT_HX8357& tft);
void drawMainMenu(TFT_HX8357& tft);
void drawSoftError(TFT_HX8357& tft);
void drawCriticalError(TFT_HX8357& tft);
void drawSettingsMenu(TFT_HX8357& tft);


//! Experimental
void updateMenuSelection(TFT_HX8357& tft, Menu& menu, int selected, bool fullRedraw);
void executeMenuAction(Menu& menu, TFT_HX8357 &tft);


#endif 