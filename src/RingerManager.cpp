#include "RingerManager.h"
// #include "Config.h"  // Commented out for now to avoid dependencies

RingerManager::RingerManager() {
    ringers = nullptr;
    phoneCount = 0;
    lastStatusPrint = 0;
    enableSerialOutput = true;  // Default to enabled
    activeRelayCount = 8;       // Default to all relays active
}

RingerManager::~RingerManager() {
    if (ringers) {
        delete[] ringers;
    }
}

void RingerManager::initialize(const int* relayPins, int numPhones, const SystemConfig* config, bool enableSerialOutput) {
    // Clean up any existing ringers
    if (ringers) {
        delete[] ringers;
    }
    
    this->enableSerialOutput = enableSerialOutput;  // Store the flag
    phoneCount = numPhones;
    systemConfig = config;
    ringers = new TelephoneRinger[phoneCount];
    
    // Initialize each ringer with its relay pin and configuration
    for (int i = 0; i < phoneCount; i++) {
        ringers[i].initialize(relayPins[i], config, enableSerialOutput);
    }
    
    lastStatusPrint = millis();
    
    if (enableSerialOutput) {
        Serial.print("RingerManager initialized with ");
        Serial.print(phoneCount);
        Serial.println(" phones");
    }
}

void RingerManager::step(unsigned long currentTime) {
    // Step only active relays
    int activeCount = min(activeRelayCount, phoneCount);
    for (int i = 0; i < activeCount; i++) {
        ringers[i].step(currentTime);
    }
    
    // Periodically print status only if serial output is enabled
    if (enableSerialOutput && currentTime - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
        printStatus();
        lastStatusPrint = currentTime;
    }
}

void RingerManager::startCall(int phoneIndex, int ringCount, bool cutShort, bool useUKStyle) {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        ringers[phoneIndex].startCall(ringCount, cutShort, useUKStyle);
    }
}

void RingerManager::startCall(int phoneIndex) {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        ringers[phoneIndex].startCall();
    }
}

void RingerManager::stopCall(int phoneIndex) {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        ringers[phoneIndex].stopCall();
    }
}

void RingerManager::stopAllCalls() {
    for (int i = 0; i < phoneCount; i++) {
        ringers[i].stopCall();
    }
}

void RingerManager::setCanStartCallCallback(bool (*callback)()) {
    // This method sets the callback for the manager itself (future use)
    // For now, we'll implement the method that sets it for all phones
}

void RingerManager::setCanStartCallCallbackForAllPhones(bool (*callback)()) {
    for (int i = 0; i < phoneCount; i++) {
        ringers[i].setCanStartCallCallback(callback);
    }
}

void RingerManager::setActiveRelayCount(int count) {
    activeRelayCount = max(0, min(count, phoneCount));
    
    // Stop calls on phones that are now inactive
    for (int i = activeRelayCount; i < phoneCount; i++) {
        ringers[i].stopCall();
    }
}

int RingerManager::getActiveCallCount() const {
    int count = 0;
    for (int i = 0; i < phoneCount; i++) {
        if (ringers[i].isActive()) {
            count++;
        }
    }
    return count;
}

int RingerManager::getRingingPhoneCount() const {
    int count = 0;
    for (int i = 0; i < phoneCount; i++) {
        if (ringers[i].isRinging()) {
            count++;
        }
    }
    return count;
}

int RingerManager::getTotalPhoneCount() const {
    return phoneCount;
}

int RingerManager::getActivePhoneCount() const {
    // Return the number of enabled relays
    return activeRelayCount;
}

bool RingerManager::isPhoneRinging(int phoneIndex) const {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        return ringers[phoneIndex].isRinging();
    }
    return false;
}

bool RingerManager::isPhoneActive(int phoneIndex) const {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        return ringers[phoneIndex].isActive();
    }
    return false;
}

String RingerManager::getPhoneStatus(int phoneIndex) const {
    if (phoneIndex >= 0 && phoneIndex < phoneCount) {
        return ringers[phoneIndex].getStateString();
    }
    return "Invalid";
}

String RingerManager::getStatusLine1() const {
    String status = "Calls: ";
    status += String(getActiveCallCount());
    status += "/";
    status += String(phoneCount);
    return status;
}

String RingerManager::getStatusLine2() const {
    String status = "Ring: ";
    status += String(getRingingPhoneCount());
    status += " Active: ";
    status += String(getActiveCallCount());
    return status;
}

void RingerManager::debugPrint(const String& message) const {
    // Only print if serial output is enabled
    if (enableSerialOutput) {
        Serial.println(message);
    }
}

void RingerManager::printStatus() const {
    if (!enableSerialOutput) return;  // Don't print if serial output is disabled
    
    int activeCalls = getActiveCallCount();
    int ringingPhones = getRingingPhoneCount();
    
    Serial.print(F("Status: "));
    Serial.print(activeCalls);
    Serial.print(F(" active calls, "));
    Serial.print(ringingPhones);
    Serial.print(F(" phones ringing out of "));
    Serial.print(phoneCount);
    Serial.println(F(" total phones"));
    
    // Print individual phone status (cap at 8 phones for safety)
    Serial.print(F("Phones: "));
    int maxPhonesToShow = min(phoneCount, 8);  // Safety cap
    for (int i = 0; i < maxPhonesToShow; i++) {
        if (ringers[i].isRinging()) {
            Serial.print(F("R"));
        } else if (ringers[i].isActive()) {
            Serial.print(F("A"));
        } else {
            Serial.print(F("."));
        }
    }
    Serial.println(F(" (R=Ringing, A=Active, .=Idle)"));
    
    // Show concurrent limit information
    Serial.print(F("Concurrent: "));
    Serial.print(activeCalls);
    Serial.print(F(" active (limit enforced by callback system)"));
    Serial.println();
}
