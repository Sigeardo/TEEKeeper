
#include <TFT_HX8357.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

// TFT Screen setup
TFT_HX8357 tft = TFT_HX8357(); // Use the HX8357 library

// Rotary encoder setup (Pins 4, 3, 2)
ClickEncoder encoder(4, 3, 2); // Adjusted to your configuration
int menuIndex = 0; // Current selected menu index
int lastMenuIndex = -1; // To track changes in menu selection

void timerIsr();

// Menu items
const char* menuItems[] = {"> Select from SD", "> Settings", "> Stop"};
const int menuCount = 3;

// Color definitions
#define TEEK_BLACK  (uint16_t) 0x0000 //
#define TEEK_BLUE   (uint16_t) 0x1107 //
#define TEEK_YELLOW (uint16_t) 0xfd02 // HEX #ffa210
#define TEEK_SILVER (uint16_t) 0xe73c //  
#define TEEK_WHITE  (uint16_t) 0xffff // 
#define GREEN         0x07E0
#define RED           0xF800

// Dummy core data (replace with actual functionality)
struct {
  float CurrentTemperature() { return 75.25; }
  float TargetTemperature() { return 100.0; }
  bool isFiring() { return false; }
  int Unit() { return 0; } // 0: CELSIUS, 1: FAHRENHEIT, 2: KELVIN
} __core;

void drawBaseScreen(TFT_HX8357& tft);
void drawMainMenu(TFT_HX8357& tft);
void updateMenuSelection(TFT_HX8357& tft, int selected, bool fullRedraw = false);
void executeMenuAction(int index);

void setup() {
  // Initialize screen
  tft.begin();
  tft.setRotation(3); // Adjusted screen rotation
  drawBaseScreen(tft); // Draw the base screen
  drawMainMenu(tft);   // Draw the initial menu

  // Initialize encoder
  Timer1.initialize(1000); // Timer for smooth encoder input
  Timer1.attachInterrupt(timerIsr);
  encoder.setAccelerationEnabled(true);

  Serial.begin(9600);
}

void loop() {
  // Handle rotary encoder
  int value = encoder.getValue();
  if (value != 0) {
    lastMenuIndex = menuIndex;
    menuIndex = (menuIndex + value + menuCount) % menuCount; // Wrap around
    updateMenuSelection(tft, menuIndex); // Update only the changed items
  }

  // Handle button press
  if (encoder.getButton() == ClickEncoder::Clicked) {
    executeMenuAction(menuIndex);
  }

  delay(50); // Small delay to debounce
}

// Draw the base UI
void drawBaseScreen(TFT_HX8357& tft) {
  tft.fillScreen(TEEK_SILVER); // fill the screen with the silver color
  
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
}

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
    for (int i = 0; i < menuCount; i++) {
      tft.setCursor(30, 150 + i * 50);
      if (i == menuIndex) {
        tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlight current selection
      } else {
        tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      }
      tft.print(menuItems[i]);
    }
}

// Update the menu selection
void updateMenuSelection(TFT_HX8357& tft, int selected, bool fullRedraw) {
  if (fullRedraw) {
    drawMainMenu(tft); // Redraw entire menu
  } else {
    // Clear and redraw only the affected menu items
    if (lastMenuIndex != -1) {
      tft.setCursor(30, 150 + lastMenuIndex * 50);
      tft.setTextColor(TEEK_BLACK, TEEK_SILVER); // Normal text
      tft.print(menuItems[lastMenuIndex]);
    }

    // Highlight current selection
    tft.setCursor(30, 150 + selected * 50);
    tft.setTextColor(TEEK_BLACK, TEEK_YELLOW); // Highlighted text
    tft.print(menuItems[selected]);
  }
}

// Handle menu actions
void executeMenuAction(int index) {
  tft.fillScreen(TEEK_SILVER);
  tft.setTextColor(GREEN);
  tft.setTextSize(3);
  tft.setCursor(50, 150);

  if (index == 0) {
    tft.print("Loading from SD...");
  } else if (index == 1) {
    tft.print("Opening Settings...");
  } else if (index == 2) {
    tft.print("Stopping system...");
  }

  delay(2000); // Pause for feedback
  drawBaseScreen(tft); // Redraw base screen
  drawMainMenu(tft);   // Return to main menu
}

// Interrupt service routine for encoder
void timerIsr() {
  encoder.service();
}
