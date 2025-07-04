#ifndef RINGER_MANAGER_H
#define RINGER_MANAGER_H

#include <Arduino.h>
#include "TelephoneRinger.h"

// Forward declaration  
struct SystemConfig;

class RingerManager {
public:
    // Constructor
    RingerManager();
    
    // Destructor
    ~RingerManager();
    
    // Initialize with array of relay pins and configuration
    void initialize(const int* relayPins, int numPhones, const SystemConfig* config, bool enableSerialOutput = true);
    
    // Step all ringers with current time
    void step(unsigned long currentTime);
    
    // Start a call on a specific phone (0-based index)
    void startCall(int phoneIndex);
    
    // Start a call with specific parameters
    void startCall(int phoneIndex, int ringCount, bool cutShort = false, bool useUKStyle = false);
    
    // Stop a call on a specific phone
    void stopCall(int phoneIndex);
    
    // Stop all calls
    void stopAllCalls();
    
    // Set callback for checking if new calls can start
    void setCanStartCallCallback(bool (*callback)());
    
    // Set callback for all phones to check concurrent limit
    void setCanStartCallCallbackForAllPhones(bool (*callback)());
    
    // Set the number of active relays (0-8) - phones beyond this count won't activate
    void setActiveRelayCount(int count);
    
    // Get status information
    int getActiveCallCount() const;
    int getRingingPhoneCount() const;
    int getTotalPhoneCount() const;
    int getActivePhoneCount() const; // Based on configuration
    
    // Individual phone status
    bool isPhoneRinging(int phoneIndex) const;
    bool isPhoneActive(int phoneIndex) const;
    
    // Print status to Serial
    void printStatus() const;

private:
    TelephoneRinger* ringers;
    int phoneCount;
    const SystemConfig* systemConfig;
    unsigned long lastStatusPrint;
    bool enableSerialOutput;  // Flag to control serial output
    int activeRelayCount;     // Number of active relays
    
    static const unsigned long STATUS_PRINT_INTERVAL = 10000; // Print status every 10 seconds
    
    // Note: Legacy debugPrint(String) method removed for heap safety
};

#endif
