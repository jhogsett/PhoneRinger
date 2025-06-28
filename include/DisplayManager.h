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
    void initialize();
    
    // Update display content
    void update(unsigned long currentTime, bool systemPaused, const RingerManager* ringerManager, int maxConcurrent = -1);
    
    // Display control
    void setBrightness(uint8_t brightness);
    void clear();
    void showMessage(const String& line1, const String& line2 = "", 
                    const String& line3 = "", const String& line4 = "");
    
    // Display specific screens
    void showStatus(const RingerManager* ringerManager, bool paused, int maxConcurrent = -1);
    void showStartupMessage();
    void showPauseMessage();
    void showResumeMessage();

private:
    hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
    bool lcdAvailable;  // Track if LCD is actually working
    unsigned long lastUpdate;
    uint8_t currentScreen;
    bool displayNeedsUpdate;
    
    // Display update intervals
    static const unsigned long NORMAL_UPDATE_INTERVAL = 500;   // 0.5 seconds
    static const unsigned long FAST_UPDATE_INTERVAL = 100;     // 0.1 seconds for active changes
    
    // Helper methods
    String formatTime(unsigned long milliseconds);
    String formatPhoneStatus(const RingerManager* ringerManager);
    String formatActiveCount(const RingerManager* ringerManager);
    void centerText(String text, int line, int width = 20);
    String padString(String str, int length);
};

#endif
