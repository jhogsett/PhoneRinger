#include "DisplayManager.h"
#include "RingerManager.h"
#include "StringUtils.h"

// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;

// Update intervals
const unsigned long NORMAL_UPDATE_INTERVAL = 500;  // 500ms when paused
const unsigned long FAST_UPDATE_INTERVAL = 100;    // 100ms when active

// 🌪️ Custom character definitions for 3-frame storm animation (5x8 pixels)
// Each byte represents a row, each bit a pixel (1=on, 0=off)
// Frame 0: Storm swirl starting position
const uint8_t stormFrame0[8] = {
  0b00000,  // -----
  0b00100,  // --X--
  0b01110,  // -XXX-
  0b11011,  // XX-XX
  0b10001,  // X---X
  0b01110,  // -XXX-
  0b00100,  // --X--
  0b00000   // -----
};

// Frame 1: Storm swirl middle position
const uint8_t stormFrame1[8] = {
  0b00000,  // -----
  0b00010,  // ---X-
  0b01100,  // -XX--
  0b11010,  // XX-X-
  0b01011,  // -X-XX
  0b00110,  // --XX-
  0b01000,  // -X---
  0b00000   // -----
};

// Frame 2: Storm swirl end position
const uint8_t stormFrame2[8] = {
  0b00000,  // -----
  0b01000,  // -X---
  0b00110,  // --XX-
  0b01011,  // -X-XX
  0b11010,  // XX-X-
  0b01100,  // -XX--
  0b00010,  // ---X-
  0b00000   // -----
};

// Array of pointers to the frame data for easy access
const uint8_t* stormFrames[3] = {stormFrame0, stormFrame1, stormFrame2};

DisplayManager::DisplayManager() {
    lastUpdate = 0;
    currentScreen = 0;
    displayNeedsUpdate = true;
    lcdAvailable = false;  // Will be set to true if LCD initializes successfully
    showingTempMessage = false;
    tempMessageStartTime = 0;
    tempMessageText[0] = '\0';
    animationEnabled = true;
    lastAnimationUpdate = 0;
    currentAnimationFrame = 0;
}

void DisplayManager::initialize(bool enableSerialOutput) {
    if (enableSerialOutput) {
        Serial.println("Initializing 20x4 LCD Display...");
    }
    
    // Add a timeout to prevent hanging if LCD is not connected
    if (enableSerialOutput) {
        Serial.println("Scanning I2C bus for LCD...");
    }
    
    // Try to initialize with a timeout approach
    int status = -1;
    unsigned long startTime = millis();
    const unsigned long INIT_TIMEOUT = 2000; // 2 second timeout
    
    // Attempt LCD initialization with timeout protection
    bool initSuccess = false;
    
    // First, try a simple I2C scan to see if anything responds
    Wire.begin();
    Wire.beginTransmission(0x27); // Try common address first
    if (Wire.endTransmission() == 0) {
        if (enableSerialOutput) {
            Serial.println("Found I2C device at 0x27");
        }
        initSuccess = true;
    } else {
        Wire.beginTransmission(0x3F); // Try alternate address
        if (Wire.endTransmission() == 0) {
            if (enableSerialOutput) {
                Serial.println("Found I2C device at 0x3F");
            }
            initSuccess = true;
        }
    }
    
    if (initSuccess) {
        status = lcd.begin(LCD_COLS, LCD_ROWS);
        if (status == 0) {
            lcdAvailable = true;  // Mark LCD as available
            lcd.clear();
            
            // Initialize custom characters for storm animation
            initializeStormAnimation();
            
            showStartupMessage();
            if (enableSerialOutput) {
                Serial.println("20x4 LCD Display initialized successfully");
                Serial.println("Storm animation characters loaded");
            }
        } else {
            if (enableSerialOutput) {
                Serial.print("LCD initialization failed with status: ");
                Serial.println(status);
                Serial.println("Continuing without LCD...");
            }
            lcdAvailable = false;
        }
    } else {
        if (enableSerialOutput) {
            Serial.println("No I2C LCD found at addresses 0x27 or 0x3F");
            Serial.println("Continuing without LCD...");
        }
        lcdAvailable = false;
    }
}

void DisplayManager::update(unsigned long currentTime, bool systemPaused, const RingerManager* ringerManager, int maxConcurrent) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    // Update storm animation (independent of display updates)
    updateStormAnimation();
    
    // Determine update interval based on system state
    unsigned long updateInterval = systemPaused ? NORMAL_UPDATE_INTERVAL : FAST_UPDATE_INTERVAL;
    
    if (currentTime - lastUpdate >= updateInterval || displayNeedsUpdate) {
        if (systemPaused) {
            showPauseMessage();
        } else {
            showStatus(ringerManager, false, maxConcurrent);
        }
        
        lastUpdate = currentTime;
        displayNeedsUpdate = false;
    }
}

