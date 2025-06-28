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

void DisplayManager::initialize() {
    Serial.println("Initializing 20x4 LCD Display...");
    
    // Add a timeout to prevent hanging if LCD is not connected
    Serial.println("Scanning I2C bus for LCD...");
    
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
        Serial.println("Found I2C device at 0x27");
        initSuccess = true;
    } else {
        Wire.beginTransmission(0x3F); // Try alternate address
        if (Wire.endTransmission() == 0) {
            Serial.println("Found I2C device at 0x3F");
            initSuccess = true;
        }
    }
    
    if (initSuccess) {
        status = lcd.begin(LCD_COLS, LCD_ROWS);
        if (status == 0) {
            lcdAvailable = true;  // Mark LCD as available
            lcd.clear();
            showStartupMessage();
            Serial.println("20x4 LCD Display initialized successfully");
        } else {
            Serial.print("LCD initialization failed with status: ");
            Serial.println(status);
            Serial.println("Continuing without LCD...");
            lcdAvailable = false;
        }
    } else {
        Serial.println("No I2C LCD found at addresses 0x27 or 0x3F");
        Serial.println("Continuing without LCD...");
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
    
    lcd.clear();
    
    if (line1.length() > 0) {
        lcd.setCursor(0, 0);
        lcd.print(padString(line1, 20));
    }
    if (line2.length() > 0) {
        lcd.setCursor(0, 1);
        lcd.print(padString(line2, 20));
    }
    if (line3.length() > 0) {
        lcd.setCursor(0, 2);
        lcd.print(padString(line3, 20));
    }
    if (line4.length() > 0) {
        lcd.setCursor(0, 3);
        lcd.print(padString(line4, 20));
    }
}

void DisplayManager::showStatus(const RingerManager* ringerManager, bool paused, int maxConcurrent) {
    if (!lcdAvailable) return; // Skip if LCD not available
    
    // Line 1: Title and system time (20 chars max: "CallCenter  00:00")
    lcd.setCursor(0, 0);
    String titleLine = "CallCenter  " + formatTime(millis());
    lcd.print(padString(titleLine, 20));
    
    // Line 2: Active calls and ringing phones with concurrent limit (20 chars max)
    // Format: "A:0 R:0 Tot:8 Max:4" or "A:0 R:0 Tot:8" if no limit
    lcd.setCursor(0, 1);
    String statusLine = "A:" + String(ringerManager->getActiveCallCount()) + 
                       " R:" + String(ringerManager->getRingingPhoneCount()) +
                       " Tot:" + String(ringerManager->getActivePhoneCount());
    
    if (maxConcurrent > 0 && maxConcurrent < ringerManager->getTotalPhoneCount()) {
        statusLine += " Max:" + String(maxConcurrent);
    }
    
    lcd.print(padString(statusLine, 20));
    
    // Line 3: Visual phone status (20 chars max for 8 phones)
    lcd.setCursor(0, 2);
    String phoneStatus = formatPhoneStatus(ringerManager);
    lcd.print(padString(phoneStatus, 20));
    
    // Line 4: Status message (20 chars max)
    lcd.setCursor(0, 3);
    if (paused) {
        centerText("** PAUSED **", 3);
    } else {
        String runningMsg = "Run - A0=Pause";  // Shortened to fit
        lcd.print(padString(runningMsg, 20));
    }
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

String DisplayManager::padString(String str, int length) {
    while (str.length() < length) {
        str += " ";
    }
    return str.substring(0, length); // Truncate if too long
}
