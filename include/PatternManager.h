#ifndef PATTERN_MANAGER_H
#define PATTERN_MANAGER_H

#include <Arduino.h>
#include "Config.h"

// Forward declaration
class RingerManager;

class PatternManager {
public:
    PatternManager();
    
    // Initialize with reference to ringer manager
    void initialize(RingerManager* ringerMgr, const SystemConfig* config);
    
    // Update pattern logic
    void step(unsigned long currentTime);
    
    // Pattern control
    void setPatternMode(PatternMode mode);
    void startPattern();
    void stopPattern();
    void pausePattern();
    void resumePattern();
    
    // Pattern state
    bool isPatternActive() const { return patternActive; }
    PatternMode getCurrentMode() const { return currentMode; }
    
    // Get pattern status for display
    String getPatternStatus() const;
    
private:
    RingerManager* ringerManager;
    const SystemConfig* systemConfig;
    
    PatternMode currentMode;
    bool patternActive;
    bool patternPaused;
    unsigned long lastPatternStep;
    
    // Pattern-specific state variables
    uint8_t sequentialIndex;           // Current phone in sequential mode
    uint8_t wavePosition;              // Current position in wave
    bool waveDirection;                // Wave direction (true = forward)
    unsigned long burstStartTime;      // Start time of current burst
    bool inBurst;                      // Currently in a burst period
    
    // Pattern implementations
    void stepRandomPattern(unsigned long currentTime);
    void stepSequentialPattern(unsigned long currentTime);
    void stepWavePattern(unsigned long currentTime);
    void stepMayhemPattern(unsigned long currentTime);
    void stepBurstPattern(unsigned long currentTime);
    
    // Helper methods
    void activatePhone(uint8_t phoneIndex);
    void deactivatePhone(uint8_t phoneIndex);
    uint8_t getActivePhoneCount() const;
    bool shouldLimitSimultaneousRings() const;
    uint8_t getRandomInactivePhone() const;
    void startWave();
    void startBurst();
};

#endif
