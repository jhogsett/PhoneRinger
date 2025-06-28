#ifndef RINGER_MANAGER_H
#define RINGER_MANAGER_H

#include <Arduino.h>
#include "TelephoneRinger.h"

class RingerManager {
public:
    // Constructor
    RingerManager();
    
    // Destructor
    ~RingerManager();
    
    // Initialize with array of relay pins
    void initialize(const int* relayPins, int numPhones);
    
    // Step all ringers with current time
    void step(unsigned long currentTime);
    
    // Start a call on a specific phone (0-based index)
    void startCall(int phoneIndex);
    
    // Stop a call on a specific phone
    void stopCall(int phoneIndex);
    
    // Stop all calls
    void stopAllCalls();
    
    // Get status information
    int getActiveCallCount() const;
    int getRingingPhoneCount() const;
    int getTotalPhoneCount() const;
    
    // Print status to Serial
    void printStatus() const;

private:
    TelephoneRinger* ringers;
    int phoneCount;
    unsigned long lastStatusPrint;
    
    static const unsigned long STATUS_PRINT_INTERVAL = 10000; // Print status every 10 seconds
};

#endif
