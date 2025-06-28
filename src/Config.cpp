#include "Config.h"
#include <EEPROM.h>

// Default configuration values
const SystemConfig DEFAULT_CONFIG = {
    // Basic operation settings
    .activeRelayCount = 8,              // Use all 8 relays by default
    .maxSimultaneousRings = 3,          // Max 3 phones ringing at once
    .maxRingsPerCall = 8,               // Max 8 rings per call
    
    // Ring timing settings (US standard)
    .ringOnDuration = 2000,             // 2 seconds on
    .ringOffDuration = 4000,            // 4 seconds off
    .minWaitTime = 5000,                // 5 seconds min wait
    .maxWaitTime = 30000,               // 30 seconds max wait
    .shortRingDuration = 300,           // 0.3 seconds for "answered" rings
    
    // Behavioral settings
    .answerProbability = 70,            // 70% chance of being "answered"
    .ringStyle = RING_STYLE_US,         // US style by default
    .patternMode = PATTERN_RANDOM,      // Random pattern by default
    
    // Display settings
    .statusDisplayEnabled = true,       // Show status display
    .displayBrightness = 8,             // Medium brightness
    .displayTimeout = 30,               // 30 seconds before dimming
    
    // Advanced settings
    .sequentialDelay = 1000,            // 1 second between phones in sequential
    .waveSpeed = 5,                     // Medium wave speed
    .debugOutput = true                 // Enable debug output initially
};

ConfigManager::ConfigManager() : configChanged(false) {
    config = DEFAULT_CONFIG;
}

void ConfigManager::loadConfig() {
    // Check if valid configuration exists in EEPROM
    uint16_t magic;
    EEPROM.get(EEPROM_CONFIG_ADDRESS, magic);
    
    if (magic == CONFIG_MAGIC_NUMBER) {
        // Load configuration from EEPROM
        EEPROM.get(EEPROM_CONFIG_ADDRESS + sizeof(magic), config);
        
        if (isConfigValid()) {
            Serial.println("Configuration loaded from EEPROM");
            configChanged = false;
            return;
        } else {
            Serial.println("Invalid configuration in EEPROM, using defaults");
        }
    } else {
        Serial.println("No valid configuration found, using defaults");
    }
    
    // Use defaults and save them
    config = DEFAULT_CONFIG;
    saveConfig();
}

void ConfigManager::saveConfig() {
    constrainValues();
    
    // Write magic number first
    uint16_t magic = CONFIG_MAGIC_NUMBER;
    EEPROM.put(EEPROM_CONFIG_ADDRESS, magic);
    
    // Write configuration
    EEPROM.put(EEPROM_CONFIG_ADDRESS + sizeof(magic), config);
    
    configChanged = false;
    Serial.println("Configuration saved to EEPROM");
}

void ConfigManager::resetToDefaults() {
    config = DEFAULT_CONFIG;
    configChanged = true;
    saveConfig();
    Serial.println("Configuration reset to defaults");
}

void ConfigManager::setActiveRelayCount(uint8_t count) {
    if (count >= 1 && count <= 8) {
        config.activeRelayCount = count;
        configChanged = true;
    }
}

void ConfigManager::setMaxSimultaneousRings(uint8_t max) {
    if (max >= 1 && max <= config.activeRelayCount) {
        config.maxSimultaneousRings = max;
        configChanged = true;
    }
}

void ConfigManager::setMaxRingsPerCall(uint8_t max) {
    if (max >= 1 && max <= 15) {
        config.maxRingsPerCall = max;
        configChanged = true;
    }
}

void ConfigManager::setRingStyle(RingStyle style) {
    if (style <= RING_STYLE_CUSTOM) {
        config.ringStyle = style;
        configChanged = true;
    }
}

void ConfigManager::setPatternMode(PatternMode mode) {
    if (mode <= PATTERN_CUSTOM) {
        config.patternMode = mode;
        configChanged = true;
    }
}

void ConfigManager::setAnswerProbability(uint8_t probability) {
    if (probability <= 100) {
        config.answerProbability = probability;
        configChanged = true;
    }
}

void ConfigManager::setDisplayBrightness(uint8_t brightness) {
    if (brightness <= 15) {
        config.displayBrightness = brightness;
        configChanged = true;
    }
}

void ConfigManager::setDebugOutput(bool enabled) {
    config.debugOutput = enabled;
    configChanged = true;
}

bool ConfigManager::isConfigValid() const {
    return (config.activeRelayCount >= 1 && config.activeRelayCount <= 8 &&
            config.maxSimultaneousRings >= 1 && config.maxSimultaneousRings <= config.activeRelayCount &&
            config.maxRingsPerCall >= 1 && config.maxRingsPerCall <= 15 &&
            config.ringOnDuration >= 100 && config.ringOnDuration <= 10000 &&
            config.ringOffDuration >= 100 && config.ringOffDuration <= 20000 &&
            config.answerProbability <= 100 &&
            config.ringStyle <= RING_STYLE_CUSTOM &&
            config.patternMode <= PATTERN_CUSTOM);
}

void ConfigManager::constrainValues() {
    config.activeRelayCount = constrain(config.activeRelayCount, 1, 8);
    config.maxSimultaneousRings = constrain(config.maxSimultaneousRings, 1, config.activeRelayCount);
    config.maxRingsPerCall = constrain(config.maxRingsPerCall, 1, 15);
    config.ringOnDuration = constrain(config.ringOnDuration, 100, 10000);
    config.ringOffDuration = constrain(config.ringOffDuration, 100, 20000);
    config.minWaitTime = constrain(config.minWaitTime, 1000, 120000);
    config.maxWaitTime = constrain(config.maxWaitTime, config.minWaitTime, 300000);
    config.answerProbability = constrain(config.answerProbability, 0, 100);
    config.displayBrightness = constrain(config.displayBrightness, 0, 15);
    config.sequentialDelay = constrain(config.sequentialDelay, 100, 5000);
    config.waveSpeed = constrain(config.waveSpeed, 1, 10);
}

void ConfigManager::getRingTiming(uint16_t& onDuration, uint16_t& offDuration, bool isUKStyle) const {
    switch (config.ringStyle) {
        case RING_STYLE_US:
        default:
            onDuration = config.ringOnDuration;
            offDuration = config.ringOffDuration;
            break;
            
        case RING_STYLE_UK:
            // UK: Double ring - 0.4s on, 0.2s off, 0.4s on, 2s off
            // Simplified to single values for now
            onDuration = 400;
            offDuration = 2000;
            break;
            
        case RING_STYLE_MIXED:
            // For mixed mode, caller specifies UK or US style
            if (isUKStyle) {
                onDuration = 400;
                offDuration = 2000;
            } else {
                onDuration = config.ringOnDuration;
                offDuration = config.ringOffDuration;
            }
            break;
            
        case RING_STYLE_CUSTOM:
            onDuration = config.ringOnDuration;
            offDuration = config.ringOffDuration;
            break;
    }
}
