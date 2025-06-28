#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include "Config.h"

// Forward declarations
class ConfigManager;
class RingerManager;
class PatternManager;

// UI input types
enum UIInput {
    UI_NONE = 0,
    UI_ENCODER_CW,          // Encoder clockwise
    UI_ENCODER_CCW,         // Encoder counter-clockwise
    UI_ENCODER_PRESS,       // Encoder button press
    UI_ENCODER_LONG_PRESS   // Encoder long press
};

// UI states/screens
enum UIScreen {
    SCREEN_STATUS = 0,      // Main status display
    SCREEN_MENU,            // Main menu
    SCREEN_RELAY_COUNT,     // Set number of active relays
    SCREEN_PATTERN_MODE,    // Select pattern mode
    SCREEN_RING_STYLE,      // Select ring style
    SCREEN_MAX_RINGS,       // Set max rings per call
    SCREEN_SIMULTANEOUS,    // Set max simultaneous rings
    SCREEN_TIMING,          // Timing settings
    SCREEN_ADVANCED,        // Advanced settings
    SCREEN_ABOUT            // About/info screen
};

class UIManager {
public:
    UIManager();
    
    // Initialize with hardware pins and references
    void initialize(int encoderPinA, int encoderPinB, int encoderButton,
                   ConfigManager* configMgr, RingerManager* ringerMgr, 
                   PatternManager* patternMgr);
    
    // Main update loop
    void step(unsigned long currentTime);
    
    // Input handling
    UIInput checkInput();
    void handleInput(UIInput input);
    
    // Display management
    void updateDisplay();
    void setDisplayBrightness(uint8_t brightness);
    void wakeDisplay();
    
    // Menu navigation
    void enterMenu();
    void exitMenu();
    void navigateMenu(int direction);
    void selectMenuItem();
    
    // Screen management
    void setScreen(UIScreen screen);
    UIScreen getCurrentScreen() const { return currentScreen; }
    
    // Get display strings for current screen
    String getDisplayLine1() const;
    String getDisplayLine2() const;
    
private:
    // Hardware references
    int encoderA, encoderB, encoderBtn;
    ConfigManager* configManager;
    RingerManager* ringerManager;
    PatternManager* patternManager;
    
    // UI state
    UIScreen currentScreen;
    uint8_t menuIndex;
    uint8_t subMenuIndex;
    bool inMenu;
    bool settingValue;
    
    // Input handling
    int lastEncoderA, lastEncoderB;
    bool lastButtonState;
    unsigned long buttonPressTime;
    unsigned long lastInputTime;
    
    // Display state
    bool displayActive;
    unsigned long lastDisplayUpdate;
    uint8_t currentBrightness;
    
    // Menu definitions
    static const char* MAIN_MENU_ITEMS[];
    static const uint8_t MAIN_MENU_COUNT;
    
    // Screen handlers
    void handleStatusScreen(UIInput input);
    void handleMenuScreen(UIInput input);
    void handleSettingScreen(UIInput input);
    
    // Display helpers
    String formatStatusLine1() const;
    String formatStatusLine2() const;
    String formatMenuLine(uint8_t index) const;
    String formatSettingValue(UIScreen screen) const;
    
    // Input helpers
    bool readEncoder(int& direction);
    bool readButton(bool& longPress);
    void debounceInputs();
    
    // Value adjustment helpers
    void adjustCurrentSetting(int direction);
    void saveCurrentSetting();
};

#endif
