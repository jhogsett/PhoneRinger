#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Configuration structure to hold all user settings
struct SystemConfig {
    // Basic operation settings
    uint8_t activeRelayCount;           // Number of relays to use (1-8)
    uint8_t maxSimultaneousRings;       // Max phones ringing at once (1-8)
    uint8_t maxRingsPerCall;            // Maximum rings per call (1-15)
    
    // Ring timing settings (in milliseconds)
    uint16_t ringOnDuration;            // How long each ring lasts
    uint16_t ringOffDuration;           // Silence between rings
    uint16_t minWaitTime;               // Min wait between call attempts
    uint16_t maxWaitTime;               // Max wait between call attempts
    uint16_t shortRingDuration;         // Duration of "answered" rings
    
    // Behavioral settings
    uint8_t answerProbability;          // % chance final ring gets cut short (0-100)
    uint8_t ringStyle;                  // Ring style (US, UK, Mixed, etc.)
    uint8_t patternMode;                // Pattern mode (Random, Sequential, Wave, etc.)
    
    // Display settings
    bool statusDisplayEnabled;          // Show status on LED display
    uint8_t displayBrightness;          // Display brightness (0-15)
    uint8_t displayTimeout;             // Seconds before display dims/off
    
    // Advanced settings
    uint16_t sequentialDelay;           // Delay between phones in sequential mode (ms)
    uint8_t waveSpeed;                  // Speed of wave effect (1-10)
    bool debugOutput;                   // Enable serial debug output
};

// Ring style definitions
enum RingStyle {
    RING_STYLE_US = 0,      // 2 sec on, 4 sec off
    RING_STYLE_UK = 1,      // 0.4 sec on, 0.2 sec off, 0.4 sec on, 2 sec off
    RING_STYLE_MIXED = 2,   // Random mix of US and UK
    RING_STYLE_CUSTOM = 3   // User-defined timing
};

// Pattern mode definitions
enum PatternMode {
    PATTERN_RANDOM = 0,     // Random activation (current behavior)
    PATTERN_SEQUENTIAL = 1, // Activate phones in sequence 1->2->3->etc
    PATTERN_WAVE = 2,       // Wave effect across phones
    PATTERN_MAYHEM = 3,     // Maximum chaos - all phones active
    PATTERN_BURST = 4,      // Periodic bursts of activity
    PATTERN_CUSTOM = 5      // User-defined pattern
};

// Default configuration values
extern const SystemConfig DEFAULT_CONFIG;

// Configuration management class
class ConfigManager {
public:
    ConfigManager();
    
    // Load/save configuration
    void loadConfig();
    void saveConfig();
    void resetToDefaults();
    
    // Get current configuration
    const SystemConfig& getConfig() const { return config; }
    
    // Update configuration settings
    void setActiveRelayCount(uint8_t count);
    void setMaxSimultaneousRings(uint8_t max);
    void setMaxRingsPerCall(uint8_t max);
    void setRingStyle(RingStyle style);
    void setPatternMode(PatternMode mode);
    void setAnswerProbability(uint8_t probability);
    void setDisplayBrightness(uint8_t brightness);
    void setDebugOutput(bool enabled);
    
    // Validate and constrain values
    bool isConfigValid() const;
    void constrainValues();
    
    // Get ring timing based on current style
    void getRingTiming(uint16_t& onDuration, uint16_t& offDuration, bool isUKStyle = false) const;
    
private:
    SystemConfig config;
    bool configChanged;
    
    // EEPROM addresses for configuration storage
    static const int EEPROM_CONFIG_ADDRESS = 0;
    static const uint16_t CONFIG_MAGIC_NUMBER = 0xABCD; // To verify valid config
};

#endif