void DisplayManager::setBrightness(uint8_t brightness) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    // hd44780_I2Cexp automatically manages backlight
    // Just turn on/off based on brightness value
    if (brightness > 0) {
        lcd.backlight();
    } else {
        lcd.noBacklight();
    }
}

void DisplayManager::clear() {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    lcd.clear();
    displayNeedsUpdate = true;
}

void DisplayManager::showMessage(const char* line1, const char* line2, 
                                const char* line3, const char* line4) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    lcd.clear();
    
    if (line1 && strlen(line1) > 0) {
        lcd.setCursor(0, 0);
        padStringToGlobalBuffer(line1, 20);
        lcd.print(globalStringBuffer);
    }
    if (line2 && strlen(line2) > 0) {
        lcd.setCursor(0, 1);
        padStringToGlobalBuffer(line2, 20);
        lcd.print(globalStringBuffer);
    }
    if (line3 && strlen(line3) > 0) {
        lcd.setCursor(0, 2);
        padStringToGlobalBuffer(line3, 20);
        lcd.print(globalStringBuffer);
    }
    if (line4 && strlen(line4) > 0) {
        lcd.setCursor(0, 3);
        padStringToGlobalBuffer(line4, 20);
        lcd.print(globalStringBuffer);
    }
}

void DisplayManager::showStatus(const RingerManager* ringerManager, bool paused, int maxConcurrent) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    // Line 1: CallStorm branding with storm icon and right-aligned timer (20 chars: "CallStorm🌪️    12:34")
    lcd.setCursor(0, 0);
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    // Format: "CallStorm" + storm icon (char 0) + spaces + timer
    snprintf(globalStringBuffer, sizeof(globalStringBuffer), "CallStorm%c     %02lu:%02lu", '\x00', minutes % 100, seconds);
    // Ensure exactly 20 characters by padding with spaces
    int len1 = strlen(globalStringBuffer);
    for (int i = len1; i < 20; i++) {
        globalStringBuffer[i] = ' ';
    }
    globalStringBuffer[20] = '\0';
    lcd.print(globalStringBuffer);
    
    // Line 2: Show temporary message if active, otherwise leave blank for alerts
    lcd.setCursor(0, 1);
    unsigned long currentTime = millis();
    
    // Check if we should show a temporary message
    if (showingTempMessage) {
        if (currentTime - tempMessageStartTime < TEMP_MESSAGE_DURATION) {
            // Still showing temp message
            snprintf(globalStringBuffer, sizeof(globalStringBuffer), "%s", tempMessageText);
            int len = strlen(globalStringBuffer);
            for (int i = len; i < 20; i++) {
                globalStringBuffer[i] = ' ';
            }
            globalStringBuffer[20] = '\0';
            lcd.print(globalStringBuffer);
        } else {
            // Temp message expired, clear it
            showingTempMessage = false;
            snprintf(globalStringBuffer, sizeof(globalStringBuffer), "                    ");
            lcd.print(globalStringBuffer);
        }
    } else {
        // No temp message, show normal blank line for future alerts
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "                    ");
        lcd.print(globalStringBuffer);
    }
    
    // Line 3: Active calls and ringing phones with enabled relay count (20 chars max)
    // Format: "A:0 R:0 En:8 Max:4" or "A:0 R:0 En:8" if no limit
    lcd.setCursor(0, 2);
    if (maxConcurrent > 0 && maxConcurrent < ringerManager->getTotalPhoneCount()) {
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "A:%d R:%d En:%d Max:%d", 
                ringerManager->getActiveCallCount(),
                ringerManager->getRingingPhoneCount(),
                ringerManager->getActivePhoneCount(),
                maxConcurrent);
    } else {
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "A:%d R:%d En:%d", 
                ringerManager->getActiveCallCount(),
                ringerManager->getRingingPhoneCount(),
                ringerManager->getActivePhoneCount());
    }
    // Pad to 20 characters
    int len = strlen(globalStringBuffer);
    for (int i = len; i < 20; i++) {
        globalStringBuffer[i] = ' ';
    }
    globalStringBuffer[20] = '\0';
    lcd.print(globalStringBuffer);
    
    // Line 4: Spaced and centered phone status (15 chars: "  R A - - X X X X  ")
    lcd.setCursor(0, 3);
    if (paused) {
        snprintf(globalStringBuffer, sizeof(globalStringBuffer), "** PAUSED **");
        // Center the text
        int textLen = strlen(globalStringBuffer);
        int spaces = (20 - textLen) / 2;
        for (int i = 19; i >= 0; i--) {
            if (i >= spaces && i < spaces + textLen) {
                globalStringBuffer[i] = globalStringBuffer[i - spaces];
            } else {
                globalStringBuffer[i] = ' ';
            }
        }
        globalStringBuffer[20] = '\0';
    } else {
        // Create spaced phone status: each phone gets a char + space, then center it
        char phoneChars[8];
        for (int i = 0; i < 8; i++) {
            if (i < ringerManager->getActivePhoneCount()) {
                if (ringerManager->isPhoneRinging(i)) {
                    phoneChars[i] = 'R';  // Ringing
                } else if (ringerManager->isPhoneActive(i)) {
                    phoneChars[i] = 'A';  // Active
                } else {
                    phoneChars[i] = '-';  // Idle
                }
            } else {
                phoneChars[i] = 'X';  // Disabled
            }
        }
        
        // Build spaced string: "R A - - X X X X" (15 chars)
        globalStringBuffer[0] = phoneChars[0];
        for (int i = 1; i < 8; i++) {
            globalStringBuffer[i * 2 - 1] = ' ';      // Space
            globalStringBuffer[i * 2] = phoneChars[i]; // Phone char
        }
        // Total spaced string is 15 chars, center it in 20 chars (2 spaces on each side)
        // Move the 15 chars to positions 2-16, then pad
        for (int i = 14; i >= 0; i--) {
            globalStringBuffer[i + 2] = globalStringBuffer[i];
        }
        globalStringBuffer[0] = ' ';
        globalStringBuffer[1] = ' ';
        globalStringBuffer[17] = ' ';
        globalStringBuffer[18] = ' ';
        globalStringBuffer[19] = ' ';
        globalStringBuffer[20] = '\0';
    }
    lcd.print(globalStringBuffer);
}

