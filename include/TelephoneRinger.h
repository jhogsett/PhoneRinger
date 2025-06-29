#ifndef TELEPHONE_RINGER_H
#define TELEPHONE_RINGER_H

#include <Arduino.h>

// Forward declaration
struct SystemConfig;

// External reference to global call frequency setting
extern int maxCallDelaySetting;

// Callback function type for checking if a new call can start
typedef bool (*CanStartCallCallback)();

class TelephoneRinger {
public:
    // Constructor
    TelephoneRinger();
    
    // Initialize with relay pin and configuration
    void initialize(int relayPin, const SystemConfig* config, bool enableSerialOutput = true);
    
    // Set callback for checking if new calls are allowed
    void setCanStartCallCallback(CanStartCallCallback callback);
    
    // Step the state machine with current time
    void step(unsigned long currentTime);
    
    // Start a new call sequence
    void startCall();
    
    // Start a call with specific parameters
    void startCall(int ringCount, bool cutShort = false, bool useUKStyle = false);
    
    // Stop the current call
    void stopCall();
    
    // Check if currently ringing
    bool isRinging() const;
    
    // Check if currently in a call sequence
    bool isActive() const;
    
    // Get current state for display
    String getStateString() const;

private:
    enum RingerState {
        IDLE,           // Waiting for next call
        RING_ON,        // Ring tone is on
        RING_OFF,       // Ring tone is off (between rings)
        CALL_ANSWERED,  // Call answered (hanging up)
        WAITING         // Waiting before next call attempt
    };
    
    // State variables
    RingerState state;
    int relayPin;
    unsigned long lastStateChange;
    int currentRingCount;
    int totalRingsToMake;
    bool finalRingCutShort;
    bool useUKRingStyle;
    unsigned long waitDuration;
    bool enableSerialOutput;  // Flag to control serial output
    
    // Configuration reference
    const SystemConfig* systemConfig;
    
    // Callback for checking if new calls are allowed
    CanStartCallCallback canStartCallCallback;
    
    // Current timing values (may vary based on ring style)
    unsigned long currentRingOnDuration;
    unsigned long currentRingOffDuration;
    
    // Helper methods
    void setRelayState(bool active);
    unsigned long getRandomWaitTime();
    void debugPrint(const String& message);
};

#endif
