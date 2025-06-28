#ifndef TELEPHONE_RINGER_H
#define TELEPHONE_RINGER_H

#include <Arduino.h>

class TelephoneRinger {
public:
    // Constructor
    TelephoneRinger();
    
    // Initialize with relay pin
    void initialize(int relayPin);
    
    // Step the state machine with current time
    void step(unsigned long currentTime);
    
    // Start a new call sequence
    void startCall();
    
    // Stop the current call
    void stopCall();
    
    // Check if currently ringing
    bool isRinging() const;
    
    // Check if currently in a call sequence
    bool isActive() const;

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
    unsigned long waitDuration;
    
    // Timing constants (in milliseconds)
    static const unsigned long RING_ON_DURATION = 2000;    // 2 seconds ring on
    static const unsigned long RING_OFF_DURATION = 4000;   // 4 seconds ring off
    static const unsigned long MIN_WAIT_TIME = 5000;       // 5 seconds minimum wait
    static const unsigned long MAX_WAIT_TIME = 30000;      // 30 seconds maximum wait
    static const unsigned long SHORT_RING_DURATION = 300;  // Short ring when "answered"
    
    // Helper methods
    void setRelayState(bool active);
    unsigned long getRandomWaitTime();
    int getRandomRingCount();
    bool shouldCutShortFinalRing();
};

#endif
