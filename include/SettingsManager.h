#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

// Version for settings format - increment when changing structure
#define SETTINGS_VERSION 2

// EEPROM addresses
#define EEPROM_VERSION_ADDR 0
#define EEPROM_SETTINGS_ADDR 4

// Settings structure - keep this simple and avoid complex types
struct Settings {
    uint8_t version;              // Settings version for compatibility
    uint8_t maxConcurrent;        // Concurrent phone limit (1-8)
    uint8_t activeRelays;         // Number of active relays (0-8)
    uint16_t maxCallDelay;        // Call frequency max delay in seconds (10-1000)
    uint8_t ringerHangTime;       // Ringer power hang time in seconds (0-60)
    uint8_t checksum;             // Simple checksum for validation
};

class SettingsManager {
public:
    // Initialize settings manager
    static void initialize();
    
    // Load settings from EEPROM
    static bool loadSettings(Settings& settings);
    
    // Save settings to EEPROM
    static bool saveSettings(const Settings& settings);
    
    // Get default settings
    static Settings getDefaultSettings();
    
    // Validate settings values
    static bool validateSettings(const Settings& settings);
    
private:
    // Calculate simple checksum
    static uint8_t calculateChecksum(const Settings& settings);
};

#endif
