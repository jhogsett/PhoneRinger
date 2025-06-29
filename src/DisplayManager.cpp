#include "DisplayManager.h"
#include "RingerManager.h"

// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;

DisplayManager::DisplayManager() {
    lastUpdate = 0;
    currentScreen = 0;
    displayNeedsUpdate = true;
    lcdAvailable = false;  // Will be set to true if LCD initializes successfully
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
            showStartupMessage();
            if (enableSerialOutput) {
                Serial.println("20x4 LCD Display initialized successfully");
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

void DisplayManager::showMessage(const String& line1, const String& line2, 
                                const String& line3, const String& line4) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    static char lineBuffer[21];  // Static buffer for padding
    
    lcd.clear();
    
    if (line1.length() > 0) {
        lcd.setCursor(0, 0);
        padStringToBuffer(lineBuffer, line1.c_str(), 20);
        lcd.print(lineBuffer);
    }
    if (line2.length() > 0) {
        lcd.setCursor(0, 1);
        padStringToBuffer(lineBuffer, line2.c_str(), 20);
        lcd.print(lineBuffer);
    }
    if (line3.length() > 0) {
        lcd.setCursor(0, 2);
        padStringToBuffer(lineBuffer, line3.c_str(), 20);
        lcd.print(lineBuffer);
    }
    if (line4.length() > 0) {
        lcd.setCursor(0, 3);
        padStringToBuffer(lineBuffer, line4.c_str(), 20);
        lcd.print(lineBuffer);
    }
}

void DisplayManager::showMessage(const char* line1, const char* line2, 
                                const char* line3, const char* line4) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    static char lineBuffer[21];  // Static buffer for padding
    
    lcd.clear();
    
    if (line1 && strlen(line1) > 0) {
        lcd.setCursor(0, 0);
        padStringToBuffer(lineBuffer, line1, 20);
        lcd.print(lineBuffer);
    }
    if (line2 && strlen(line2) > 0) {
        lcd.setCursor(0, 1);
        padStringToBuffer(lineBuffer, line2, 20);
        lcd.print(lineBuffer);
    }
    if (line3 && strlen(line3) > 0) {
        lcd.setCursor(0, 2);
        padStringToBuffer(lineBuffer, line3, 20);
        lcd.print(lineBuffer);
    }
    if (line4 && strlen(line4) > 0) {
        lcd.setCursor(0, 3);
        padStringToBuffer(lineBuffer, line4, 20);
        lcd.print(lineBuffer);
    }
}

void DisplayManager::showStatus(const RingerManager* ringerManager, bool paused, int maxConcurrent) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    // Static buffers to avoid String allocation
    static char line1Buffer[21];
    static char line2Buffer[21]; 
    static char line3Buffer[21];
    static char line4Buffer[21];
    
    // Line 1: Title and system time (20 chars max: "CallCenter  00:00")
    lcd.setCursor(0, 0);
    unsigned long seconds = millis() / 1000;
    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    snprintf(line1Buffer, sizeof(line1Buffer), "CallCenter  %02lu:%02lu", minutes % 100, seconds);
    // Ensure exactly 20 characters by padding with spaces
    int len1 = strlen(line1Buffer);
    for (int i = len1; i < 20; i++) {
        line1Buffer[i] = ' ';
    }
    line1Buffer[20] = '\0';
    lcd.print(line1Buffer);
    
    // Line 2: Active calls and ringing phones with enabled relay count (20 chars max)
    // Format: "A:0 R:0 En:8 Max:4" or "A:0 R:0 En:8" if no limit
    lcd.setCursor(0, 1);
    if (maxConcurrent > 0 && maxConcurrent < ringerManager->getTotalPhoneCount()) {
        snprintf(line2Buffer, sizeof(line2Buffer), "A:%d R:%d En:%d Max:%d", 
                ringerManager->getActiveCallCount(),
                ringerManager->getRingingPhoneCount(),
                ringerManager->getActivePhoneCount(),
                maxConcurrent);
    } else {
        snprintf(line2Buffer, sizeof(line2Buffer), "A:%d R:%d En:%d", 
                ringerManager->getActiveCallCount(),
                ringerManager->getRingingPhoneCount(),
                ringerManager->getActivePhoneCount());
    }
    // Pad to 20 characters
    int len = strlen(line2Buffer);
    for (int i = len; i < 20; i++) {
        line2Buffer[i] = ' ';
    }
    line2Buffer[20] = '\0';
    lcd.print(line2Buffer);
    
    // Line 3: Visual phone status (20 chars max for 8 phones)
    lcd.setCursor(0, 2);
    // Create phone status without String allocation
    for (int i = 0; i < 8 && i < 20; i++) {
        if (i < ringerManager->getActivePhoneCount()) {
            if (ringerManager->isPhoneRinging(i)) {
                line3Buffer[i] = 'R';  // Ringing
            } else if (ringerManager->isPhoneActive(i)) {
                line3Buffer[i] = 'A';  // Active
            } else {
                line3Buffer[i] = '-';  // Idle
            }
        } else {
            line3Buffer[i] = 'X';  // Disabled
        }
    }
    // Pad remaining characters
    for (int i = 8; i < 20; i++) {
        line3Buffer[i] = ' ';
    }
    line3Buffer[20] = '\0';
    lcd.print(line3Buffer);
    
    // Line 4: Status message (20 chars max)
    lcd.setCursor(0, 3);
    if (paused) {
        snprintf(line4Buffer, sizeof(line4Buffer), "** PAUSED **");
        // Center the text
        int textLen = strlen(line4Buffer);
        int spaces = (20 - textLen) / 2;
        for (int i = 19; i >= 0; i--) {
            if (i >= spaces && i < spaces + textLen) {
                line4Buffer[i] = line4Buffer[i - spaces];
            } else {
                line4Buffer[i] = ' ';
            }
        }
        line4Buffer[20] = '\0';
    } else {
        snprintf(line4Buffer, sizeof(line4Buffer), "Run - A0=Pause      ");
    }
    lcd.print(line4Buffer);
}

