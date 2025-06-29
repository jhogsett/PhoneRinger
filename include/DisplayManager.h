#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

// Forward declarations
class RingerManager;

class DisplayManager {
public:
    DisplayManager();
    
    // Initialize display
    void initialize(bool enableSerialOutput = true);
    
    // Update display content
    void update(unsigned long currentTime, bool systemPaused, const RingerManager* ringerManager, int maxConcurrent = -1);
    
    // Display control
    void setBrightness(uint8_t brightness);
    void clear();
    // Display methods - using const char* for heap safety
    void showMessage(const char* line1, const char* line2 = "", 
                    const char* line3 = "", const char* line4 = "");
    
    // Display specific screens
    void showStatus(const RingerManager* ringerManager, bool paused, int maxConcurrent = -1);
    void showStartupMessage();
    void showPauseMessage();
    void showResumeMessage();
    void showChaosMessage(); // Maximum chaos easter egg display
    void showRelayAdjustmentMessage(int newCount); // Brief relay adjustment feedback
    void showRelayAdjustmentDirection(int newCount, bool increment); // Show +1/-1 style feedback

private:
    hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
    bool lcdAvailable;  // Track if LCD is actually working
    unsigned long lastUpdate;
    uint8_t currentScreen;
    bool displayNeedsUpdate;
    
    // Temporary message state (non-blocking)
    bool showingTempMessage;
    unsigned long tempMessageStartTime;
    char tempMessageText[21];  // Buffer for temporary message
    static const unsigned long TEMP_MESSAGE_DURATION = 800;  // 0.8 seconds
    
    // Display update intervals
    static const unsigned long NORMAL_UPDATE_INTERVAL = 500;   // 0.5 seconds
    static const unsigned long FAST_UPDATE_INTERVAL = 100;     // 0.1 seconds for active changes
    
    // Helper methods
    // Note: Legacy String-based methods removed for heap safety
};

#endif
