#include "RingerManager.h"

RingerManager::RingerManager() {
    ringers = nullptr;
    phoneCount = 0;
    lastStatusPrint = 0;
}

RingerManager::~RingerManager() {
    if (ringers) {
        delete[] ringers;
    }
}

void RingerManager::initialize(const int* relayPins, int numPhones) {
    // Clean up any existing ringers
    if (ringers) {
        delete[] ringers;
    }
    
    phoneCount = numPhones;
    ringers = new TelephoneRinger[phoneCount];
    
    // Initialize each ringer with its relay pin
    for (int i = 0; i < phoneCount; i++) {
        ringers[i].initialize(relayPins[i]);
    }
    
    lastStatusPrint = millis();
    
    Serial.print("RingerManager initialized with ");
    Serial.print(phoneCount);
    Serial.println(" phones");
}

void RingerManager::step(unsigned long currentTime) {
    // Step all ringers
    for (int i = 0; i < phoneCount; i++) {
        ringers[i].step(currentTime);
    }
    
    // Periodically print status
    if (currentTime - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
        printStatus();
        lastStatusPrint = currentTime;
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

void RingerManager::printStatus() const {
    int activeCalls = getActiveCallCount();
    int ringingPhones = getRingingPhoneCount();
    
    Serial.print("Status: ");
    Serial.print(activeCalls);
    Serial.print(" active calls, ");
    Serial.print(ringingPhones);
    Serial.print(" phones ringing out of ");
    Serial.print(phoneCount);
    Serial.println(" total phones");
    
    // Print individual phone status
    Serial.print("Phones: ");
    for (int i = 0; i < phoneCount; i++) {
        if (ringers[i].isRinging()) {
            Serial.print("R");
        } else if (ringers[i].isActive()) {
            Serial.print("A");
        } else {
            Serial.print(".");
        }
    }
    Serial.println(" (R=Ringing, A=Active, .=Idle)");
}