void DisplayManager::showStartupMessage() {
    showMessage("CallStorm Simulator",
                "Version 2.0",
                "8-Phone System",
                "Initializing...");
}

void DisplayManager::showPauseMessage() {
    showMessage("CallStorm System",
                "** SYSTEM PAUSED **",
                "All relays OFF",
                "Press PinA0 to resume");
}

void DisplayManager::showResumeMessage() {
    showMessage("CallStorm System",
                "** SYSTEM RESUMED **",
                "Calls restarting...",
                "");
    delay(1000); // Show resume message briefly
    displayNeedsUpdate = true;
}

void DisplayManager::showChaosMessage() {
    showMessage("CallStorm 2000",
                "** MAXIMUM CHAOS **",
                "ALL SYSTEMS ACTIVE!",
                "BRACE FOR IMPACT!");
    delay(3000); // Show chaos message for 3 seconds
    displayNeedsUpdate = true;
}

void DisplayManager::showRelayAdjustmentMessage(int newCount) {
    // Start showing a temporary message (non-blocking)
    snprintf(tempMessageText, sizeof(tempMessageText), "Relays: %d", newCount);
    showingTempMessage = true;
    tempMessageStartTime = millis();
    displayNeedsUpdate = true; // Trigger immediate display update
}

void DisplayManager::showRelayAdjustmentDirection(int newCount, bool increment) {
    // Start showing a temporary directional message (non-blocking)
    if (increment) {
        snprintf(tempMessageText, sizeof(tempMessageText), "Relays +1 (%d)", newCount);
    } else {
        snprintf(tempMessageText, sizeof(tempMessageText), "Relays -1 (%d)", newCount);
    }
    showingTempMessage = true;
    tempMessageStartTime = millis();
    displayNeedsUpdate = true; // Trigger immediate display update
}

void DisplayManager::showSaveExitMessage() {
    // Show brief "Settings Saved!" message before returning to operation
    snprintf(tempMessageText, sizeof(tempMessageText), "Settings Saved!");
    showingTempMessage = true;
    tempMessageStartTime = millis();
    displayNeedsUpdate = true; // Trigger immediate display update
}

void DisplayManager::initializeStormAnimation() {
    if (!lcdAvailable) return;
    
    // Load the first frame as custom character 0
    lcd.createChar(0, stormFrames[0]);
    
    // Initialize animation state
    currentAnimationFrame = 0;
    lastAnimationUpdate = millis();
}

void DisplayManager::updateStormAnimation() {
    if (!lcdAvailable || !animationEnabled) return;
    
    unsigned long currentTime = millis();
    
    // Check if it's time to update the animation frame
    if (currentTime - lastAnimationUpdate >= ANIMATION_FRAME_DURATION) {
        // Move to next frame
        currentAnimationFrame = (currentAnimationFrame + 1) % ANIMATION_FRAME_COUNT;
        
        // Load the new frame into custom character slot 0
        lcd.createChar(0, stormFrames[currentAnimationFrame]);
        
        lastAnimationUpdate = currentTime;
        displayNeedsUpdate = true; // Trigger display refresh
    }
}
