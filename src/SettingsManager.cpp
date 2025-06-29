#include "SettingsManager.h"

void SettingsManager::initialize() {
    // Initialize EEPROM (some Arduino variants need this)
    // This is safe to call multiple times
}

bool SettingsManager::loadSettings(Settings& settings) {
    // Read version first
    uint8_t version;
    EEPROM.get(EEPROM_VERSION_ADDR, version);
    
    if (version != SETTINGS_VERSION) {
        // Version mismatch or uninitialized EEPROM - use defaults
        settings = getDefaultSettings();
        return false;
    }
    
    // Read settings structure
    EEPROM.get(EEPROM_SETTINGS_ADDR, settings);
    
    // Validate checksum
    uint8_t expectedChecksum = calculateChecksum(settings);
    if (settings.checksum != expectedChecksum) {
        // Corrupted data - use defaults
        settings = getDefaultSettings();
        return false;
    }
    
    // Validate settings values
    if (!validateSettings(settings)) {
        // Invalid values - use defaults
        settings = getDefaultSettings();
        return false;
    }
    
    return true;
}

bool SettingsManager::saveSettings(const Settings& settings) {
    // Validate before saving
    if (!validateSettings(settings)) {
        return false;
    }
    
    // Create a copy with updated checksum
    Settings settingsToSave = settings;
    settingsToSave.version = SETTINGS_VERSION;
    settingsToSave.checksum = calculateChecksum(settingsToSave);
    
    // Save version
    EEPROM.put(EEPROM_VERSION_ADDR, SETTINGS_VERSION);
    
    // Save settings
    EEPROM.put(EEPROM_SETTINGS_ADDR, settingsToSave);
    
    return true;
}

Settings SettingsManager::getDefaultSettings() {
    Settings defaults;
    defaults.version = SETTINGS_VERSION;
    defaults.maxConcurrent = 4;      // MAX_CONCURRENT_ACTIVE_PHONES default
    defaults.activeRelays = 8;       // NUM_PHONES default  
    defaults.maxCallDelay = 30;      // 30 seconds default
    defaults.checksum = 0;           // Will be calculated when saving
    
    return defaults;
}

bool SettingsManager::validateSettings(const Settings& settings) {
    // Validate concurrent limit (1-8)
    if (settings.maxConcurrent < 1 || settings.maxConcurrent > 8) {
        return false;
    }
    
    // Validate active relays (0-8)
    if (settings.activeRelays > 8) {
        return false;
    }
    
    // Validate call delay (10-1000 seconds)
    if (settings.maxCallDelay < 10 || settings.maxCallDelay > 1000) {
        return false;
    }
    
    return true;
}

uint8_t SettingsManager::calculateChecksum(const Settings& settings) {
    // Simple XOR checksum (excluding the checksum field itself)
    uint8_t checksum = 0;
    checksum ^= settings.version;
    checksum ^= settings.maxConcurrent;
    checksum ^= settings.activeRelays;
    checksum ^= (uint8_t)(settings.maxCallDelay & 0xFF);
    checksum ^= (uint8_t)(settings.maxCallDelay >> 8);
    
    return checksum;
}