void DisplayManager::showStartupMessage() {
    showMessage("Call Center Simulator",
                "Version 2.0",
                "8-Phone System",
                "Initializing...");
}

void DisplayManager::showPauseMessage() {
    showMessage("Call Center Sim",
                "** SYSTEM PAUSED **",
                "All relays OFF",
                "Press PinA0 to resume");
}

void DisplayManager::showResumeMessage() {
    showMessage("Call Center Sim",
                "** SYSTEM RESUMED **",
                "Calls restarting...",
                "");
    delay(1000); // Show resume message briefly
    displayNeedsUpdate = true;
}

String DisplayManager::formatTime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    
    String timeStr = "";
    if (minutes < 10) timeStr += "0";
    timeStr += String(minutes) + ":";
    if (seconds < 10) timeStr += "0";
    timeStr += String(seconds);
    
    return timeStr;
}

String DisplayManager::formatPhoneStatus(const RingerManager* ringerManager) {
    String status = "";
    
    // Limit to 8 phones max to prevent memory issues
    int maxPhones = min(8, ringerManager->getTotalPhoneCount());
    
    // Format: "12345678" where each digit represents phone state
    // R = Ringing, A = Active, . = Idle
    for (int i = 0; i < maxPhones; i++) {
        if (ringerManager->isPhoneRinging(i)) {
            status += "R";  // Ringing
        } else if (ringerManager->isPhoneActive(i)) {
            status += "A";  // Active
        } else {
            status += ".";  // Idle
        }
    }
    
    // Add phone position labels on remaining space
    if (status.length() < 20) {
        status += " [12345678]";  // Shows which position is which phone
    }
    
    return status;
}

String DisplayManager::formatActiveCount(const RingerManager* ringerManager) {
    return "A:" + String(ringerManager->getActiveCallCount()) + 
           " R:" + String(ringerManager->getRingingPhoneCount());
}

void DisplayManager::centerText(String text, int line, int width) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    int padding = (width - text.length()) / 2;
    lcd.setCursor(padding, line);
    lcd.print(text);
}

void DisplayManager::padStringToBuffer(char* buffer, const char* str, int length) {
    int strLen = strlen(str);
    
    // Copy the string, truncating if too long
    int copyLen = min(strLen, length);
    strncpy(buffer, str, copyLen);
    
    // Pad with spaces if needed
    for (int i = copyLen; i < length; i++) {
        buffer[i] = ' ';
    }
    
    // Null terminate
    buffer[length] = '\0';
}
